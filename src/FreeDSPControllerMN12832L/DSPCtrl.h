#pragma once

#include <Arduino.h>

#include <SigmaDSP.h>

class DSPCtrl {
  public:
    DSPCtrl();

    void readLevels();
    void setVolume(int16_t volumeDB);

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

    SigmaDSP dsp;
};
