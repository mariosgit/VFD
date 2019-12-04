# AdafruitGFX lib compatible driver for an MN12832JC VFD display.

![VFD in action](/images/mn12832jc.gif)

Works with TeensyLC, could be possible on Arduinos too, displayRefresh is called via IntervalTimer, which is Teensy only.

Just mind the signal level gap, use a level shifter or bring down VDD1 to ~4.3V (diode), high is specified as 80% of VDD1. When turning down VDD1 the display will transition from eratic to clean.

### Dependencies:
- mbLog, stdout style serial log, can be found in ../libraries

### Pinout, Connections
- These are hardcoded, see mbGFX_MN12832JC.cpp

## Todo:

### DMA

DMASPI: https://github.com/crteensy/DmaSpi

adds some defines.. https://github.com/Blackaddr/DmaSpi

