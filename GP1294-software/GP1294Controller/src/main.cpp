#include <Arduino.h>

#define NO_LED_FEEDBACK_CODE
#define DECODE_NEC
#define USE_NO_SEND_PWM
#include <IRremote.h>

#include <TeensyThreads.h>

#include "config.h"
#include "input.h"
#include "ampctl.h"
#include "display.h"

int threadId = 0;
Threads::Mutex my_lock;

Input input;
AmpControl ampctl;
Display display;

elapsedMillis emChecker = 0;
elapsedMillis emIRevent = 0;

void setup()
{
    Serial.begin(115200);

    Wire1.begin();
    Wire1.setSDA(pinSDA);
    Wire1.setSCL(pinSCL);

    IrReceiver.begin(pinRemote, false, 0); // Start the receiver

    pinMode(pinFFBlank, OUTPUT);
    digitalWrite(pinFFBlank, true); // required to get fillament going, can be dimmed with PWM.
    // analogWrite(pinFFBlank, 128); // makes noise with 4500Hz

    pinMode(pinAmbi, INPUT);

    // 74165 shift register with buttons/encoder inputs
    pinMode(pinSLat, OUTPUT);
    pinMode(pinSCLK, OUTPUT);
    pinMode(pinSIN, INPUT);

    pinMode(pinDBG0, OUTPUT);
    pinMode(pinDBG1, OUTPUT);
    pinMode(pinDBG2, OUTPUT);
    digitalWrite(pinDBG0, LOW);
    digitalWrite(pinDBG1, LOW);
    digitalWrite(pinDBG2, LOW);

    input.begin(50);   // x
    ampctl.begin(100); // :adr
    display.begin(20); // .
}

void loop()
{
    if (emChecker > 5000)
    {
        emChecker = 0;

        Threads::Scope m(my_lock); // lock on creation
        // do stuff...

        Serial.println(threads.threadsInfo());

        // Serial.print("main id: "); Serial.println(threads.id());
        // Serial.print("in   id: "); Serial.println(input.id());
        // Serial.print("amp  id: "); Serial.println(ampctl.id());
        // Serial.print("disp id: "); Serial.println(display.id());

        // 0:Stack size:10240|Used:537329664|Remains:-537319424|State:RUNNING|
        // 1:Stack size:1024|Used:72|Remains:952|State:RUNNING|
        // 2:Stack size:1024|Used:72|Remains:952|State:RUNNING|

        // 0:Stack size:10240|Used:537329664|Remains:-537319424|State:RUNNING|
        // 1:Stack size:1024|Used:72|Remains:952|State:RUNNING|
        // 2:Stack size:1024|Used:72|Remains:952|State:RUNNING|

        // 0:Stack size:10240|Used:537329664|Remains:-537319424|State:RUNNING|
        // 1:Stack size:1024|Used:72|Remains:952|State:RUNNING|
        // 2:Stack size:1024|Used:72|Remains:952|State:RUNNING|
        // 3:Stack size:1024|Used:312|Remains:712|State:RUNNING|
    }

    if(emIRevent > 50 && IrReceiver.decode())
    {
        emIRevent = 0;

        // Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX); // Print "old" raw data
        // IrReceiver.printIRResultShort(&Serial);                       // Print complete received data in one line
        // IrReceiver.printIRSendUsage(&Serial);                         // Print the statement required to send this data
        decode_type_t pro = IrReceiver.lastDecodedProtocol;
        uint32_t adr = IrReceiver.lastDecodedAddress;
        uint32_t com = IrReceiver.lastDecodedCommand;
        if (pro == APPLE)
        {
            Serial.print("ir: ");
            Serial.print(adr);
            Serial.print(" ");
            Serial.print(com);
            Serial.println();

            if (com == 4)
            {
                // snprintf(textbuffer, 100, "> Play/Pause");
                int ambi = analogRead(pinAmbi); // ambient light sensor
                snprintf(display.getTextBuffer(), 100, "Ambi:%d", ambi);
            } // play/pause
            if (com == 2)
            {
                // snprintf(display->getTextBuffer(), 100, "MENU");
                // snprintf(display.getTextBuffer(), 100, "Input:0x%02x", input);
            } // menu
            if (com == 11)
            {
                snprintf(display.getTextBuffer(), 100, "UP ^^^");
            } // up
            if (com == 13)
            {
                snprintf(display.getTextBuffer(), 100, "DOWN :(");
            } // down
            if (com == 8)
            {
                snprintf(display.getTextBuffer(), 100, "<<<");
            } // left
            if (com == 7)
            {
                snprintf(display.getTextBuffer(), 100, ">>>");
            } // right
        }
        IrReceiver.resume(); // Enable receiving of the next value
    }

    // threads.idle();
}
