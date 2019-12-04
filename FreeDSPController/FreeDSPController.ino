#include <Arduino.h>
#include <Wire.h>
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

const byte pinSDA = 18;
const byte pinSCL = 19;

// useable space is 126 width, one coloum missing at the sides
MN12832JC canvas;  //  uint8_t *getBuffer(void);
elapsedMillis encoderTime;
elapsedMillis frameTime;
elapsedMillis textChangeTime;
elapsedMillis peakDecrementTime;

IntervalTimer encoderUpdater;

ClickEncoder encoder1(pinROT1R, pinROT1L, pinROT1C, 4, HIGH); // Bourns, STEC12E08(Alps plastik)
ClickEncoder encoder2(pinROT2R, pinROT2L, pinROT2C, 4, HIGH); // Bourns, STEC12E08(Alps plastik)

int i = 0;
int dir = 1;
int32_t pos1 = 5;
int16_t pos1y = 8;
int16_t pos2 = 0;

// LevelMeter related
bool dcdone = false;
uint8_t data[6*4];
int level1 = 0;
int level2 = 0;
int peakLevel1 = -100;
int peakLevel2 = -100;

void encoderServiceFunc()
{
    encoder1.service();
    encoder2.service();
}

void checkDevice()
{
    // register 0-5 je 4 bytes  pro kanal const,hold,decay
    const byte addr = 0x68 >> 1;

    // ReadBack Addresses..
    // from TEST-DEMO-FILT.xml
    // or   TEST-DEMO-FILT_IC_1_PARAM.h:97
    // and  TEST-DEMO-FILT_IC_1_PARAM.h:75
    const int16_t trapLevel1 = 0x0292;
    const int16_t trapLevel2 = 0x020E;

    // Write to data capture reg 2074 and 2075
    const int16_t DC1 = 2074;
    const int16_t DC2 = 2075;

    if(!dcdone) // as long as we use only these, setup only once
    {
        // setup DC to read LevelMeters
        Wire.beginTransmission(addr);
        Wire.write(DC1 >> 8);  // address high byte
        Wire.write(DC1 & 0xff);  // address low byte
        Wire.write(trapLevel1 >> 8);  // address high byte
        Wire.write(trapLevel1 & 0xff);  // address low byte
        Wire.write(trapLevel2 >> 8);  // address high byte
        Wire.write(trapLevel2 & 0xff);  // address low byte
        Wire.endTransmission();
    }
  
    Wire.beginTransmission(addr);
    Wire.write(DC1 >> 8);  // address high byte
    Wire.write(DC1 & 0xff);  // address low byte
    Wire.endTransmission(false); // no stop before reading !
    // read...
    byte readlen = 6; // 2x3
    Wire.requestFrom(addr, readlen);
    int idx = 0;
    while(Wire.available())    // slave may send less than requested
    {
        data[idx++] = Wire.read();    // receive a byte as character
        if(idx > readlen)
          break;
    }

    if(idx == 6)
    {
        dcdone = true;
        // 24bit 5.19 fixpoint value. The overall shift is needed for negative values.
        int32_t value1 = (((int32_t)data[2] << 8) | ((int32_t)data[1] << 16) | ((int32_t)data[0] << 24)) >> 8;
        int32_t value2 = (((int32_t)data[5] << 8) | ((int32_t)data[4] << 16) | ((int32_t)data[3] << 24)) >> 8;

        // 5.19 fixpoint to float.
        float realval1 = -1.0f * (100.0f - (float)value1 / 5242.880f); // x / (1 << 19)
        float realval2 = -1.0f * (100.0f - (float)value2 / 5242.880f); // x / (1 << 19)
        
        // Output data.
        // Serial.print("Measure: left = ");
        // Serial.print(realval1);
        Serial.print("dB, right:");
        Serial.print(realval2);
        Serial.print("dB, peak:");
        Serial.print(peakLevel2);
        Serial.println();

        level1 = realval1;
        level2 = realval2;
        level1 = (level1<-31)?-31:level1;
        level2 = (level2<-31)?-31:level2;

        peakLevel1 = (level1 > peakLevel1)?level1:peakLevel1;
        peakLevel2 = (level2 > peakLevel2)?level2:peakLevel2;

//        LOG << "i2c length: " <<idx <<" \tbuf:" <<(float)realval1 <<",\t" <<(float)realval2 <<"\n";
    }

    // Volume Control -> only when changed !
    const int16_t volAddr = 172;
    int32_t level = pos1;  // pos1 [0..99]

    // A value of 100 represents a volume level of 1.0 or 0dB.
    // Now convert our percent value to a 5.23 fixpoint as the DSP uses these.
    int32_t val32 = ((1UL << 23) * level) / 100;

    // 0 sollte so was sein
    // <Data>0x00, 0x80, 0x00, 0x00, </Data>
    LOG <<"volume: " <<level <<" " <<val32 <<"\n";

    // Highest byte must be sent first, so reverse byte order.
    uint8_t data[4] = {((uint8_t *)&val32)[3],
                       ((uint8_t *)&val32)[2],
                       ((uint8_t *)&val32)[1],
                       ((uint8_t *)&val32)[0]};

    // Write the value to the DSP param memory.
    Wire.beginTransmission(addr);
    Wire.write(volAddr >> 8);  // address high byte
    Wire.write(volAddr & 0xff);  // address low byte
    Wire.write(data, 4);
    Wire.endTransmission(); // no stop before reading !
}

