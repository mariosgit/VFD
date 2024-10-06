# FreeDSP Controller with MN12832L

![panel](../../images/MN12932L-Teensy4.png)

## Usage

Top Left Encoder switches display pages. All other encoders change params on the page.

#### Main page

Center Encoder - Volume up/down
Bottom Right Encoder - Volume up/down, Click toggles mute between Headphone and Speaker.

Not much more implemented so far.

## Bugs

DSP does not boot itself !?. Press reset while panel has not started connecting - 4sec. Maybe put dsp-reset on the panel. Would be better for consistency when the prog is uploaded by the teensy.

XL6009 
* voltage divider values are wrong. Old 2.2k / 91k -> **New: 2k / 39k**
* unstable, add input caps 10-47uF + 100nF, a 100nF over R39k removed 25Hz flicker dropouts !???

HC132 Oszilator Values are wrong. Old 2.7k 4.7n -> **New: 2.7k / 100nF???**

Check Value of R3 **New: 4.7ohm**.

Add Caps 100n, 10u 
* close to Display 

Add mounting holes ? Spacer for DSP board?

Fillet at outlines

Add rev polarity safety diode

Add Fuse, ~100mA

Add backlight (LEDs behing the knobs ?)

Add IR receiver and Ambient Lt sensor.

## Todo

Control more stuff.

## Done

### Spectrum Analyser

Using the provided 7 Band or Designer Things uses a lot of DSP prog !

PlanB:

Index selectable filter before the EQ level det. Try to sycle through these to get Spectral readings ?!
```
Saveload Write
Cell Name:  EQFilterIDX
Param Value:  0-8
Bytes:  4
Param Data: 0x00, 	0x00, 	0x00, 	0x02
```
The first filter is flat, the others are 8 bands 100,200,400,800,1600,3200,6400,12800,Hz.

The emChecker loop sets these filters one after another and reads the levelmeter behind. This Results in a Spectrum analyser which seems to kind of work somewhat. Good enouth for the beginning.

## References

![DSPplan](../../images/Screenshot%202024-10-01%20153901.png)
