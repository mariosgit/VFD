// Code fot ATTiny84, handles 2 encoders + SPI buttons.

#include <TM1637.h>

#include <ATTinyCore.h>
#include <SPI.h>

#define __AVR_ATtiny84__
#include "TinyWireS.h"

#include "ClickEncoder.h"

const byte SLAVE_ADDR = 100;
const byte NUM_BYTES = 4;
volatile int8_t data[NUM_BYTES] = {0, 1, 2, 3};

ClickEncoder encoder(0, 1, 2, 4); // alps needs the 4 step stuff
TM1637 tm1637(10, 9);             // clk/dio

const byte pinLatch = 3;
const byte pinSCLK = 10;
const byte pinMISO = 9;

void setup()
{
    TinyWireS.begin(SLAVE_ADDR);
    TinyWireS.onRequest(OnTinyWireSRequest);

    tm1637.init();
    tm1637.set(BRIGHT_DARKEST); // BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

    //  encoder.setDoubleClickEnabled(false); // dann gibts auch kein click !
    // SPI.begin();
    // SPI.setDataMode(SPI_MODE2);

    pinMode(pinLatch, OUTPUT);
    pinMode(pinSCLK, OUTPUT);
    pinMode(pinMISO, INPUT);
}

int counter = 0;
void loop()
{
    encoder.service();

    data[0] += encoder.getValue();
    ClickEncoder::Button b = encoder.getButton();
    if (b == ClickEncoder::Clicked)
        data[1] += 1;
    if (b == ClickEncoder::Released) // kommt nur nach Held
        data[1] += 0;
    if (b == ClickEncoder::Held)
        data[1] = 0;

    counter++;
    if (counter > 1000)
    {
        counter = 0;
        // der kram dauert ewig
        tm1637.display(0, data[0] & 0x0f);
        tm1637.display(1, data[1] & 0x0f);
        tm1637.display(2, data[2] & 0x0f);
        tm1637.display(3, data[3] & 0x0f);
    }

    digitalWrite(pinLatch, LOW);
    digitalWrite(pinLatch, HIGH);
    // data[2] = SPI.transfer(0xaa);
    // data[3] = SPI.transfer(0x33);
    digitalWrite(pinSCLK, LOW);
    digitalWrite(pinSCLK, HIGH);
    digitalWrite(pinSCLK, LOW);
    digitalWrite(pinSCLK, HIGH);
    digitalWrite(pinSCLK, LOW);
    digitalWrite(pinSCLK, HIGH);
    digitalWrite(pinSCLK, LOW);
    digitalWrite(pinSCLK, HIGH);
}

void timerIsr()
{
    encoder.service();
}

void OnTinyWireSRequest()
{
    for (byte i = 0; i < NUM_BYTES; i++)
    {
        TinyWireS.write(data[i]);
        data[i] += 1;
    }
    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
    data[3] = 0;
}
