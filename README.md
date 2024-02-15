# VFD

A collection of upcycling projects for Vakuum Fluorescent Displays (VFD). These are Display as you may find in Audio/Video gear like CD/DVD players. A VFD requires high voltage to control anodes and gates, and a small AC voltage for the fillament(heater). To drive each single display element, a time multiplexed controller is required.

## New structure...

I will move things a bit in order to make (my) live easier :) I might keep the old version in a tag "unstructured-version".

This will become a PlatformIO project with a bit of example code.
* Libraries will move into seperate repos which can be linked as submodules.
* PCBs ? stay or move out.. will see.

### VFD_TPIC_Test

Arduino programm to test TPIC6b595 based circuits.
https://www.instructables.com/id/Audio-Level-Meter-From-an-Upcycled-VFD/
The TPIC.. is a shift register with a gate or anode attached to each output. You can attach the display gate/anode pins in any order and later figure out the bit's to use. You will cygle through the anodes and turn on the corosponding gates for each digit...

Works, but.. the open drain of these drivers is suboptimal because of high voltage and many pullup resistors a significant amount of current is running and it's getting hot and inefficient.

### VFD_MN12832JC

TeensyLC program to drive the MN12832JC graphical display.

![VFD in action](/images/mn12832jc.gif)

Works! Just mind the signal level gap, use a level shifter or bring down VDD1 to ~4.3V (diode), high is specified as 80% of VDD1. When turning down VDD1 the display will transition from eratic to clean.

## PCBs

PCB-Eagle contains one for the MN12832JC + all the power supply.

### Status

See README in subdirectories.

### Comments

At the moment I use Eagle (also playing with conversion to KiCAD 5..)

The folder PCB-Eagle contains my library with
- The displays
- The chips used (XL6009, ZXBM5210)

The folder PCB-Eagle-Blocks contains some design blocks which can be dropped in a PCB project.
- For the power converter

They all use a similar net naming
- +5V
- VPP for the 50V anode voltage
- F1 and F2 for the fillament AC voltage

The ZIP file is a set of production files.


# Refs

### Used Libs and Stuff

