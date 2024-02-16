#include <mbGFX_MN12832L.h>
#include <elapsedMillis.h>
#include <mbLog.h>


class MN12832Ltest
{
    public:
    MN12832Ltest() :
        checker(0),
        frameCounter(0)
    {

    }
    void setup()
    {
        display.begin();
    }
    void loop()
    {
        if(checker > 1000)
        {
            checker = 0;

            // display.fillScreen(0);
            display.drawLine(6,0,18,12, 1);
            display.setCursor(30,5);
            display.setTextSize(2,2);
            display.print("MARIO");
            // display.swapBuffers();

            LOG <<"disp time: " <<display.getDisplayTime() <<"us, possible fps:" <<1000000/display.getDisplayTime() <<" current fps:" <<1000000/display.getDisplayFps1() <<LOG.endl;
        }
    }

    MN12832L display;
    elapsedMillis checker = 0;
    uint32_t frameCounter = 0;
};
