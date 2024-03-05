#include <Arduino.h>

#include <U8g2lib.h>
#include <SPI.h>

U8G2_GP1294AI_256X48_F_4W_HW_SPI u8g2(U8G2_R0, 10, U8X8_PIN_NONE, 9);
// F version has 1536 buf size ! Correct, was machen die anderen ???

void setup()
{
    u8g2.begin();
    u8g2.setContrast(055);
}

void loop()
{


    // u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.drawStr(10, 24, "Hello World!");

    u8g2.drawLine(0, 0, 63, 47);
    u8g2.drawLine(64, 0, 2*64-1, 47);
    u8g2.drawLine(2*64, 0, 3*64-1, 47);
    u8g2.drawLine(3*64, 0, 4*64-1, 47);

    u8g2.drawRBox(150, 30, 100,10, 2);

    u8g2.sendBuffer();
    delay(100);
}

