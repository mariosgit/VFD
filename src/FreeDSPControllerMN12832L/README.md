# FreeDSP Controller with MN12832L

## Bugs

DSP does not boot itself !?.

## Todo

### Spectrum Analyser

Using the provided 7 Band or Designer Things uses a lot of DSP prog !

PlanB:

Index selectable filter before the EQ level det. Try to sycle through these to get Spectral readings ?!
```
Block Write
Cell Name:  EQFilterIDX
Param Name:  DCInpAlg1
Param Address:  0x000A
Param Value:  2
Bytes:  4
Param Data: 0x00, 	0x00, 	0x00, 	0x02
```

Knattert rum -> use saveload !?
Wackelt rum aber nix sinnvolles