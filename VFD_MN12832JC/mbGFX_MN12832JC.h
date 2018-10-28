/*
Mario Becker, 2018, License:MIT.

Code is for TeensyLC, others not tested! Critical things used:
- IntervalTimer
*/

#pragma once

#include <Adafruit_GFX.h>

class MN12832JC : public Adafruit_GFX {
public:
    MN12832JC();
    ~MN12832JC(void);
    void begin();
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
    virtual void fillScreen(uint16_t color);
    inline uint8_t *getBuffer(void) { return buffer+_offset; }
    void swapBuffers();

private:
    static void nextGate(byte gate);
    static void displayRefresh();
    uint8_t *buffer;
    uint32_t _offset = 0;

    static MN12832JC *_the;
};


