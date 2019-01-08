/// A GFX 1-bit canvas context for graphics

/// double buffer:
///   buffer+offset is the one to draw, the other will be for display

#include <Arduino.h>
#include <SPI.h>
#include <mbLog.h>

#include "mbGFX_MN12832JC.h"

// display spi kann bis 200ns/5MHz geht aber auch mit 16 noch ???
#ifdef __STM32F1__
#else
SPISettings settingsA(16000000, LSBFIRST, SPI_MODE0);
#endif

const byte pinBLK1 = PB1;
const byte pinLAT1 = PB0;

const byte pinGBLK = PA2;
const byte pinGLAT = PA3;
const byte pinGCLK = PB10; // A4 -> no output might be related to SPI function
const byte pinGSIN = PA1; // A6 = MISO seems to be used

const byte pinPWM = 20;
#ifdef __STM32F1__
// SCLK=PA5 MOSI=PA7
#else
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14
#endif

// Create an IntervalTimer object 
#ifdef __STM32F1__
HardwareTimer timer(2);
#else
IntervalTimer myTimer;
#endif

byte *ptr;
unsigned long displayTime;


MN12864K *MN12864K::_the = nullptr;


MN12864K::MN12864K() : Adafruit_GFX(128, 64) {
    uint16_t bytes = ((128 + 7) / 8) * 64;
    if((buffer = (uint8_t *)malloc(bytes*2))) {
        memset(buffer, 0, bytes*2);
    }
    _the = this;
}

MN12864K::~MN12864K(void) {
    if(buffer) free(buffer);
}

void MN12864K::begin()
{
    pinMode(pinBLK1, OUTPUT);
    pinMode(pinLAT1, OUTPUT);

    pinMode(pinGBLK , OUTPUT);
    pinMode(pinGLAT , OUTPUT);
    pinMode(pinGCLK , OUTPUT);
    pinMode(pinGSIN , OUTPUT);

    digitalWrite(pinBLK1, HIGH);
    digitalWrite(pinLAT1, LOW);

    digitalWrite(pinGBLK, HIGH);
    digitalWrite(pinGLAT, LOW);
    digitalWrite(pinGCLK, HIGH);
    digitalWrite(pinGSIN, LOW);

    digitalWrite(pinGSIN, LOW);
    for(byte i = 0; i < 64; i++)
    {
        digitalWrite(pinGCLK, LOW);
        digitalWrite(pinGCLK, HIGH);
    }
    digitalWrite(pinGLAT, HIGH);
    digitalWrite(pinGLAT, LOW);

#ifdef __STM32F1__
    SPI.begin(); //Initiallize the SPI 1 port.
    SPI.setBitOrder(LSBFIRST); // Set the SPI-1 bit order (*)  
    SPI.setDataMode(SPI_MODE0); //Set the  SPI-1 data mode (**)  
    SPI.setClockDivider(SPI_CLOCK_DIV8);		// Slow speed (72 / 16 = 4.5 MHz SPI speed)    timer.pause();
    timer.setPeriod(100000); // in microseconds
    timer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
    timer.setCompare(TIMER_CH1, 1);  // Interrupt 1 count after each update
    timer.attachCompare1Interrupt(displayRefresh);
    timer.refresh();
    timer.resume();
#else
    analogWrite(pinPWM, 128);
    
    SPI.setMOSI(SDCARD_MOSI_PIN);
    SPI.setSCK(SDCARD_SCK_PIN);
    SPI.begin();

    SPI.beginTransaction(settingsA);
    myTimer.begin(displayRefresh, 2000);
#endif

}

void MN12864K::drawPixel(int16_t x, int16_t y, uint16_t color) {
    LOG << "drawPixel " << x <<"," << y <<"\n";
    if(buffer)
    {
        if((x < 0) || (y < 0) || (x >= _width) || (y >= _height)) return;

        register uint8_t adr = x/6 * 6*8; // (((x/4) << 1) | s1s2);
        register uint8_t xr = x%6;
        register uint8_t   *ptr = &buffer[adr+_offset];
        register uint8_t offset = y/4;

        register uint16_t bitpos = (y*6 + xr);
        register uint16_t bytepos = bitpos / 8;
        bitpos = bitpos % 8;

        LOG <<"  adr:" <<adr <<", bitpos:" <<bitpos <<" bytepos:" <<bytepos <<"\n";

        ptr += bytepos;

        if(color) *ptr |=   0x80 >> (bitpos & 7);
        else      *ptr &= ~(0x80 >> (bitpos & 7));
    }
}

void MN12864K::fillScreen(uint16_t color)
{
    if(buffer) {
        uint16_t bytes = ((WIDTH + 7) / 8) * HEIGHT;
        memset(buffer+_offset, color ? 0xFF : 0x00, bytes);
    }
}

void MN12864K::swapBuffers()
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

void MN12864K::nextGate(byte gate)
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

// byte gate = 0;
void MN12864K::displayRefresh()
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

    for(byte gate = 0; gate < 44; )  // ein gate = 2 x pixel
    {
        ptr = drawBuffer + (gate/2)*8*6 -8;
        for(byte i = 0; i < 8*6; i++)
        {
            // if(gate & 0x01 && i==3)
            //     SPI.transfer( ~(0x01 << (gate-1)) );
            // if(i < 10)
            //     SPI.transfer(0xff); //ptr[7-i]);
            // else
                SPI.transfer(B01010101 & ptr[i]);
        }

        digitalWrite(pinBLK1, HIGH);
        nextGate(gate);

        digitalWrite(pinLAT1, HIGH);
        digitalWrite(pinLAT1, LOW);

        digitalWrite(pinBLK1, LOW);

        // ---------------------------------
        gate++;
        // ---------------------------------

        for(byte i = 0; i < 8*6; i++)
        {
            // if(gate & 0x01 && i==3)
            //     SPI.transfer( ~(0x01 << (gate-1)) );
            // if(i < 5)
            //     SPI.transfer(0xFF); //ptr[7-i]);
            // else
                SPI.transfer(B10101010 & ptr[i]);
        }

        digitalWrite(pinBLK1, HIGH);
        nextGate(gate);

        digitalWrite(pinLAT1, HIGH);
        digitalWrite(pinLAT1, LOW);

        digitalWrite(pinBLK1, LOW);

        // ---------------------------------

        gate++;
        // gate = (gate % 22);

        // delay(10);
    }
    // shift the gates out...
    // nextGate(64);
    // nextGate(65);
    displayTime = micros() - time;;
}
