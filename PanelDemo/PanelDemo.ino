/**
 * @author Mario Becker 2019
 * Sketch to work with the control panel in the PCB-Eagle dir.
 * In addition to the display, it has 2 encoders and 2 flip switches.
 */

#include <Arduino.h>
#include <SPI.h>
#include <mbLog.h>
#include <elapsedMillis.h>
#include <ClickEncoder.h>
#include <ArduinoLog.h>

#include "mbGFX_MN12832JC.h"


// Encoders
const byte pinROT1L = 15;
const byte pinROT1R = 16;
const byte pinROT1C = 11;
const byte pinROT2L = 10;
const byte pinROT2R = 17;
const byte pinROT2C = 12;

const byte pinSWRU = 22;
const byte pinSWRD = 23;
const byte pinSWLU = 0;
const byte pinSWLD = 1;

// useable space is 126 width, one coloum missing at the sides
MN12832JC canvas;  //  uint8_t *getBuffer(void);
elapsedMillis encoderTime;
elapsedMillis frameTime;
elapsedMillis textChangeTime;

IntervalTimer encoderUpdater;

ClickEncoder encoder1(pinROT1R, pinROT1L, pinROT1C, 4, HIGH); // Bourns, STEC12E08(Alps plastik)
ClickEncoder encoder2(pinROT2R, pinROT2L, pinROT2C, 4, HIGH); // Bourns, STEC12E08(Alps plastik)

int i = 0;
int dir = 1;
int16_t pos1 = 0;
int16_t pos1y = 8;
int16_t pos2 = 0;

void encoderServiceFunc()
{
    encoder1.service();
    encoder2.service();
}

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

    canvas.fillScreen(0);
    
    canvas.setTextSize(1);
    canvas.setCursor(128+7+pos2,24);
    canvas.print("@mb");
    canvas.setCursor(128+42+pos2, 24);
    canvas.print("VFD:MN12832JC");
/*
    canvas.swapBuffers();

    canvas.setCursor(7,24);
    canvas.print("@mb");
    canvas.setCursor(42,24);
    canvas.print("VFD:MN12832JC");
*/
}

void setup()
{
    Serial.begin(115200);
    //Wait for console...
    // while (!Serial);
    // Serial.println("setup - start");

    randomSeed(analogRead(0));

    pinMode(pinROT1L, INPUT); // INPUT_PULLUP
    pinMode(pinROT1R, INPUT);
    pinMode(pinROT1C, INPUT);
    pinMode(pinROT2L, INPUT);
    pinMode(pinROT2R, INPUT);
    pinMode(pinROT2C, INPUT);

    pinMode(pinSWRU, INPUT);
    pinMode(pinSWRD, INPUT);
    pinMode(pinSWLU, INPUT);
    pinMode(pinSWLD, INPUT);

    canvas.begin();
    drawSomething();

    encoder1.setAccelerationEnabled(true);
    encoder2.setAccelerationEnabled(true);
    
    encoderUpdater.begin(encoderServiceFunc, 750);
}

/**************************************************************/

void loop()
{
    if(encoderTime > 100)
    {
        encoderTime = 0;
        
        int16_t b1 = encoder1.getButton();
        int16_t b2 = encoder2.getButton();
        int16_t v1 = encoder1.getValue();
        int16_t v2 = encoder2.getValue();

        pos1 += v1;
        pos1 = (pos1 % 120);
        
        pos1y += v2;
        pos1y = (pos1y % 32);

//        if(0) //b1 || v1)
        {
            //Log.notice("Enc1: %d %d, Enc2: %d %d\n", b1, v1, b2, v2);
            LOG << "enc1:" <<b1 <<" " <<v1 <<pos1   << "  enc2:" <<b2 <<v2 <<"\n";
        }
//        if(b2 || v2)
        {
            // Log.notice("Enc2: %d %d\n", b2, v2);
            // LOG << "enc1:" <<v2    << "enc2:" <<v2 <<"\n";
        }
        // ClickEncoder::Clicked:
                    // VERBOSECASE(ClickEncoder::Pressed);
            // VERBOSECASE(ClickEncoder::Held)
            // VERBOSECASE(ClickEncoder::Released)


    }
    
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

        drawSomething();
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
        canvas.fillCircle(7+pos1,2+pos1y,7,1);
        canvas.fillCircle(22+pos1,2+pos1y,7,1);
        canvas.fillCircle(7+pos1+i12,1+random(3)+pos1y,3,0);
        canvas.fillCircle(22+pos1-i12,1+random(3)+pos1y,3,0);
        canvas.drawLine(14+pos1,11+pos1y, 15+pos1,11+pos1y, 1);

        canvas.swapBuffers();

        // LOG <<"disp time:" <<canvas.getDisplayTime() <<"us\n";

        pos2 -= 1;
        pos2 = (pos2 % 300);

    }
}

