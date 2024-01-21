# PCB's

### Reminder

To edit a schematic/pcb, open the .pro file first (double click) KiCad needs this to handle sch/pcb together, otherwise u get wiered errors when updating pcb from schematic.

## GP1294

![image](GP1294-Teens4.png)

A Panel with that display and an Teensy4.

Intended Use: Graphical EQ. The bottom PCB is a FreeDSP-AllInOne.
 * Infopage https://freedsp.github.io/
 * Github: https://github.com/freeDSP/freeDSP-ALLinONE 

Contains
* Power for VFD (~3V fillament, 30-50V for anode and grid) and 5V for other stuff.
* Teensy4 - Controls the Amp and shows vintage spectrum analyser.
* 2 Encoders
* 5 Buttons.

### Todo

* Power in/out detection circuit ! To control Amp muting.
* Teensy3 ? cheeper / better ?
* Test audio connection to/from teensy I2S slave ! Can it do USB playback ?
* Generate DSP C interface files. Test ! (It worked with the FreeDSP controller project)
* FreeDSP-AIO flaws
  * speaker mute before shutdown...
    * it hase a startup unmute circuit but no shutdown...
    * mute = ground pin 5 of 7414 would trigger a mute ? Does it work -> test!!!
    * [Tranistor mute](https://www.falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWEAmM0EE4DMGAcB2ZLMSANjABYxkR8l7IQEBTAWjDACgA3cCikFgQ1KA3IwkgKjLNCxQFMBJwDufMY2QJSIcVFUptumaVzH9AJ0GnzWG2FKSS8SAbtmhI-oOH6ADlIYOg6MFEE+NPISBlQ6ntYevq5WWjqpgsiaRozOrgDm6hGF0pKuAekVRlh2gooG6dVpVUn1zZE28a5qWJlF7kWuAC4ZmpAeNplmjOzg+LA4uBjYqMIYCAjyYAjQphQI1JCU4pAY+ALOIAAmzABmAIYArgA2g269kyOGOl0J4I6-YAwNB+sT+JjMIX0ajCwX+oMhIPC8VBnU4AGMpA4wZi4kkoLAXBA0EINgQsGRCLgEPgMPi4FwsPhwShkAJ+vEINEApgRP8eX06mp+cikS0wLgaDDsfyEfikESdoI5PpGVlgkCvprOfoAEpajUfQHA8DgHJmxToTgAeyYTGoCmkeFtMG1XBtqDtxsdZiQLpNbpAtP2XpOZl6fpEnCAA) 
   * Mute with threshhold via a [555 thing](https://www.falstad.com/circuit/circuitjs.html?ctz=CQAgjCAMB0l3BWEAmM0EE4DMGAcB2ZLMSANjABYxkQFTbbJaBTAWjDACgxSktIK4Ukyr0eIkBWgYZsjAnjwuAdxQUmrCrhBZS26tsicASrQqDWWGhSshLNJiKZJHUdJwDyk28mSCKGPS+ghBGqsh0KLjaEUGRYSiRsToCifQJycjRKYJZhpwAbmnFYBg0yUwQJI7QoVBuCCbgZSXCxZXglV31MI0ADsV5xVgINFg9nADmIKSCCNQzc1kT0+pMVk7ruhOquvrLNjQGUJyqh+AHtqUOnABOahpaD0L11fCnzUct-CEtRgDGzx4z1wrjQighkLAbHwbkgWHwuBG8nwgS0uHkkBcsEgXGmfiYoOegQmAx+L3JRPGjimORAVNSJJpqhJ1xArPSHzZwIJLwy6gpqWBRlMszsT1smkM9QFLh67lMe0kwPJoh6kiYyFcNUauz0OlGOn1GxOqm5LVZf04ABc7JQQvh6OwFmBHfV2OrqiAACbMABmAEMAK4AG2ttLZTzZTJOtvEOi+Ywc4DcAXIuh4+FdGJwsN6WFzqAilCwuGQLS9vsDofD9yubtsthE+Dgd3ZTqeHNeELbSpGY31aq6rawggAWpZpCm0FmbKNcNcF-BkDD6sgcZDIbhBFgp1wAF6fWgulrmGIgCe7jCcUcX1gIaBicDQWf9hdlJdwFesPMoDebxRt1oR9wE4Q8tW0M8UEgbRS1yO8H1INsxX7RYomlCgWwyJJsjFIYjAGDhrCjBZ5jGHZwAWKCxSghIiOPGgxTI000Kg4FaK5egoPsSQngSJiXXoZi6KEhYeOEzhAWBZj2L48A2G0VgYAMMACAQbctUdLNUhgOAuHucSxNsWjGBHQQ-WUb1bgAez6AAdABHDBpxfEt50XN5v1-XxoALeZzHwbTSGCwLXk4b00Lg1inhoCyrNsxzr1vOKbPshzxiqVy5yyDzl1XJh8GgcwFC0CIW18XxeDCiKaKeATyhAFKEvSm8W0iw0UKAup+UebQSSlFj+rqiwow+MVgSpG5bUbRid2TCBNBfUEsnIfB1LgCgEE2txUECWYBHnLFsAwEI4B9f1gzDNtzSONom06RRaRo7QxRjIx7jFAbxvSB73ltZ0aB41giR4jQFvXeESCRBBiFwOg2GIQG0Bkag9FdIhIHwBGXTOqtLvDaz2QTeoAA8lx5KRBF0+AMEdOgIjXBhNCOaVAfmThCfGe6yZIHkakUWneFIBnNRTH96ARV47BbTggA) 
   * Other theorie: Use LED row to get a voltage threshhold around 9V
  * Not much capacitance at Vin, more caps ! Could delay the shutdown ! More time to mute !
* Add 3d Knobs for encoder and buttons
* Classic flip switches instead of se buttons ?

# Refs

Used Kicad 3d models:
 * models https://gitlab.com/MountyRox/kicad-packages3D.git 
 * sources https://gitlab.com/kicad/libraries/kicad-packages3D-source.git
