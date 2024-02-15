/// A GFX 1-bit canvas context for graphics

/// double buffer:
///   buffer+offset is the one to draw, the other will be for display

#include <SPI.h>
#include "mbGFX_MN12832L.h"
#include <mbLog.h>

// display spi kann bis 200ns/5MHz geht aber auch mit 16 noch ???
SPISettings settingsA(16000000, LSBFIRST, SPI_MODE0);

// display port
const byte pinBLK = 4;
const byte pinLAT = 6;
const byte pinGCP = 3;
const byte MOSI_PIN = 7;
const byte SCK_PIN = 14;

// pwm to control filament driver
const byte pinPWM = 20;

// Create an IntervalTimer object 
#ifdef __STM32F1__
HardwareTimer timer(2);
#else
IntervalTimer myTimer;
#endif

MN12832L *MN12832L::_the  = nullptr;

MN12832L::MN12832L() : Adafruit_GFX(128, 32),
    _gate(50),
    _displayTime(0)
{
    uint16_t bytes = getBufferSize();
    if((buffer = (uint8_t *)malloc(bytes*2))) {
        memset(buffer, 0, bytes*2);
    }
    LOG << "MN12832L:: allocated 2x " <<bytes <<"B screen buffer" <<LOG.endl;
    _the = this;
}

MN12832L::~MN12832L(void) {
    if(buffer) free(buffer);
}

void MN12832L::begin()
{
    pinMode(pinBLK , OUTPUT);
    pinMode(pinGCP , OUTPUT);
    pinMode(pinLAT, OUTPUT);

    digitalWrite(pinBLK, HIGH);
    digitalWrite(pinLAT, LOW);
    digitalWrite(pinGCP, LOW);

    SPI.setMOSI(MOSI_PIN);
    SPI.setSCK(SCK_PIN);
    SPI.begin();

    pinMode(pinPWM , OUTPUT);
    analogWrite(pinPWM, 128); // 50% duty
    
    // display needs to cygle all the time !
    myTimer.begin(displayRefresh, 1000000); // starting slowly
}

void MN12832L::drawPixel(int16_t x, int16_t y, uint16_t color) {
#ifdef __AVR__
    // Bitmask tables of 0x80>>X and ~(0x80>>X), because X>>Y is slow on AVR
    static const uint8_t PROGMEM
        GFXsetBit[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 },
        GFXclrBit[] = { 0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE };
#endif

    if(buffer)
    {
        if((x < 0) || (y < 0) || (x >= _width) || (y >= _height)) return;

        register uint8_t    xU3 = x & 0x3;
        // register uint8_t   s1s2 = (xU3==0 || xU3==3) ? 0 : 1;
        register uint8_t    adr = x/2; // (((x/4) << 1) | s1s2);
        register uint8_t   *ptr = &buffer[adr*8+_offset];
        register uint8_t offset = y/4;

        ptr += offset;

        register uint8_t bitpos = (y%4 << 1);
        bitpos += (xU3 == 2 || xU3 == 0) ? 1 : 0;

        if(color) *ptr |=   0x80 >> (bitpos & 7);
        else      *ptr &= ~(0x80 >> (bitpos & 7));
    }
}

void MN12832L::fillScreen(uint16_t color)
{
    if(buffer) {
        uint16_t bytes = getBufferSize();
        memset(buffer+_offset, color ? 0xFF : 0x00, bytes);
    }
}

void MN12832L::swapBuffers()
{
    noInterrupts();
    uint16_t bytes = getBufferSize(); // size of one screen
    if(_offset)
    {
        _offset = 0;
    }
    else
    {
        _offset = bytes;
    }
    interrupts();
}

uint32_t MN12832L::getDisplayTime()
{
    uint32_t result;
    noInterrupts();
    result = _the->_displayTime;
    interrupts();
    return result;
}

void MN12832L::nextGate()
{
    uint8_t gate = _the->_gate + 1;

    if(gate > 43)
        gate = 0;

    // bits 192-236 are the gates..  
    // but shifter has 48 bits / 6 bytes, use upper 6 bytes of gateBuf / last 5 hex digits unuses !
    if(gate == 0)
        _the->_gateBuf = 0xC000000000000000;
    else if(gate == 43)
        _the->_gateBuf = 0x8000000000100000;
    else 
        _the->_gateBuf = _the->_gateBuf >> 1;

    _the->_gate = gate;

    // LOG <<"gate:" <<LOG.dec <<gate <<LOG.hex <<" gb:" <<_the->_gateBuf <<LOG.endl;
}

void MN12832L::displayRefresh()
{
    uint32_t time = micros();

    byte* drawBuffer;
    uint16_t bytes = _the->getBufferSize();
    if(_the->_offset)
    {
        drawBuffer = _the->getBuffer() - bytes;
    }
    else
    {
        drawBuffer = _the->getBuffer() + bytes;
    }

    // copy rows from display buffer !
    {
        nextGate();

        uint8_t *ptr;
        ptr = drawBuffer ;//+ _the->_gate * 24;
        LOG <<"data[0]:" <<LOG.bin <<*ptr <<LOG.endl;

        // toggle GCP ???
        digitalWrite(pinGCP, HIGH);
        digitalWrite(pinGCP, LOW);

        SPI.beginTransaction(settingsA);
        // shift out 24 bytes = 192 bits = 32rows * 3depth * 2colomns
        SPI.transfer(ptr, 24);


        // toggle GCP ???
        digitalWrite(pinGCP, HIGH);
        digitalWrite(pinGCP, LOW);


        // shift out gates
        // LOG <<"gate:" <<LOG.dec <<_the->_gate <<LOG.hex <<" gb:\t" <<_the->_gateBuf <<LOG.endl;
        union u64u8 {
            uint64_t u64;
            uint8_t u8[8];
        } gbuf = {_the->_gateBuf};
        ptr = gbuf.u8;
        ptr += 7;

        // toggle GCP ???
        digitalWrite(pinGCP, HIGH);
        digitalWrite(pinGCP, LOW);

        // LOG <<LOG.hex <<"         :\t";
        for(int i =0; i< 6; i++)
        {
            SPI.transfer(*ptr);
            // LOG <<*ptr;
            ptr--;
        }
        // LOG <<LOG.endl;
        SPI.endTransaction();

        // toggle GCP ???
        digitalWrite(pinGCP, HIGH);
        digitalWrite(pinGCP, LOW);

        // toggle GCP ???
        digitalWrite(pinGCP, HIGH);
        digitalWrite(pinGCP, LOW);


        digitalWrite(pinBLK, HIGH);

        digitalWrite(pinLAT, HIGH);
        digitalWrite(pinLAT, LOW);

        digitalWrite(pinBLK, LOW);

    }
    // shift the gates out...
    _the->_displayTime = micros() - time;

    // special case gate 44 should turn on gate 1
}
