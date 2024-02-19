#include <mbGFX_MN12832L.h>
#include <elapsedMillis.h>
#include <mbLog.h>

int16_t textPos = 128;
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
            uint32_t time = micros();

// #define DRAWBEBUG 1
#ifdef DRAWBEBUG
            // writes 128 pix
            // v1 ~ 1975us
            // v2 ~ 1223us
            
            display.fillScreen(0);

            display.drawLine(0,0,31,31,0);
            display.drawLine(32,0,32+31,31,1);
            display.drawLine(64,0,64+31,31,2);
            display.drawLine(96,0,96+31,31,3);
            
            display.fillRect(10,10,10,10,0);
            display.fillRect(20,10,10,10,1);
            display.fillRect(30,10,10,10,2);
            display.fillRect(40,10,10,10,3);
            // display.drawRect(1,1,4,4,1);
            // display.drawRect(7,7,4,4,1);
            // LOG <<"-----------" <<LOG.endl;
#else
            /// draw random EQ
            // v2 ~ 31ms

            display.fillScreen(0);
            display.drawLine(0,10,127,10, 1);
            display.setCursor(textPos,0);
            // display.setTextSize(2,2);
            display.setTextWrap(false);
            display.setTextColor(1);
            display.print("suber");
            display.setTextColor(3);
            display.print("majo");
            display.setTextColor(2);
            display.print(" :-P");

            textPos -= 3;
            if(textPos < -100) textPos = 120;

            for(int16_t x = 0; x < 120; x+=9)
            {
                int16_t hi = random(16);
                display.fillRect(x,32 - hi, 7, hi, 2);
            }
#endif

            display.swapBuffers();

            drawtime = micros() - time;
        }
        if(logger > 1000)
        {
            // display.fillScreen(0);
            logger = 0;
            LOG <<LOG.dec <<"gate time: " <<display.getDisplayTime() <<"us, possible fps:" <<1000000/display.getDisplayTime()/44 <<" current fps:" <<1000000/display.getDisplayFps1()/44 <<" draw:" <<drawtime <<"us" <<LOG.endl;
        }
    }

    MN12832L display;
    elapsedMillis checker;
    elapsedMillis logger;
    uint32_t frameCounter;
    uint32_t drawtime = 0;
};
