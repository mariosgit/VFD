

#include <Arduino.h>
#include <SPI.h>
#include <mbLog.h>

#include "mbGFX_MN12832JC.h"

// useable space is 126 width, one coloum missing at the sides
MN12864K canvas;  //  uint8_t *getBuffer(void);

void drawSomething()
{
    //  drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color),

    // canvas.drawLine(0, 4, 3, 4, 1);
    // LOG << "-------------------\n";
    // byte xx = 1;
    // canvas.drawLine(xx,  8, xx,  11, 1);
    // LOG << "-------------------\n";

    // canvas.drawLine(0,    0,   3,  3, 1);
    // canvas.drawLine(122,  0, 122,  3, 1);
    // canvas.drawLine(0,   31,   3, 28, 1);
    // canvas.drawLine(125, 31, 122, 28, 1);
    // canvas.drawLine(0+32,0+32, 31+32,31+32, 1);
    // canvas.drawLine(0+64,0+64, 31+64,31+64, 1);
    // canvas.drawLine(0+96,0+96, 31+96,31+96, 1);

    canvas.setCursor(7,24);
    canvas.print("@mb");
    canvas.setCursor(42,24);
    canvas.print("VFD:MN12832JC");

    canvas.swapBuffers();

    canvas.setCursor(7,24);
    canvas.print("@mb");
    canvas.setCursor(42,24);
    canvas.print("VFD:MN12832JC");

    // canvas.swapBuffers();
}

int i = 0;
int dir = 1;
void drawMore()
{
    // // fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
    canvas.fillRect(36,0,127,23,0);
    canvas.setCursor(36+i, 1+random(3));
    canvas.setTextSize(2);
    canvas.print("MAKER");
    if(i == 0)
        dir = 1;
    if(i == 32)
        dir = -1;
    i = (i + dir);

    int i12 = (i) / 14 ;
    canvas.fillCircle(7,10,7,1);
    canvas.fillCircle(22,10,7,1);
    canvas.fillCircle(7+i12,9+random(3),3,0);
    canvas.fillCircle(22-i12,9+random(3),3,0);
    canvas.drawLine(14,19,15,19,1);
}


/**************************************************************/

void setup()
{
    Serial.begin(115200);
    //Wait for console...
    // while (!Serial);
    // Serial.println("setup - start");

    randomSeed(analogRead(0));

    canvas.begin();
    canvas.fillScreen(0);
    // drawSomething();
    // drawMore();
}

void loop()
{
    canvas.fillScreen(0);
    // canvas.drawLine(0,0,63,63,1);
    // canvas.drawLine(64,0,127,63,1);

    uint8_t *buffer = canvas.getBuffer();
    buffer[0] = 0xfC;
    buffer[1] = 0x0f;
    buffer[2] = 0xc0;

    buffer[3] = 0xfC;
    buffer[4] = 0x0f;
    buffer[5] = 0xc0;

    // drawMore();
    // Serial.println("hallo");

    canvas.swapBuffers();
    delay(1000);
}
