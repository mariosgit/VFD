# Status

none. Trying again after some years... seen a video proof on hackaday projects but nothing more :(

<a href="https://www.buymeacoffee.com/mariosgeu" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/default-orange.png" alt="Buy Me A Coffee" height="41" width="174"></a>

# How does it work ???

### Theory 1 

* Can chain the 2 shift registers to get one with 480 bits, thats 60 bytes

* Bits 1-192 = 24bytes are pixel, 193-236 are gates, so the later just shift..
  * 3 bit per gate - that seems to be the 3 gray values - WRONG thats 3 pixels per gate, almost, first and last are shorter

* 
  * Ah mhm ok so it scans from left to right and you give it 2 coluns of pixels each time ?

* After shifting we pull LAT pin high, normally it stays low

* BLK (blank) should go high when data is transfered ? or just during the latch ?, this turns off the high voltage

* always have 2 neightboring gates on, shift them trough

* MYSTERY 1: Why 2 shift registers
  * Maybe just use them in parallel ???

* MYSTERY 2: GCP = "is the counter clock for the PWM decoder" this seems to turn high 5 times during shifting ??? Haeh ???
  * Mhm it toggles within one gate phase buhaahh then I need to set a time to do that ???
    * So do the latch/blank
    * Start timer to get these 5 impulses out ??? 
    * And why 6 pulses ? 
    * Add 6 pulses during shifting... no timer ?
  * New idea,
    * This just toggles through the 6 pixels.
    * Because there are 2 gates for 6 pixels, only one column can be on during one gate cycle.
    * if not cygled or with inconsistant timing this will should lead to different brightness per column !?
    * Then the 8 grey levels just come from the fastness, you can baller through 3 subframes per frame if needed.

* Mystery 3:
  * Ahh the bits are swapped around afbecd... so mem layout is???  bits: 162534, mhm have to write each pixel alone anyway.