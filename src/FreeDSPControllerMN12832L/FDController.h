#pragma once

#include <VFD_MN12832L.h>
#include <elapsedMillis.h>
#include <mbLog.h>

#include <IntervalTimerEx.h>
#include <ClickEncoder.h>

class FDController
{
public:
    FDController();

    void setup();
    void loop();

    void inputService();
    void saveloadWrite(uint16_t address, uint32_t value);
    void WireEndTransmission(boolean sendStop=true);

    MN12832Lgrey display;

    /// @brief DSP I2C address
    uint8_t _addr = 0;
    /// @brief If disabled, I2C comm stops, can connect sigmastudio then.
    boolean dspEnabled = false;
    uint16_t dspReadCycle = 0;

    elapsedMillis emChecker = 0;
    elapsedMillis emInput = 0;
    elapsedMillis emDraw = 0;
    elapsedMillis emLogger = 0;

    uint32_t frameCounter = 0;
    uint32_t drawtime = 0;
    int16_t textPos = 128;

    // Create an IntervalTimer object
    IntervalTimerEx myTimer;

    struct Levels
    {
        float inL;
        float inR;
        float distortion;
        float postEQ[10];
    } _levels;

    int16_t _volumeDB = -40;
    uint8_t _mute = 1; // DSP starts with speaker muted
    const uint8_t MUTE_SPK_MASK = 1;
    const uint8_t MUTE_HP_MASK = 2;

    uint32_t input;
    volatile uint32_t inqueue[8];
    volatile uint32_t inqupos = 0;
    uint32_t inacclast = 0;

    //   1     4
    //      3 
    //   2     5
    ClickEncoder enc1;
    ClickEncoder enc2;
    ClickEncoder enc3;
    ClickEncoder enc4;
    ClickEncoder enc5;
};
