

#include <Arduino.h>
#include <SPI.h>
#include <mbLog.h>
#include <elapsedMillis.h>

#include "mbGFX_MN12832JC.h"

// useable space is 126 width, one coloum missing at the sides
MN12832JC canvas;  //  uint8_t *getBuffer(void);
elapsedMillis frameTime;
elapsedMillis textChangeTime;

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
}

void setup()
{
    Serial.begin(115200);
    //Wait for console...
    // while (!Serial);
    // Serial.println("setup - start");

    randomSeed(analogRead(0));

    canvas.begin();
    drawSomething();

}

/**************************************************************/



int i = 0;
int dir = 1;
void loop()
{
    byte xpos = 36;
    char* tmaker = "MAKER";
    if(textChangeTime > 1000)
    {
        xpos = 50;
        tmaker = "was";
    }
    if(textChangeTime > 2000)
    {
        xpos = 36;
        tmaker = "here!";
    }
    if(textChangeTime > 3000)
    {
        xpos = 48;
        tmaker = "8-P";
    }
    if(textChangeTime > 4000)
    {
        textChangeTime = 0;
    }
    if(frameTime > 20) // draw stuff with 50Hz
    {
        frameTime = 0;
        // // fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
        canvas.fillRect(36,0,127,23,0);
        canvas.setCursor(xpos+i, 1+random(3));
        canvas.setTextSize(2);
        canvas.print(tmaker);
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

        canvas.swapBuffers();

        LOG <<"disp time:" <<canvas.getDisplayTime() <<"us\n";
    }
}
