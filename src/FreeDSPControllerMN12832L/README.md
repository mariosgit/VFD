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

#### The strangly vintage universe of SigmaDSP

Dear analog devices, you web resources did'nt age so well.

Documentation of the DSP blocks... best to search with Ggl
https://wiki.analog.com/resources/tools-software/sigmastudio/toolbox/leveldetectorslookuptables/singlebandleveldetector

Reading Levels and other tutorials ... https://ez.analog.com/dsp/sigmadsp/w/documents/14313/monitoring-average-and-instantaneous-signal-levels


### SigmaStudio logs

#### SoftClip

Link [SigmaWiki SoftClip:](https://wiki.analog.com/resources/tools-software/sigmastudio/toolbox/nonlinearprocessors/standardcubic)


```
Block Write
Time:  17:31:17 - 173ms
IC:  IC 1
Cell Name:  SoftClip1
Param Name:  SoftClipAlgG21alpha
Param Address:  0x0062
Param Value:  10
Bytes:  4
Param Data:
0x05, 	0x00, 	0x00, 	0x00
```


Change to "9" which is off. Range is 0.1 - 10. You always write the value and it's reciproke, so 1/9. SigStud does a block write here. Values 3,4 are static.

```
Cell Name:  SoftClip1
Param Name:  SoftClipAlgG21alpha
Param Address:  0x0062
Param Value:  9
Bytes:  4
Param Data: 0x04, 	0x80, 	0x00, 	0x00

Cell Name:  SoftClip1
Param Name:  SoftClipAlgG21alpham1
Param Address:  0x0063
Param Value:  0.111111164093018
Bytes:  4
Param Data:
0x00, 	0x0E, 	0x38, 	0xE4

// constant.. ?

Cell Name:  SoftClip1
Param Name:  SoftClipAlgG21onethird
Param Address:  0x0064
Param Value:  0.333333373069763
Bytes:  4
Param Data: 0x00, 	0x2A, 	0xAA, 	0xAB

Cell Name:  SoftClip1
Param Name:  SoftClipAlgG21twothird
Param Address:  0x0065
Param Value:  0.666666626930237
Bytes:  4
Param Data: 0x00, 	0x55, 	0x55, 	0x55
```

turn to bypass
```
Cell Name:  Nx2-1
Param Name:  StMuxSwSlew1coeffname
Param Address:  0x0097
Param Value:  0
Bytes:  4
Param Data: 0x00, 	0x00, 	0x00, 	0x00
```

turn switch on
```
Cell Name:  Nx2-1
Param Name:  StMuxSwSlew1coeffname
Param Address:  0x0097
Param Value:  1
Bytes:  4
Param Data: 0x00, 	0x00, 	0x00, 	0x01
```
