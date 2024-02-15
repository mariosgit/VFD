# VFD

A collection of upcycling projects for Vakuum Fluorescent Displays (VFD). These are Display as you may find in Audio/Video gear like CD/DVD players. A VFD requires high voltage to control anodes and gates, and a small AC voltage for the fillament(heater). To drive each single display element, a time multiplexed controller is required.

## New structure...

I changed this into a PlatformIO project and moved out the display libraries. This should make it easier to integrate a lib into your project. Just add it as a git submodule like i did here.

You can also add a ref to the repo in the platformio.ini lib_deps (if you do not plane to change the lib code)
```
lib_deps =
    ; external Git resource
    https://github.com/mariosgit/VFD_MN12832JC.git
```

VFD_TPIC.. stuff was not integrated into this platformio project yet..

### src/main

You can switch between demo programms by commenting in/out the defines.. only one at a time...

```
// #define RUN_FREEDSPCONTROLLER
#define RUN_PANELDEMO
```

### VFD_TPIC_Test

Arduino programm to test TPIC6b595 based circuits.
https://www.instructables.com/id/Audio-Level-Meter-From-an-Upcycled-VFD/
The TPIC.. is a shift register with a gate or anode attached to each output. You can attach the display gate/anode pins in any order and later figure out the bit's to use. You will cygle through the anodes and turn on the corosponding gates for each digit...

Works, but.. the open drain of these drivers is suboptimal because of high voltage and many pullup resistors a significant amount of current is running and it's getting hot and inefficient.

## PCBs

PCB-Kicad contains one for the MN12832JC + all the power supply.

### Status

See README in subdirectories.

### Comments

PCBs...

They all use a similar net naming
- +5V
- VPP for the 50V anode voltage
- F1 and F2 for the fillament AC voltage

