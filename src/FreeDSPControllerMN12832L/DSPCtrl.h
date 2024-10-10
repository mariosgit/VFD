#pragma once

#include <Arduino.h>

class DSPCtrl {
  public:
    DSPCtrl();

    void readLevels();
    void setVolume(int16_t volumeDB);
    void saveloadWrite(uint16_t address, uint32_t value);
    void saveloadWrite(uint16_t address1, uint32_t value1,
                       uint16_t address2, uint32_t value2);

    inline uint32_t floatTo523(float val) { return val * (float)0x0800000; }
    
    /// @brief DSP I2C address
    uint8_t addr = 0;

    /// @brief If disabled, I2C comm stops, can connect sigmastudio then.
    boolean dspEnabled = false;
    uint16_t dspReadCycle = 0;

    struct Levels
    {
        float inL;
        float inR;
        float distortion;
        float postEQ[10];
    } levels = {-99,-99,-99,{-99,-99,-99,-99,-99,-99,-99,-99,-99,-99}};

  private:
    void WireEndTransmission(boolean sendStop=true);

};
