#pragma once

#include <Arduino.h>

#include <SigmaDSP.h>

class DSPCtrl {
public:
    DSPCtrl();

    void init();

    void checkProgramm();

    void readLevels();
    void setVolume(int16_t volumeDB);

    void eqBandNext() { eqBand = (eqBand + 1) % 9; };
    void eqBandPrev() { eqBand = (eqBand - 1) % 9; if(eqBand < 0) eqBand += 9; };
    void eqVal(int diff);

    void setLowShelf(float *coef, float frequency, float gain, float slope);
    void setHighShelf(float *coef, float frequency, float gain, float slope);

    inline float toLog(float val) { return 20 * log10f(val); }

    /// @brief If disabled, I2C comm stops, can connect sigmastudio then.
    boolean dspEnabled = false;
    uint16_t dspReadCycle = 0;

    int8_t eqBand = 0;   // 8 bands
    int8_t eqValues[9] = {0,0,0,0, 0,0,0,0 ,0};  // gain value for each band, +-16
    const float eqQues[9] = {1.2, 1.2, 1.2, 1.2,    1.2, 1.2, 1.2, 1.0,     0.77};
    const float eqFreq[9] = {50., 100., 200., 400., 800., 1600, 3200, 6400, 12800};    // between analyser freqs 100,200, 400,800, 1600,3200, 6400,12800

    struct Levels
    {
        float inL;
        float inR;
        float postall;
        float distortion;
        float peakPostUserEQ;
        float peakOutputLoX2;
        float peakAnalogIn;
        float avgDistortion;
        float postEQ[10];
    } levels = {-99,-99,-99,-99,-99,-99, -99, -99, {-99,-99,-99,-99,-99,-99,-99,-99,-99,-99}};

    const float AUDIO_SAMPLE_RATE_EXACT = 48000.0f;
    SigmaDSP dsp;
};
