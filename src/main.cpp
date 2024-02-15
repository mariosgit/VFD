#include <Arduino.h>

// wrapping the different code examples into main...
// choose one !

// #define RUN_FREEDSPCONTROLLER
#define RUN_PANELDEMO

// do not touch anything below :)
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
