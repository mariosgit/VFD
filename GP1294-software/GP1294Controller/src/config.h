#pragma once

#include <Arduino.h>
#include <Wire.h>

#include <TeensyThreads.h>
extern Threads::Grab<usb_serial_class> SerialXtra;
#define Serial ThreadClone(SerialXtra)

// pins...
const byte pinDBG0 = 0;
const byte pinDBG1 = 1;
const byte pinDBG2 = 2;

const byte pinSLat = 3;
const byte pinSCLK = 4;
const byte pinSIN = 5;

const byte pinCS = 10;
const byte pinReset = 9;
const byte pinFFBlank = 23; //=A9;
const byte pinAmbi = 22;    // ambient light, analog value
const byte pinRemote = 21;  // LIRC input pin, sensor=4838

const byte pinSDA = 17; // A3
const byte pinSCL = 16; // A2
