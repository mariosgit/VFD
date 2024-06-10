#include "config.h"
#include "ampctl.h"

void AmpControl::begin(int delayTime)
{
    this->delayTime = delayTime;
    threadId = threads.addThread((ThreadFunction)(&AmpControl::run), (void *)this);
}

void AmpControl::run()
{
    byte i2cScanAdr = 1;
    byte error;
    int nDevices;

    // // just some output to check the port.
    // Wire1.beginTransmission(0xa7);
    // Wire1.write("subermajo");
    // Wire1.endTransmission();


    while (1)
    {
        // Serial.print("Scanning...");
        Serial.print(":");
        Serial.println(i2cScanAdr);

        nDevices = 0;
        // for (i2cScanAdr = 1; i2cScanAdr < 127; i2cScanAdr++)
        {
            // The i2c_scanner uses the return value of
            // the Write.endTransmisstion to see if
            // a device did acknowledge to the address.
            Wire1.beginTransmission(i2cScanAdr);
            error = Wire1.endTransmission();

            if (error == 0)
            {
                Serial.print("I2C device found at address 0x");
                if (i2cScanAdr < 16)
                    Serial.print("0");
                Serial.print(i2cScanAdr, HEX);
                Serial.println("  !");

                nDevices++;
            }
            else if (error == 4)
            {
                // Serial.print("Unknown error at address 0x");
                if (i2cScanAdr < 16)
                    Serial.print("0");
                Serial.println(i2cScanAdr, HEX);
            }
        }
        // if (nDevices == 0)
        //     Serial.println("No I2C devices found\n");
        // else
        //     Serial.println("done\n");

        i2cScanAdr = (i2cScanAdr + 1) % 128;

        threads.delay(this->delayTime);
    }
}
