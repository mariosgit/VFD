#include <Arduino.h>
#include <TeensyThreads.h>

// this method is experimental
ThreadWrap(Serial, SerialXtra);
#define Serial ThreadClone(SerialXtra)
