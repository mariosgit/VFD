/// A GFX 1-bit canvas context for graphics

#include <SPI.h>
#include "mbGFX_MN12832L.h"
#include <mbLog.h>

const int16_t targetFps = 150 * 44; // this is per gate // 

// display spi kann bis 200ns/5MHz geht aber auch mit 16 noch ???
SPISettings settingsA(16000000, MSBFIRST, SPI_MODE0);

// Create an IntervalTimer object
#ifdef __STM32F1__
HardwareTimer timer(2);
#else
IntervalTimer myTimer;
#endif

MN12832L *MN12832L::_the = nullptr;

MN12832L::MN12832L(
    byte pinBLK,
    byte pinLAT,
    byte pinGCP,
    byte MOSI_PIN,
    byte SCK_PIN,
    byte pinPWM) : Adafruit_GFX(128, 32),
                   gate(50),
                   displayTime(0),
                   pinBLK(pinBLK),
                   pinLAT(pinLAT),
                   pinGCP(pinGCP),
                   MOSI_PIN(MOSI_PIN),
                   SCK_PIN(SCK_PIN),
                   pinPWM(pinPWM)
{
    _the = this;
}

MN12832L::~MN12832L(void)
{
}

void MN12832L::begin()
{
    pinMode(pinBLK, OUTPUT);
    pinMode(pinGCP, OUTPUT);
    pinMode(pinLAT, OUTPUT);

    digitalWrite(pinBLK, HIGH);
    digitalWrite(pinLAT, LOW);
    digitalWrite(pinGCP, LOW);

    SPI.setMOSI(MOSI_PIN);
    SPI.setSCK(SCK_PIN);
    SPI.begin();

    pinMode(pinPWM, OUTPUT);
    analogWrite(pinPWM, 128); // 50% duty

    // display needs to cygle all the time !
    myTimer.begin(displayRefresh, 1000000 / targetFps); // starting slowly
}

void MN12832L::drawPixel(int16_t px, int16_t py, uint16_t color)
{
    if ((px < 0) || (py < 0) || (px >= _width) || (py >= _height))
        return;

    // mem coordinates, allways 3 bytes = 4 lines = 24 pixel in one block
    register uint8_t gate = px / 6 + 1;
    register uint8_t pixl = px % 6;
    register uint8_t yblk = py / 4;
    register uint8_t yoff = py % 4;

    // choose pixels
    register uint32_t pixp;
    if (pixl == 0)      pixp = B100000; // a
    else if (pixl == 1) pixp = B001000; // b
    else if (pixl == 2) pixp = B000010; // c
    else if (pixl == 3) pixp = B000001; // d
    else if (pixl == 4) pixp = B000100; // e
    else if (pixl == 5) pixp = B010000; // f

    // write pixel 6packs 111111xx xxxxxxxx xxxxxxxx
    if (yoff == 0)
        pixp = pixp << 18;
    else if (yoff == 1)
        pixp = pixp << 12;
    else if (yoff == 2)
        pixp = pixp << 6;
    else if (yoff == 3)
        pixp = pixp << 0;

    // LOG <<LOG.dec <<"x:" <<px <<"x" <<py <<" X+6:\tG:" <<gate <<":" <<LOG.bin <<fetch <<LOG.endl;

    if(color)
    {
        buffer[bufferOffset + 24 * gate + yblk * 3 + 0] |= (pixp & 0x00FF0000) >> 16;
        buffer[bufferOffset + 24 * gate + yblk * 3 + 1] |= (pixp & 0x0000FF00) >> 8;
        buffer[bufferOffset + 24 * gate + yblk * 3 + 2] |= (pixp & 0x000000FF);
    }
    else
    {
        buffer[bufferOffset + 24 * gate + yblk * 3 + 0] &= ~((pixp & 0x00FF0000) >> 16);
        buffer[bufferOffset + 24 * gate + yblk * 3 + 1] &= ~((pixp & 0x0000FF00) >> 8);
        buffer[bufferOffset + 24 * gate + yblk * 3 + 2] &= ~((pixp & 0x000000FF));
    }

    // LOG << LOG.dec << "pixel: " << x << "," << y << " gate:" << gate << " pixl:" << pixl << " yblk:" << yblk << " yoff:" << yoff  ;
    // LOG <<" buf:" <<LOG.bin;
    // LOG <<" " <<buffer[24 * gate + yblk + 0];
    // LOG <<" " <<buffer[24 * gate + yblk + 1];
    // LOG <<" " <<buffer[24 * gate + yblk + 2];
    // LOG << LOG.endl;
}