void setup()
{
    Wire.setSDA(pinSDA);
    Wire.setSCL(pinSCL);
    Wire.begin();
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
        pos1 = (pos1<0)?0:pos1;
        pos1 = (pos1>99)?99:pos1;
        
        pos1y += v2;
        pos1y = (pos1y % 32);

//        if(0) //b1 || v1)
        {
            //Log.notice("Enc1: %d %d, Enc2: %d %d\n", b1, v1, b2, v2);
            // LOG << "enc1:" <<b1 <<" " <<v1 <<pos1   << "  enc2:" <<b2 <<v2 <<"\n";
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
    if(peakDecrementTime > 500)
    {
        peakLevel1--;
        peakLevel2--;
        peakDecrementTime = 0;
    }
    if(frameTime > 20) // draw stuff with 50Hz
    {
        frameTime = 0;

        canvas.fillScreen(0);
        
        canvas.setTextSize(1);
        canvas.setCursor(128+42+pos2, 24);
        canvas.print("FreeDSP Controller @mb");

        // Volume
        int fix = 0;
        fix = (pos1>9)?12:0;
        canvas.setCursor(103-fix, 5);
        canvas.setTextSize(2);
        canvas.print(pos1);
        canvas.print("%");

        // // fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
        /*
        canvas.fillRect(36,0,127,23,0);
        canvas.setCursor(xpos+i, 1+random(3));
        canvas.setTextSize(2);
        canvas.print(tmaker);
        if(i == 0)
            dir = 1;
        if(i == 32)
            dir = -1;
        i = (i + dir);
        */

        int smileX = 8+pos1+32+level1;
        int size = 4 + (32 + level2) / 8;
        int i12 = (i) / 14 ;
        canvas.fillCircle(   smileX,     2+pos1y,size, 1);
        canvas.fillCircle(15+smileX,     2+pos1y,size, 1);
        canvas.fillCircle(   smileX+i12, 1+random(3)+pos1y,3, 0);
        canvas.fillCircle(15+smileX-i12, 1+random(3)+pos1y,3, 0);
        canvas.drawLine  ( 7+smileX,     6+pos1y, 7+smileX,8+pos1y, 1);
        canvas.drawLine  ( 5+smileX,     11+pos1y, 9+smileX,11+pos1y, 1);

        // LOG <<"disp time:" <<canvas.getDisplayTime() <<"us\n";

        pos2 -= 1;
        pos2 = (pos2 % 300);

        checkDevice();

        canvas.fillRect(0,0,16,31,0);
        canvas.drawLine(0,-peakLevel1, 5,-peakLevel1, 1);
        canvas.drawLine(7,-peakLevel2,12,-peakLevel2, 1);
        canvas.fillRect(0,-level1, 2, 32+level1, 1);
        canvas.fillRect(7,-level2, 2, 32+level2, 1);

        canvas.swapBuffers();
    }
}

