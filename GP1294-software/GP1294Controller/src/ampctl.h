#pragma once

#include <stdint.h>

class AmpControl
{
public:
    void begin(int delayTime = 100);
    inline int id() {return threadId;}

private:
    void run();

    int delayTime = 100;

    int threadId = 0;
};
