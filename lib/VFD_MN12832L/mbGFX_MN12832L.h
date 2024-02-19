/*
Mario Becker, 2018, License:MIT.

Code is for TeensyLC, others not tested! Critical things used:
- IntervalTimer
*/

#pragma once

#include <Adafruit_GFX.h>

class MN12832L : public Adafruit_GFX
{
public:
    MN12832L(
        byte pinBLK = 4,
        byte pinLAT = 6,
        byte pinGCP = 3,
        byte MOSI_PIN = 11,
        byte SCK_PIN = 13,
        byte pinPWM = 20);
    ~MN12832L(void);
    void begin();
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
    virtual void fillScreen(uint8_t color);
    void swapBuffers();

    static uint32_t getDisplayTime(); // 24us
    static uint32_t getDisplayFps1(); // 24us

private:
    static void nextGate();
    static void displayRefresh();                   // timer isr
    static const int16_t bufferSize = 136 / 8 * 32; // black'n'white, + margin

    // bufferlayout
    ///   front | back
    ///   0 | 1 | 0 | 1 //bitplanes
    uint8_t buffer[bufferSize * 4];
    uint16_t bufferOffset = 0;
    uint8_t tempBuffer[30];

    union u32u4
    {
        uint64_t u32;
        uint8_t u4[4];
    };

    union u64u8
    {
        uint64_t u64;
        uint8_t u8[8];
    };
    u64u8 gateBuf;
    uint8_t gate;
    uint32_t displayTime;
    uint32_t displayLast;
    uint32_t displayFps1;

    uint32_t loadLast;
    uint32_t loadFps1;

    const byte pinBLK;
    const byte pinLAT;
    const byte pinGCP;
    const byte MOSI_PIN;
    const byte SCK_PIN;
    const byte pinPWM;
    static MN12832L *_the;
};
