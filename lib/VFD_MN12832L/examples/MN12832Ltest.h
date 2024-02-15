#include <mbGFX_MN12832L.h>

class MN12832Ltest
{
    public:
    MN12832Ltest()
    {

    }
    void setup()
    {
        display.begin();
    }
    void loop()
    {
        display.fillScreen(1);
        display.swapBuffers();
    }

    MN12832L display;
};
