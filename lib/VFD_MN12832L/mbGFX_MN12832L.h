/*
Mario Becker, 2018, License:MIT.

Code is for TeensyLC, others not tested! Critical things used:
- IntervalTimer
*/

#pragma once

#include <Adafruit_GFX.h>

class MN12832L : public Adafruit_GFX {
public:
    MN12832L();
    ~MN12832L(void);
    void begin();
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
    virtual void fillScreen(uint16_t color);
    inline uint8_t *getBuffer(void) { return buffer+_offset; }
    void swapBuffers();

    static uint32_t getDisplayTime();  //24us

private:
    uint16_t getBufferSize() { return (_width / 8 * 3) * _height; }
    static void nextGate();
    static void displayRefresh(); // timer isr
    uint8_t *buffer;
    uint32_t _offset = 0;

    uint8_t  _gate;
    uint64_t _gateBuf;
    uint32_t _displayTime;

    static MN12832L *_the;
};


