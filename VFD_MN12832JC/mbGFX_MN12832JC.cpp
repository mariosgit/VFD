/// A GFX 1-bit canvas context for graphics

/// double buffer:
///   buffer+offset is the one to draw, the other will be for display

#include <SPI.h>
#include "mbGFX_MN12832JC.h"

// display spi kann bis 200ns/5MHz geht aber auch mit 16 noch ???
SPISettings settingsA(16000000, LSBFIRST, SPI_MODE0);

const byte pinBLK1 = 4;
const byte pinBLK2 = 5;
const byte pinLAT12 = 6;
const byte pinGBLK = 3;
const byte pinGLAT = 2;
const byte pinGCLK = 8;
const byte pinGSIN = 9;
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14

const byte pinPWM = 20;

// Create an IntervalTimer object 
IntervalTimer myTimer;


byte *ptr;
unsigned long displayTime;


MN12832JC *MN12832JC::_the = nullptr;


MN12832JC::MN12832JC() : Adafruit_GFX(128, 32) {
    uint16_t bytes = ((128 + 7) / 8) * 32;
    if((buffer = (uint8_t *)malloc(bytes*2))) {
        memset(buffer, 0, bytes*2);
    }
    _the = this;
}

MN12832JC::~MN12832JC(void) {
    if(buffer) free(buffer);
}

void MN12832JC::begin()
{
    pinMode(pinBLK1 , OUTPUT);
    pinMode(pinBLK2 , OUTPUT);
    pinMode(pinLAT12, OUTPUT);

    pinMode(pinGBLK , OUTPUT);
    pinMode(pinGLAT , OUTPUT);
    pinMode(pinGCLK , OUTPUT);
    pinMode(pinGSIN , OUTPUT);

    digitalWrite(pinBLK1, HIGH);
    digitalWrite(pinBLK2, HIGH);
    digitalWrite(pinLAT12, LOW);

    digitalWrite(pinGBLK, HIGH);
    digitalWrite(pinGLAT, LOW);
    digitalWrite(pinGCLK, HIGH);
    digitalWrite(pinGSIN, LOW);

    SPI.setMOSI(SDCARD_MOSI_PIN);
    SPI.setSCK(SDCARD_SCK_PIN);
    SPI.begin();

    analogWrite(pinPWM, 128);
    
    digitalWrite(pinGSIN, LOW);
    for(byte i = 0; i < 64; i++)
    {
        digitalWrite(pinGCLK, LOW);
        digitalWrite(pinGCLK, HIGH);
    }
    digitalWrite(pinGLAT, HIGH);
    digitalWrite(pinGLAT, LOW);

    SPI.beginTransaction(settingsA);

    myTimer.begin(displayRefresh, 2000);

}


void MN12832JC::drawPixel(int16_t x, int16_t y, uint16_t color) {
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

void MN12832JC::fillScreen(uint16_t color)
{
    if(buffer) {
        uint16_t bytes = ((WIDTH + 7) / 8) * HEIGHT;
        memset(buffer+_offset, color ? 0xFF : 0x00, bytes);
    }
}

void MN12832JC::swapBuffers()
{
    noInterrupts();
    uint16_t bytes = ((_width + 7) / 8) * _height;
    if(_offset)
    {
        _offset = 0;
    }
    else
    {
        _offset = bytes;
    }

    unsigned long time = displayTime;
    interrupts();
    // LOG << "dispTime:" <<time <<"us\n";
    // 1500us, mhm not so bad

}

void MN12832JC::nextGate(byte gate)
{
    if(gate < 2)
        digitalWrite(pinGSIN, HIGH);
    else
        digitalWrite(pinGSIN, LOW);
    // clk
    digitalWrite(pinGCLK, LOW);
    digitalWrite(pinGCLK, HIGH);
    // latch
    digitalWrite(pinGLAT, HIGH);
    digitalWrite(pinGLAT, LOW);

    digitalWrite(pinGBLK, LOW);
}

void MN12832JC::displayRefresh()
{
    unsigned long time = micros();

    uint32_t bytes = ((_the->width() + 7) / 8) * _the->height();
    byte* drawBuffer;
    if(_the->_offset)
    {
        drawBuffer = _the->getBuffer() - bytes;
    }
    else
    {
        drawBuffer = _the->getBuffer() + bytes;
    }

    for(byte gate = 0; gate < 64; )  // ein gate = 2 x pixel
    {
        ptr = drawBuffer + gate*8 -8;
        for(byte i = 0; i < 8; i++)
        {
            // if(gate & 0x01 && i==3)
            //     SPI.transfer( ~(0x01 << (gate-1)) );
            if(gate < 1)
                SPI.transfer(0); // 1 Spalte ganz links is komisch.
            else
                SPI.transfer(ptr[7-i]);
        }

        digitalWrite(pinBLK1, HIGH);
        digitalWrite(pinBLK2, HIGH);
        nextGate(gate);

        // volatile int bla;
        // for(int i = 0; i < 100; i++)
        //     bla = i;

        digitalWrite(pinLAT12, HIGH);
        digitalWrite(pinLAT12, LOW);

        if(gate & 0x01)
        {
            // cb
            digitalWrite(pinBLK1, HIGH);
            digitalWrite(pinBLK2, LOW);
        }
        else
        {
            // ad
            digitalWrite(pinBLK1, LOW);
            digitalWrite(pinBLK2, HIGH);
        }

        gate++;
    }
    // shift the gates out...
    nextGate(64);
    nextGate(65);
    displayTime = micros() - time;;
}
