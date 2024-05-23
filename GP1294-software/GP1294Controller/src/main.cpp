#include <Arduino.h>

#define NO_LED_FEEDBACK_CODE
#define DECODE_NEC
#define USE_NO_SEND_PWM
#include <IRremote.h>

#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>

const byte pinCS = 10;
const byte pinReset = 9;
const byte pinFFBlank = 23; //=A9;
const byte pinAmbi = 22;    // ambient light, analog value
const byte pinRemote = 21;  // LIRC input pin, sensor=4838

const byte pinSDA = 17; // A3
const byte pinSCL = 16; // A2

const byte pinSLat = 3;
const byte pinSCLK = 4;
const byte pinSIN  = 5;

// (const u8g2_cb_t *rotation, uint8_t cs, uint8_t dc, uint8_t reset = (uint8_t)255U)
U8G2_GP1294AI_256X48_F_4W_HW_SPI u8g2(U8G2_R0, pinCS, U8X8_PIN_NONE, pinReset);
// F version has 1536 buf size ! Correct, was machen die anderen ???

elapsedMillis elapsedRedraw = 0;

int textPos = 200;
int textAdd = 1;

uint8_t levelR[4];
uint8_t levelL[4];
int levelPos = 0;

uint8_t levelRmax = 0;
uint8_t levelLmax = 0;

char textbuffer[100] = "Hello World!";

void setup()
{
    Serial.begin(115200);

    Wire1.begin();
    Wire1.setSDA(pinSDA);
    Wire1.setSCL(pinSCL);

    u8g2.begin();
    u8g2.setContrast(5);
    IrReceiver.begin(pinRemote, false, 0); // Start the receiver

    pinMode(pinFFBlank, OUTPUT);
    digitalWrite(pinFFBlank, true); // required to get fillament going, can be dimmed with PWM.
    // analogWrite(pinFFBlank, 128); // makes noise with 4500Hz
}

void loop()
{
    if (elapsedRedraw > 1000 / 20)
    {
        elapsedRedraw = 0;

        Serial.print(".");
        i2cscan();

        // just some output to check the port.
        Wire1.beginTransmission(0xa7);
        Wire1.write("subermajo");
        Wire1.endTransmission();

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
    }

    if (IrReceiver.decode())
    {
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
                snprintf(textbuffer, 100, "> Play/Pause");
            } // play/pause
            if (com == 2)
            {
                snprintf(textbuffer, 100, "MENU");
            } // menu
            if (com == 11)
            {
                snprintf(textbuffer, 100, "UP ^^^");
            } // up
            if (com == 13)
            {
                snprintf(textbuffer, 100, "DOWN :(");
            } // down
            if (com == 8)
            {
                snprintf(textbuffer, 100, "<<<");
            } // left
            if (com == 7)
            {
                snprintf(textbuffer, 100, ">>>");
            } // right
        }
        IrReceiver.resume(); // Enable receiving of the next value
    }
}

byte i2cScanAdr = 1;
void i2cscan()
{
    byte error;
    int nDevices;

    Serial.println("Scanning...");

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
}
