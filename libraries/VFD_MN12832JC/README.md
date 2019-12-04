# AdafruitGFX lib compatible driver for an MN12832JC VFD display.

![VFD in action](/images/mn12832jc.gif)

Works with TeensyLC, could be possible on Arduinos too, displayRefresh is called via IntervalTimer, which is Teensy only.

### Dependencies:
- mbLog, stdout style serial log, can be found in ../libraries

### Pinout, Connections
- These are hardcoded, see mbGFX_MN12832JC.cpp
