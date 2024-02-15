#include <Arduino.h>

/// wrapping the different code examples into main...
/// choose one !
#define RUN_MN12832L_TEST
// #define RUN_FREEDSPCONTROLLER
// #define RUN_PANELDEMO

/// do not touch anything below :)

#ifdef RUN_MN12832L_TEST
#include <examples/MN12832Ltest.h>
MN12832Ltest test;
#define SETUP test.setup();
#define LOOP test.loop();
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

void setup() { SETUP }
void loop() { LOOP }
