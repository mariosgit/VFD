#pragma once

#include "config.h"

#include <U8g2lib.h>


/**
 * Will do the display drawing.
 * Uses SPI !
*/
class Display
{
public:
    Display();
    void begin(int delayTime = 100);
    inline int id() {return threadId;}
    inline char* getTextBuffer() {return textbuffer;}

private:
    void run();

    int delayTime = 100;

    int threadId = 0;

    // (const u8g2_cb_t *rotation, uint8_t cs, uint8_t dc, uint8_t reset = (uint8_t)255U)
    U8G2_GP1294AI_256X48_F_4W_HW_SPI u8g2;
    // F version has 1536 buf size ! Correct, was machen die anderen ???

    char textbuffer[100] = "Hello World!";

    int textPos = 200;
    int textAdd = 1;

    uint8_t levelR[4];
    uint8_t levelL[4];
    int levelPos = 0;

    uint8_t levelRmax = 0;
    uint8_t levelLmax = 0;

    elapsedMillis frameTime = 0;
};
