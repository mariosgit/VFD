#pragma once

#include <VFD_MN12832L.h>
#include <elapsedMillis.h>
#include <mbLog.h>

#include <IntervalTimerEx.h>
#include <ClickEncoder.h>

#include "DSPCtrl.h"

class FDController
{
public:
    FDController();

    void setup();
    void loop();

    void inputService();

    DSPCtrl dspctrl;
    MN12832Lgrey display;

    elapsedMillis emChecker = 0;
    elapsedMillis emInput = 0;
    elapsedMillis emSerial = 0;
    elapsedMillis emDraw = 0;
    elapsedMillis emLogger = 0;

    uint32_t inputSlot = 0;
    uint32_t dspOffCounter = 0;
    uint32_t frameCounter = 0;
    uint32_t drawtime = 0;
    int16_t textPos = 128;

    // Create an IntervalTimer object
    IntervalTimerEx refreshTimer;
    IntervalTimerEx inputTimer;
    bool inputStuffEnabled = true; // it's a interrupt disable kind off

    int16_t _volumeDB = -40;
    float _distortion = 10.0; // 10 = none
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

private:
    void taskChecker();
    void taskInput();
    void taskSerial();
    void taskDisplay();
    void taskLogger();
};
