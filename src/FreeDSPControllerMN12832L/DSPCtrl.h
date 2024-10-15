#pragma once

#include <Arduino.h>

#include <SigmaDSP.h>

class DSPCtrl {
  public:
    DSPCtrl();

    void readLevels();
    void setVolume(int16_t volumeDB);

    void eqBandNext() { eqBand = (eqBand + 1) % 4; };
    void eqBandPrev() { eqBand = (eqBand - 1) % 4; if(eqBand < 0) eqBand += 4; };
    void eqVal(int diff);

    /// @brief If disabled, I2C comm stops, can connect sigmastudio then.
    boolean dspEnabled = false;
    uint16_t dspReadCycle = 0;

    int8_t eqBand = 0;   // 4 bands
    int8_t eqValues[4] = {0,0,0,0};  // gain value for each band, +-16
    const float eqFreq[4] = {300., 600., 2400., 9600.};    // between analyser freqs 100,200, 400,800, 1600,3200, 6400,12800

    struct Levels
    {
        float inL;
        float inR;
        float distortion;
        float postEQ[10];
    } levels = {-99,-99,-99,{-99,-99,-99,-99,-99,-99,-99,-99,-99,-99}};

    SigmaDSP dsp;
};
