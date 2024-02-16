/// A GFX 1-bit canvas context for graphics

/// double buffer:
///   buffer+offset is the one to draw, the other will be for display

#include <SPI.h>
#include "mbGFX_MN12832L.h"
#include <mbLog.h>

const int16_t targetFps = 200 * 44; // this is per gate

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

void MN12832L::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (y < 0) || (x >= _width) || (y >= _height))
        return;

    x+= 6;

    register uint8_t gate = x / 6;
    register uint8_t pixl = x % 6;
    register uint8_t yblk = y / 4;
    register uint8_t yoff = y % 4;

    // if(pixl > 3) gate++; // ja da is overlap !?

    register uint8_t pixp = 0;
    if (pixl == 0)
    {
        pixp = B100000;
    } // a
    else if (pixl == 1)
    {
        pixp = B001000;
    } // b
    else if (pixl == 2)
    {
        pixp = B000010;
    } // c
    else if (pixl == 3)
    {
        pixp = 0;//B000001;
    } // d
    else if (pixl == 4)
    {
        pixp = 0;//B000100;
    } // e
    else if (pixl == 5)
    {
        pixp = 0;//B010000;
    } // f

    register uint32_t fetch = 0;
    if (yoff == 0)
    {
        fetch = pixp << 18;
    } // write 1st 6pack 111111xx xxxxxxxx xxxxxxxx
    else if (yoff == 1)
    {
        fetch = pixp << 12;
    } // 2nd/3rd         xxxxxx11 1111xxxx xxxxxxxx
    else if (yoff == 2)
    {
        fetch = pixp << 6;
    } // 3rd/4th         xxxxxxxx xxxx1111 11xxxxxx
    else if (yoff == 3)
    {
        fetch = pixp;
    } // 4th             xxxxxxxx xxxxxxxx xx111111

    bufferEven[24 * gate + yblk * 3 + 0] |= (fetch & 0x00FF0000) >> 16;
    bufferEven[24 * gate + yblk * 3 + 1] |= (fetch & 0x0000FF00) >> 8;
    bufferEven[24 * gate + yblk * 3 + 2] |= (fetch & 0x000000FF);

    // same again for defabc pixel order...
    x-= 3;
    gate = x / 6;
    pixl = x % 6;
    yblk = y / 4;
    yoff = y % 4;

    // reverse order ???
    if (pixl == 0)
    {
        pixp = B000001;
    } // a
    else if (pixl == 1)
    {
        pixp = B000100;
    } // b
    else if (pixl == 2)
    {
        pixp = B010000;
    } // c
    else if (pixl == 3)
    {
        pixp = 0;//B100000;
    } // d
    else if (pixl == 4)
    {
        pixp = 0;//B001000;
    } // e
    else if (pixl == 5)
    {
        pixp = 0;//B000010;
    } // f

    fetch = 0;
    if (yoff == 0)
    {
        fetch = pixp << 18;
    } // write 1st 6pack 111111xx xxxxxxxx xxxxxxxx
    else if (yoff == 1)
    {
        fetch = pixp << 12;
    } // 2nd/3rd         xxxxxx11 1111xxxx xxxxxxxx
    else if (yoff == 2)
    {
        fetch = pixp << 6;
    } // 3rd/4th         xxxxxxxx xxxx1111 11xxxxxx
    else if (yoff == 3)
    {
        fetch = pixp;
    } // 4th             xxxxxxxx xxxxxxxx xx111111

    bufferOdd[24 * gate + yblk * 3 + 0] |= (fetch & 0x00FF0000) >> 16;
    bufferOdd[24 * gate + yblk * 3 + 1] |= (fetch & 0x0000FF00) >> 8;
    bufferOdd[24 * gate + yblk * 3 + 2] |= (fetch & 0x000000FF);

    // LOG << LOG.dec << "pixel: " << x << "," << y << " gate:" << gate << " pixl:" << pixl << " yblk:" << yblk << " yoff:" << yoff  ;
    // LOG <<" buf:" <<LOG.bin;
    // LOG <<" " <<buffer[24 * gate + yblk + 0];
    // LOG <<" " <<buffer[24 * gate + yblk + 1];
    // LOG <<" " <<buffer[24 * gate + yblk + 2];
    // LOG << LOG.endl;
}

void MN12832L::fillScreen(uint8_t color)
{
    memset(bufferEven /*+bufferOffset*/, color, bufferSize);

    // byte tempBuffer[24] = {
    //     B10000010, B00001000, B00100000, // 4 rows | a
    //     B00100000, B10000010, B00001000, // 4 rows | b
    //     B00001000, B00100000, B10000010, // 4 rows | c
    //     B00000100, B00010000, B01000001, // 4 rows | d
    //     B00010000, B01000001, B00000100, // 4 rows | e
    //     B01000001, B00000100, B00010000, // 4 rows | f
    //     0, 0, 0,
    //     0, 0, 0};

    // copy to some gates...
    // memcpy(bufferEven + 24 * 0, tempBuffer, 24);
    // memcpy(bufferEven + 24 * 1, tempBuffer, 24);
    // memcpy(bufferEven + 24 * 2, tempBuffer, 24);
    // memcpy(buffer + 24 * 3, tempBuffer, 24);
    // memcpy(buffer + 24 * 4, tempBuffer, 24);
    // memcpy(buffer + 24 * 5, tempBuffer, 24);

    // LOG << "mset buffer: " << LOG.hex << (uint32_t)buffer << ": " << LOG.bin << buffer[0] << buffer[1] << buffer[2] << buffer[3] << LOG.endl;
}

void MN12832L::swapBuffers()
{
    noInterrupts();
    // if(bufferOffset)
    // {
    //     bufferOffset = 0;
    // }
    // else
    // {
    //     bufferOffset = bufferSize;
    // }
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

    // bits 192-236 are the gates..
    // but shifter has 48 bits / 6 bytes, use upper 6 bytes of gateBuf / last 5 hex digits unuses !
    if (gate == 0)
        _the->gateBuf = 0x8000000000000000;
    if (gate == 1)
        _the->gateBuf = 0xC000000000000000;
    else if (gate == endstop)
        _the->gateBuf = 0x8000000000100000;
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

    // if(_the->_offset)
    // {
    //     ptr = _the->getBuffer() - bufferSize;
    // }
    // else
    // {
    //     ptr = _the->getBuffer() + bufferSize;
    // }

    // LOG <<"draw buffer: " <<LOG.hex <<(uint32_t)ptr <<": " <<LOG.bin <<*ptr++ <<*ptr++ <<LOG.endl;

    // if (_the->gate % 2 == 1)
    //     return;

    // LOG <<_the->gate <<LOG.endl;
    // spi.transfer will read back into the buffer.. !!! So we use a temp..
    byte tempBuffer[24];
    if (_the->gate % 2 == 1)
    {
        memcpy(tempBuffer, _the->bufferOdd + 24 * (_the->gate/2 + 0), 24); // + 24 * _the->gate;
        // memset(tempBuffer, 0, 24);// skip
    }
    else
    {
        memcpy(tempBuffer, _the->bufferEven + 24 * (_the->gate/2 + 0), 24); // + 24 * _the->gate;
        // memset(tempBuffer, 0, 24);// skip
    }


    uint8_t *ptr = tempBuffer;

    // copy columns from display buffer !
    {

        // LOG <<"data[0]:" <<LOG.bin <<*ptr <<LOG.endl;

        SPI.beginTransaction(settingsA);
        // shift out 24 bytes = 192 bits = 32rows * 3depth * 2colomns
        SPI.transfer(ptr, 24);

        // shift out gates
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
