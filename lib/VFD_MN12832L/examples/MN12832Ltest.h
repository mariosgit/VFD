#include <mbGFX_MN12832L.h>
#include <elapsedMillis.h>
#include <mbLog.h>


class MN12832Ltest
{
    public:
    MN12832Ltest() :
        checker(0),
        logger(0),
        frameCounter(0)
    {

    }
    void setup()
    {
        display.begin();
        display.fillScreen(0);
        randomSeed(analogRead(0));
    }
    void loop()
    {
        if(checker > 100)
        {
            checker = 0;
            uint32_t time = millis();

            // display.drawLine(0,0,12,0,1);
            // display.drawRect(1,1,4,4,1);
            // display.drawRect(7,7,4,4,1);
            // LOG <<"-----------" <<LOG.endl;

            // display.fillScreen(0);
            display.drawLine(0,10,127,10, 1);
            display.setCursor(20,0);
            // display.setTextSize(2,2);
            display.print("subermajo");
            display.print(" :-P");

            for(int16_t x = 0; x < 120; x+=9)
            {
                int16_t hi = random(16);
                display.fillRect(x,16,7,31,0);
                display.fillRect(x,32 - hi, 7, hi, 1);
            }


            // display.swapBuffers();

            drawtime = millis() - time;
        }
        if(logger > 1000)
        {
            // display.fillScreen(0);
            logger = 0;
            LOG <<LOG.dec <<"gate time: " <<display.getDisplayTime() <<"us, possible fps:" <<1000000/display.getDisplayTime()/44 <<" current fps:" <<1000000/display.getDisplayFps1()/44 <<" draw:" <<drawtime <<"ms" <<LOG.endl;
        }
    }

    MN12832L display;
    elapsedMillis checker;
    elapsedMillis logger;
    uint32_t frameCounter;
    uint32_t drawtime = 0;
};
