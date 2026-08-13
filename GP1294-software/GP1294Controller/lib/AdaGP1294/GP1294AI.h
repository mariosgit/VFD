/*
  GP1294AI.h

  Adafruit_GFX driver for the Noritake/Rohm GP1294AI 256x48 VFD module,
  ported from the u8g2 driver u8x8_d_gp1294ai.c
  (https://github.com/olikraus/u8g2/).

  ---------------------------------------------------------------------
  IMPORTANT WIRING ASSUMPTION - please verify against your module/datasheet
  ---------------------------------------------------------------------
  The original u8g2 driver bit-reverses (SWAP8) *every* byte it sends -
  commands, command arguments, and pixel data alike - and never touches
  a D/C (data/command) GPIO. That's the signature of a simple 3-wire
  serial protocol (CS / SCK / MOSI only), which is typical for VFD
  driver ICs of this family, rather than the 4-wire SPI + D/C pin
  scheme used by most OLED controllers (SSD1306 etc.).

  This class therefore only requires CS, SCK, MOSI (+ optional RESET).
  If your particular module breaks out a D/C pin, let me know and I'll
  adapt the transfer functions accordingly.
  ---------------------------------------------------------------------

  Framebuffer layout: 6 pages of 8 vertical pixels each (256 bytes per
  page, bit 0 = topmost pixel of the 8-pixel column) - the same layout
  u8g2 uses internally for this display, and the same convention as
  SSD1306-style buffers.

  License: same 3-clause BSD terms as the original u8g2 source this was
  derived from.
*/

#ifndef GP1294AI_H
#define GP1294AI_H

#include <Adafruit_GFX.h>
#include <SPI.h>

// ---- Physical display geometry ----
#define GP1294AI_WIDTH   256
#define GP1294AI_HEIGHT  48
#define GP1294AI_PAGES   (GP1294AI_HEIGHT / 8)   // 6 pages of 8px each

// Convenience monochrome color values for use with Adafruit_GFX calls
#define GP1294AI_BLACK   0
#define GP1294AI_WHITE   1

class GP1294AI : public Adafruit_GFX {
public:
  // cs_pin  : chip-select GPIO (required)
  // rst_pin : hardware reset GPIO (-1 if not connected; the panel is
  //           also soft-reset over SPI during begin())
  // spi     : which SPI bus to use (defaults to the main hardware SPI)
  GP1294AI(int8_t cs_pin, int8_t rst_pin = -1, SPIClass *spi = &SPI);

  // Initializes pins/SPI, resets and configures the panel, clears the
  // display and wakes it up. Returns true on success.
  bool begin(uint32_t spi_hz = 4000000UL);

  // Pushes the entire internal framebuffer to the panel. Nothing you
  // draw with Adafruit_GFX calls (drawPixel, print, drawRect, ...) is
  // visible until you call this.
  void display();

  // Clears the internal framebuffer (does NOT push to the panel -
  // call display() afterwards to actually blank the screen).
  void clearDisplay();

  // Required Adafruit_GFX override - every GFX drawing primitive
  // (lines, text, shapes, bitmaps...) ultimately calls this.
  void drawPixel(int16_t x, int16_t y, uint16_t color) override;

  // Global brightness / dimming level, 0..1023 (chip default ~0x28*4).
  void setDimming(uint16_t level);

  // Standby (true) / wake the panel back up (false). The framebuffer
  // is preserved in standby; call display() again after waking if you
  // want to be sure the panel content is current.
  void sleep(bool enable);

private:
  int8_t      _cs, _rst;
  SPIClass   *_spi;
  SPISettings _spiSettings;
  uint8_t     _buffer[GP1294AI_WIDTH * GP1294AI_PAGES]; // 1536 bytes

  void startTransfer();
  void endTransfer();
  void writeByte(uint8_t b);                 // bit-reverses, then SPI-transfers
  void sendCommand(uint8_t cmd);
  void sendCommand(uint8_t cmd, const uint8_t *args, uint8_t len);
  void sendData(const uint8_t *data, size_t len);
};

#endif // GP1294AI_H
