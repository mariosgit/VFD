#include "config.h"
#include "input.h"

void Input::begin(int delayTime)
{
    this->delayTime = delayTime;
    threadId = threads.addThread((ThreadFunction)(&Input::run), (void *)this);
}

void Input::run()
{
    while (1)
    {
        Serial.print("x"); // geht

        digitalWrite(pinDBG0, HIGH);
        uint16_t result = 0;
        digitalWrite(pinSLat, LOW);
        digitalWrite(pinSLat, HIGH);
        for (int bitt = 0; bitt < 16; bitt++)
        {
            // I get /Q as input !
            result |= (digitalRead(pinSIN)) ? 0 : 1 << bitt;
            digitalWrite(pinSCLK, LOW);
            digitalWrite(pinSCLK, HIGH);
        }
        input = result;
        digitalWrite(pinDBG0, LOW);
        threads.delay(this->delayTime);
    }
}
