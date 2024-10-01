#include <Arduino.h>

/// wrapping the different code examples into main...

// !!! selection is in .env file, copy below to .env,
// lines can be commented out with # as first char.
// All lines will be added as build flags.
/*
# RUN_MN12864K_TEST
# RUN_MN12832L_GREY_TEST
RUN_MN12832L_MONO_TEST
# RUN_FREEDSPCONTROLLER
# RUN_PANELDEMO
NAME=hanswurst
*/

// The example ino's all have setup and loop function, select just one.
// The one is pulled in here and used as main.

/// do not touch anything below :)

#ifdef RUN_MN12864K_TEST
#include <examples/VFD_MN12864K.ino>
#endif

#ifdef RUN_MN12832L_GREY_TEST
#include <examples/MN12832grey.ino>
#endif

#ifdef RUN_MN12832L_BENCHI
#include <examples/MN12832Lbenchi.ino>
#endif

#ifdef RUN_MN12832L_MONO_TEST
#include <examples/MN12832mono.ino>
#endif

#ifdef RUN_MN12832L_MONOFONT_TEST
#include <examples/MN12832monofont.ino>
#endif

#ifdef RUN_PANELDEMO
#include "PanelDemo/PanelDemo.h"
PanelDemo main;
void setup() { main.setup() }
void loop() { main.loop() }
#endif

#ifdef RUN_FREEDSPCONTROLLER
#include "FreeDSPController/FreeDSPController.h"
FreeDSPController ding;
void setup() { ding.setup(); }
void loop() { ding.loop(); }
#endif

#ifdef RUN_FREEDSPCONTROLLER_MN12832L
#include "FreeDSPControllerMN12832L/FDController.h"
FDController dings;
void setup() { dings.setup(); }
void loop()  { dings.loop(); }
#endif
