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

    void saveloadWrite(uint16_t address, uint32_t value);
    void WireEndTransmission(boolean sendStop=true);

    MN12832Lgrey display;

    /// @brief DSP I2C address
    uint8_t _addr = 0;
    /// @brief If disabled, I2C comm stops, can connect sigmastudio then.
    boolean dspEnabled = true;
    uint16_t dspReadCycle = 0;

    elapsedMillis emChecker = 0;
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
    int16_t _volumeDB = -30;
    boolean _muteSPK = true;

    ClickEncoder enc1;
};
