/*
  GP1294AI.cpp
  See GP1294AI.h for wiring notes and license.
*/

#include "GP1294AI.h"

GP1294AI::GP1294AI(int8_t cs_pin, int8_t rst_pin, SPIClass *spi)
  : Adafruit_GFX(GP1294AI_WIDTH, GP1294AI_HEIGHT),
    _cs(cs_pin), _rst(rst_pin), _spi(spi) {
}

void GP1294AI::startTransfer() {
  _spi->beginTransaction(_spiSettings);
  digitalWrite(_cs, LOW);
  delayMicroseconds(1); // tCS-CLK (240ns) guard
}

void GP1294AI::endTransfer() {
  delayMicroseconds(1); // tCLK-CS (120ns) guard
  digitalWrite(_cs, HIGH);
  _spi->endTransaction();
}

void GP1294AI::writeByte(uint8_t b) {
  _spi->transfer(b);
}

void GP1294AI::sendCommand(uint8_t cmd) {
  startTransfer();
  writeByte(cmd);
  endTransfer();
}

void GP1294AI::sendCommand(uint8_t cmd, const uint8_t *args, uint8_t len) {
  startTransfer();
  writeByte(cmd);
  for (uint8_t i = 0; i < len; i++) writeByte(args[i]);
  endTransfer();
}

void GP1294AI::sendData(const uint8_t *data, size_t len) {
  // Caller wraps this in its own startTransfer()/endTransfer() when
  // it needs to keep CS low across a command + its data (see display()).
  for (size_t i = 0; i < len; i++) writeByte(data[i]);
}

bool GP1294AI::begin(uint32_t spi_hz) {
  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);

  if (_rst >= 0) {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(1);
    digitalWrite(_rst, LOW);
    delay(1);
    digitalWrite(_rst, HIGH);
    delay(1);
  }

  _spiSettings = SPISettings(spi_hz, LSBFIRST, SPI_MODE3);
  _spi->begin();

  // ---- Software reset ----
  sendCommand(0xAA);
  delay(1); // Trth - wait for reset

  // ---- VFD Mode Setting ----
  // Original: U8X8_CAAA(0xCC, 0x01, 0x1F, 0x00) then U8X8_A4(0xFF, 0x2F, 0x00, 0x20)
  // sent as one continuous transfer (cmd + 7 args).
  {
    startTransfer();
    writeByte(0xCC);
    writeByte(0x01);
    writeByte(0x1F);
    writeByte(0x00);
    writeByte(0xFF);
    writeByte(0x2F);
    writeByte(0x00);
    writeByte(0x20);
    endTransfer();
  }

  // ---- Dimming level setting (1024 levels, 0x3FF max) ----
  {
    uint8_t args[2] = {0x28, 0x00};
    sendCommand(0xA0, args, 2);
  }

  // ---- DW1 position setting ----
  {
    uint8_t args[2] = {0x00, 0x00};
    sendCommand(0xC0, args, 2);
  }

  // ---- Display Mode Setting (Scan STOP) ----
  {
    uint8_t arg = 0x10;
    sendCommand(0x80, &arg, 1);
  }

  // ---- T1 INT Setting ----
  {
    uint8_t arg = 0x00;
    sendCommand(0x08, &arg, 1);
  }

  clearDisplay();
  display();

  // The chip needs the memory cleared manually (no dedicated "clear"
  // command), matching the original driver's u8x8_ClearDisplay() step.
  // Note: unlike the original driver (which leaves the panel in
  // standby after init), this begin() wakes the panel so it's usable
  // right away. Call sleep(true) yourself if you'd rather it start
  // powered down.
  sleep(false);

  return true;
}

void GP1294AI::clearDisplay() {
  memset(_buffer, 0, sizeof(_buffer));
}

void GP1294AI::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if ((x < 0) || (x >= width()) || (y < 0) || (y >= height())) return;

  // Account for Adafruit_GFX rotation, same pattern used by
  // Adafruit_SSD1306 and similar page-buffer displays.
  switch (getRotation()) {
    case 1:
      // _swap_int16_t(x, y);
      x = WIDTH - x - 1;
      break;
    case 2:
      x = WIDTH - x - 1;
      y = HEIGHT - y - 1;
      break;
    case 3:
      // _swap_int16_t(x, y);
      y = HEIGHT - y - 1;
      break;
  }

  uint8_t *b = &_buffer[x + (y / 8) * GP1294AI_WIDTH];
  uint8_t bit = y & 7;

  if (color) {
    *b |= (1 << bit);
  } else {
    *b &= ~(1 << bit);
  }
}

void GP1294AI::display() {
  for (uint8_t page = 0; page < GP1294AI_PAGES; page++) {
    uint8_t y = page * 8 + 4; // matches the +4 row offset in the original driver

    startTransfer();
    writeByte(0xF0);       // set write position
    writeByte(0x00);       // x = 0
    writeByte(y);          // y
    writeByte(0x07);       // return every 8 pixels
    sendData(&_buffer[page * GP1294AI_WIDTH], GP1294AI_WIDTH);
    endTransfer();
  }
}

void GP1294AI::setDimming(uint16_t level) {
  if (level > 1023) level = 1023;
  uint8_t args[2] = { (uint8_t)(level & 0xFF), (uint8_t)(level >> 8) };
  sendCommand(0xA0, args, 2);
}

void GP1294AI::sleep(bool enable) {
  if (enable) {
    sendCommand(0x61); // Standby
  } else {
    sendCommand(0x6D); // Wake up
    delay(1);           // wait for OSC to stabilize
    uint8_t arg = 0x00;
    sendCommand(0x80, &arg, 1); // SC bit auto-clears on wake; re-affirm mode
  }
}