void MN12832L::fillScreen(uint8_t color)
{
    memset(buffer+bufferOffset, color, bufferSize);

    // byte tempBuffer[24] = {
    //     B10000010, B00001000, B00100000, // 4 rows | a
    //     B00100000, B10000010, B00001000, // 4 rows | b
    //     B00001000, B00100000, B10000010, // 4 rows | c
    //     B00000100, B00010000, B01000001, // 4 rows | d
    //     B00010000, B01000001, B00000100, // 4 rows | e
    //     B01000001, B00000100, B00010000, // 4 rows | f
    //     0, 0, 0,
    //     0, 0, 0};

    // LOG << "mset buffer: " << LOG.hex << (uint32_t)buffer << ": " << LOG.bin << buffer[0] << buffer[1] << buffer[2] << buffer[3] << LOG.endl;
}

void MN12832L::swapBuffers()
{
    noInterrupts();
    if(bufferOffset > 0)
    {
        bufferOffset = 0;
    }
    else
    {
        bufferOffset = bufferSize;
    }
    interrupts();
}

uint32_t MN12832L::getDisplayTime()
{
    uint32_t result;
    noInterrupts();
    result = _the->displayTime;
    interrupts();
    return result;
}

uint32_t MN12832L::getDisplayFps1()
{
    uint32_t result;
    noInterrupts();
    result = _the->displayFps1;
    interrupts();
    return result;
}

void MN12832L::nextGate()
{
    uint8_t gate = _the->gate;

    uint8_t endstop = 43;
    if (gate > endstop)
        gate = 0;

    // shift 2 neighbouring gates down 44 bits
    // but shifter has 48 bits / 6 bytes, use 6 bytes / last 5 bits digits unuses !
    if (gate == 0)
        _the->gateBuf = 0x8000000000000000;
    if (gate == 1)
        _the->gateBuf = 0xC000000000000000;
    // else if (gate == endstop)
    //     _the->gateBuf = 0x8000000000100000;
    else
        _the->gateBuf = _the->gateBuf >> 1;

    _the->gate = gate + 1;

    // LOG <<"gate:" <<LOG.dec <<gate <<LOG.hex <<" gb:" <<_the->gateBuf <<LOG.endl;
}

void MN12832L::displayRefresh()
{
    _the->displayFps1 = micros() - _the->displayLast;
    _the->displayLast = micros();
    uint32_t time = micros();

    uint8_t *buffer = nullptr;
    buffer = _the->buffer; // double buffer disabled
    if(_the->bufferOffset)
    {
        buffer = _the->buffer;
    }
    else
    {
        buffer = _the->buffer + bufferSize;
    }

    // LOG <<"draw buffer: " <<LOG.hex <<(uint32_t)ptr <<": " <<LOG.bin <<*ptr++ <<*ptr++ <<LOG.endl;

    // if (_the->gate % 2 == 1)
    //     return;

    // LOG <<_the->gate <<LOG.endl;

    int8_t mask = (_the->gate % 2 == 1) ? B01010101 : B10101010;  // mask off either abc or cde pixels

    uint8_t *ptr = (buffer + 24 * (_the->gate/2 + 0));
    uint8_t *dst = _the->tempBuffer;
    for(int i = 0; i< 24; i++)
    {
        *dst++ = (*ptr++) & mask;
    }

    ptr = _the->tempBuffer;

    // copy columns from display buffer !
    {

        // LOG <<"data[0]:" <<LOG.bin <<*ptr <<LOG.endl;

        SPI.beginTransaction(settingsA);
        // shift out 24 bytes = 192 bits = 32rows * 3depth * 2colomns
        SPI.transfer(ptr, 24);

        // shift out gates
        // bits 192-236 are the gates..

        // LOG <<"gate:" <<LOG.dec <<_the->_gate <<LOG.hex <<" gb:\t" <<_the->gateBuf <<LOG.endl;
        union u64u8
        {
            uint64_t u64;
            uint8_t u8[8];
        } gbuf = {_the->gateBuf};
        ptr = gbuf.u8;
        ptr += 7;

        // LOG <<LOG.hex <<"         :\t";
        for (int i = 0; i < 6; i++)
        {
            SPI.transfer(*ptr);
            // LOG <<*ptr;
            ptr--;
        }
        // LOG <<LOG.endl;
        SPI.endTransaction();

        digitalWrite(_the->pinBLK, HIGH);

        digitalWrite(_the->pinLAT, HIGH);
        digitalWrite(_the->pinLAT, LOW);

        digitalWrite(_the->pinBLK, LOW);
    }

    nextGate();

    _the->displayTime = micros() - time;
}
