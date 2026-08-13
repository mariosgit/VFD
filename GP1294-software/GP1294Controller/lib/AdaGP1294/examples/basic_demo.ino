// Basic demo for the GP1294AI Adafruit_GFX driver.
//
// Wiring (adjust pin numbers for your board):
//   CS  -> GP1294AI_CS_PIN
//   SCK -> hardware SPI SCK
//   SI  -> hardware SPI MOSI
//   RST -> GP1294AI_RST_PIN (optional, or tie high and pass -1)
//
// If your module *does* break out a separate D/C line, this library
// will need a small update - see the note at the top of GP1294AI.h.

#include <SPI.h>
#include "GP1294AI.h"

#define GP1294AI_CS_PIN   10
#define GP1294AI_RST_PIN  9

GP1294AI display(GP1294AI_CS_PIN, GP1294AI_RST_PIN);

void setup() {
  if (!display.begin()) {
    while (1) {} // init failed
  }

  display.setDimming(600); // 0..1023

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(GP1294AI_WHITE);
  display.setCursor(0, 0);
  display.println("Hello, GP1294AI!");
  display.drawRect(0, 20, 100, 20, GP1294AI_WHITE);
  display.fillCircle(150, 30, 10, GP1294AI_WHITE);
  display.display();
}

void loop() {
}
