### VFD_TPIC_Test

Arduino programm to test TPIC6b595 based circuits.
https://www.instructables.com/id/Audio-Level-Meter-From-an-Upcycled-VFD/
The TPIC.. is a shift register with a gate or anode attached to each output. You can attach the display gate/anode pins in any order and later figure out the bit's to use. You will cygle through the anodes and turn on the corosponding gates for each digit...

Works, but.. the open drain of these drivers is suboptimal because of high voltage and many pullup resistors a significant amount of current is running and it's getting hot and inefficient.

