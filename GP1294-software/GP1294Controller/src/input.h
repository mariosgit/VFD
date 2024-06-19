#pragma once

class Input
{
public:
    void begin(int delayTime = 100);
    inline int id() {return threadId;}

private:
    void run();
    int delayTime = 100;
    int threadId = 0;

    volatile uint16_t input;
};
