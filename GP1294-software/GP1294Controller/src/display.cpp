#include "config.h"
#include "display.h"

Display::Display() :
    u8g2(U8G2_R0, pinCS, U8X8_PIN_NONE, pinReset)
{
}

void Display::begin(int delayTime)
{
    u8g2.begin();
    u8g2.setContrast(50);

    this->delayTime = delayTime;
    threadId = threads.addThread((ThreadFunction)(&Display::run), (void *)this);
    threads.setTimeSlice(threadId, 10); // set max time for display to 10 !? // no need to yield ?
}

void Display::run()
{
    while (1)
    {
        digitalWrite(pinDBG1, HIGH);

        Serial.print(".");
        // Serial.print(frameTime);
        frameTime = 0;
        // Serial.print(threadId); // 1

        u8g2.clearBuffer();
        u8g2.setDrawColor(1);
        u8g2.setFont(u8g2_font_courB18_tf);
        // u8g2.setColorIndex
        u8g2.drawStr(8, 24, textbuffer);

        // u8g2.drawHLine(10,28,120);
        // u8g2.drawHLine(10,30,100);
        // u8g2.drawHLine(10,32,80);
        // u8g2.drawHLine(10,34,60);
        // u8g2.drawHLine(10,36,40);

        u8g2.setDrawColor(0);
        u8g2.drawBox(194, 4, 56, 20);
        u8g2.setDrawColor(1);
        u8g2.setFont(u8g2_font_unifont_t_symbols);
        u8g2.drawGlyph(textPos, 22, 0x23f0); /* dec 9731/hex 2603 Snowman */

        u8g2.drawRFrame(0, 0, 255, 47, 2);
        u8g2.drawRBox(150, 30, 255 - 150 - 4, 10, 2);

        u8g2.setDrawColor(0);
        u8g2.setFont(u8g2_font_tinytim_tf);
        u8g2.drawStr(158, 38, "GP1294AI rocks !!!");

        textPos += textAdd;
        if (textPos > 233)
            textAdd = -1;
        if (textPos < 195)
            textAdd = 1;

        // levels !?
        levelR[levelPos] = random(120);
        levelL[levelPos] = random(120);

        uint16_t levelLavr = 0;
        uint16_t levelRavr = 0;
        for (int i = 0; i < 4; i++)
        {
            if (levelR[i] > levelRmax)
                levelRmax = levelR[i];
            levelRavr += levelR[i];
        }
        for (int i = 0; i < 4; i++)
        {
            if (levelL[i] > levelLmax)
                levelLmax = levelL[i];
            levelLavr += levelL[i];
        }

        // u8g2.setDrawColor(0);
        // u8g2.drawBox(4, 28, 124, 20);
        u8g2.setDrawColor(1);
        u8g2.drawBox(levelLmax & 0xfc, 28 + 0, 4, 6);
        u8g2.drawBox(levelRmax & 0xfc, 28 + 8, 4, 6);
        u8g2.drawBox(8, 28 + 0, (levelLavr / 4) & 0xfc, 6);
        u8g2.drawBox(8, 28 + 8, (levelRavr / 4) & 0xfc, 6);

        u8g2.setDrawColor(0);
        for (int i = 12; i < 128; i += 4)
        {
            u8g2.drawVLine(i, 28, 15);
        }

        levelPos = (levelPos + 1) % 4;

        levelLmax = (levelLmax > 4) ? levelLmax - 4 : 0;
        levelRmax = (levelRmax > 4) ? levelRmax - 4 : 0;

        u8g2.sendBuffer();

        digitalWrite(pinDBG1, LOW);

        // threads.delay(delayTime); // can run max fps ?
    }
}
