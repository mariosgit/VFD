#include <Arduino.h>

/// wrapping the different code examples into main...
/// choose one !
// #define RUN_MN12832L_GREY_TEST
#define RUN_MN12832L_MONO_TEST
// #define RUN_FREEDSPCONTROLLER
// #define RUN_PANELDEMO

/// do not touch anything below :)

#ifdef RUN_MN12832L_GREY_TEST
#include <examples/MN12832grey.ino>
#endif

#ifdef RUN_MN12832L_MONO_TEST
#include <examples/MN12832mono.ino>
#endif

#ifdef RUN_PANELDEMO
#include "PanelDemo/PanelDemo.h"
PanelDemo pd;
#define SETUP pd.setup();
#define LOOP pd.loop();
#endif

#ifdef RUN_FREEDSPCONTROLLER
#include "FreeDSPController/FreeDSPController.h"
FreeDSPController fc;
#define SETUP fc.setup();
#define LOOP fc.loop();
#endif

#if !defined(RUN_MN12832L_MONO_TEST) && !defined(RUN_MN12832L_GREY_TEST)
void setup() { SETUP }
void loop() { LOOP }
#endif