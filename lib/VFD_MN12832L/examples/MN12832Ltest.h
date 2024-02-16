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
        randomSeed(analogRead(0));
    }
    void loop()
    {
        if(checker > 1000)
        {
            checker = 0;


            uint32_t time = millis();
            display.fillScreen(0);
            display.drawLine(0,0,127,31, 1);
            display.drawRect(10,20,10,10,1);
            display.setCursor(30,1);
            // display.setTextSize(2,2);
            display.print("mb was here");

            // for(int16_t x = 0; x < 120; x+=16)
            // {
            //     int16_t hi = random(24);
            //     display.fillRect(x,32 - hi, 12, hi, 1);
            // }

            uint32_t drawtime = millis() - time;

            display.swapBuffers();

            // gps = gates per second, fps ~ gps/44
            LOG <<LOG.dec <<"disp time: " <<display.getDisplayTime() <<"us, possible gps:" <<1000000/display.getDisplayTime() <<" current gps:" <<1000000/display.getDisplayFps1() <<" draw:" <<drawtime <<"ms" <<LOG.endl;
        }
    }

    MN12832L display;
    elapsedMillis checker = 0;
    uint32_t frameCounter = 0;
};
