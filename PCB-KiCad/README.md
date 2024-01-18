# PCB's

### Reminder

To edit a schematic/pcb, open the .pro file first (double click) KiCad needs this to handle sch/pcb together, otherwise u get wiered errors when updating pcb from schematic.

## GP1294

A Panel with that display and an Teensy4.

Intended Use: Graphical EQ.

Contains
* Power for VFD and 5V for other stuff.
* Teensy4
* 2 Encoders
* 6 Buttons.

### Todo

* Power in/out detection circuit ! To control Amp muting.
* pin through for freedsp-llinone ?
* Teensy3 ?
* Teensy use SMD pins !? No throughole stuff under the disp
* move Disp down a bit -> use drawing / rendering ?
* FreeDSPAIO - speaker mute bei shutdown, mute in den pin 1 des 7414 rein, also da den elko short'n, dann geht der aus ?
  * Richtig sanft is das dann auch nich...
  * Oder pin 5 auf aehh 0 dann geht spk_sleep auf 1 und das is ein mute !? Ja irgendwie kackt der chip manchmal ab beim betaetigen !?
  * also am besten beides gegen 0 shorten, oder die caps mit ner externen spannung aufladen, also z diode.. aber ja aber nein dann dauerts ja auch bis die aus gehen -> see amp-off-mute-circuts...
  * mehr caps auf vpp/gnd am amp chip !

# Refs

Some Kicad 3d models...
https://gitlab.com/MountyRox/kicad-packages3D.git 
