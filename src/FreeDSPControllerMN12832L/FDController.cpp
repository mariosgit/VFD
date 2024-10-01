#include "FDController.h"

#define SIGMASTUDIOTYPE_SPECIAL static_cast<int16_t> // ??? just an hex uint16_t, for the 12bit ProgCounter where the readback happens ?
#include "AMPx4-TDM-Test02/SigmaKram_IC_1_PARAM.h"
#include "AMPx4-TDM-Test02/SigmaKram_IC_1_REG.h"

FDController::FDController() : display( 
                                /* pinBLK = */ 4,
                                /* pinLAT = */ 6,
                                /* pinGCP = */ 3,
                                /* MOSI_PIN = */ 11,
                                /* SCK_PIN = */ 13,
                                /* pinPWM = */ 20),
                                enc1(7,8,5,4,false)
{
    // DSP default adr
    _addr = 0x68 >> 1;
    pinMode(7, INPUT_PULLUP);
    pinMode(8, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);
}

void FDController::setup()
{
    display.begin();
    randomSeed(analogRead(0));

    Wire.begin();

    // You need to trigger the refresh function regularly !
    // myTimer.begin(display.refresh, 1000000 / display.targetFps);
    myTimer.begin([this]{
        display.refresh();
        enc1.service();
        }, 1000000 / display.targetFps);
}

void FDController::loop()
{
    if (emChecker > 1)
    {
        emChecker = 0;
        // levels read every 4th
        // eq reads once per 32

        auto encval = enc1.getValue();
        auto encbtn = enc1.getButton();
        if(encval)
        {
            _volumeDB += encval;
            if(_volumeDB > 12) _volumeDB = 12;
            if(_volumeDB < -90) _volumeDB = -90;
            float fraction = pow10f((float)_volumeDB/20.0);
            uint32_t dspval = (uint32_t)(fraction * (float)0x0800000);
            LOG <<"Volume:" <<LOG.dec <<_volumeDB <<" fract:" <<fraction <<" dsp:" <<LOG.hex <<dspval <<LOG.endl;
            saveloadWrite(MOD_VOLUME_ALG0_TARGET_ADDR, dspval);
        }
        if(encbtn == ClickEncoder::Clicked)
        {
            _muteSPK = !_muteSPK;
            // mute speaker
            uint32_t dspval = (uint32_t)( (_muteSPK ? 0.0 : 1.0) * (float)0x0800000);
            saveloadWrite(MOD_MUTESPK_ALG0_MUTEONOFF_ADDR, dspval);
        }

        if(Serial.available() > 0)
        {
            // read the incoming byte:
            byte incomingByte = Serial.read();

            // say what you got:
            LOG <<"I received: " <<incomingByte <<LOG.endl;

            if(incomingByte == '?')
            {
                LOG <<"--------------------------------------------------------------------------" <<LOG.endl
                    <<"help: single key commands:" <<LOG.endl
                    <<"  + =\tVolume up 1 dB" <<LOG.endl
                    <<"  - /\tVolume down 1 dB" <<LOG.endl
                    <<"  SPACE\tturn on/off DSP communication - when off SigmaStudio can connect." <<LOG.endl
                    <<"  s/S\tmute speakers" <<LOG.endl
                    <<"--------------------------------------------------------------------------" <<LOG.endl;
            }
            if(incomingByte == ' ')
            {
                dspEnabled = !dspEnabled;
                LOG <<"DSP connection " <<((dspEnabled)?"enabled":"disabled") <<LOG.endl;
            }

            if(incomingByte == '+' || incomingByte == '-' || incomingByte == '=' || incomingByte == '/')
            {
                int16_t step = (incomingByte == '+' || incomingByte == '=') ? 1 : -1;
                _volumeDB += step;
                if(_volumeDB > 12) _volumeDB = 12;
                if(_volumeDB < -90) _volumeDB = -90;
                float fraction = pow10f((float)_volumeDB/20.0);
                uint32_t dspval = (uint32_t)(fraction * (float)0x0800000);
                LOG <<"Volume:" <<LOG.dec <<_volumeDB <<" fract:" <<fraction <<" dsp:" <<LOG.hex <<dspval <<LOG.endl;
                saveloadWrite(MOD_VOLUME_ALG0_TARGET_ADDR, dspval);
            }
            if(incomingByte == 's' || incomingByte == 'S')
            {
                _muteSPK = !_muteSPK;
                // mute speaker
                uint32_t dspval = (uint32_t)( (_muteSPK ? 0.0 : 1.0) * (float)0x0800000);
                saveloadWrite(MOD_MUTESPK_ALG0_MUTEONOFF_ADDR, dspval);
            }
        }

        if(dspEnabled)
        {

            // input things...
            const int16_t trapLevel1 = MOD_LVIN1_ALG0_SINGLEBANDLEVELDET3_VALUES;
            const int16_t trapLevel2 = MOD_LVIN2_ALG0_SINGLEBANDLEVELDET5_VALUES;
            // post-eq, distortion
            const int16_t trapLevel3 = MOD_LVPOSTEQ_ALG0_SINGLEBANDLEVELDET2_VALUES;
            const int16_t trapLevel4 = MOD_LVDISTORTION_ALG0_SINGLEBANDLEVELDET1_VALUES;

            uint8_t data[6 * 4];
            // Write to data capture reg 2074 and 2075
            const int16_t DC1 = 2074;
            // const int16_t DC2 = 2075;

            if (dspReadCycle % 4 == 0) // as long as we use only these 2, setup only once
            {
                // setup DC to read LevelMeters
                Wire.beginTransmission(_addr);
                Wire.write(DC1 >> 8);          // address high byte
                Wire.write(DC1 & 0xff);        // address low byte
                Wire.write(trapLevel1 >> 8);   // address high byte
                Wire.write(trapLevel1 & 0xff); // address low byte
                Wire.write(trapLevel2 >> 8);   // address high byte
                Wire.write(trapLevel2 & 0xff); // address low byte
                WireEndTransmission();
            }
            else
            {
                // setup DC to read Secondary LevelMeters
                Wire.beginTransmission(_addr);
                Wire.write(DC1 >> 8);          // address high byte
                Wire.write(DC1 & 0xff);        // address low byte
                Wire.write(trapLevel3 >> 8);   // address high byte
                Wire.write(trapLevel3 & 0xff); // address low byte
                Wire.write(trapLevel4 >> 8);   // address high byte
                Wire.write(trapLevel4 & 0xff); // address low byte
                WireEndTransmission();

                // select the index of the postEQ filter 0-8 / flat,100,200,400,800,1600,3200,6400,12800
                // kein block write, es knackt !!!
                if(dspReadCycle % 4 == 1)
                {
                    saveloadWrite(MOD_EQFILTERIDX_DCINPALG1_ADDR, dspReadCycle/4);
                }
            }

            // send addr to read, no stop condition... then read...
            Wire.beginTransmission(_addr);
            Wire.write(DC1 >> 8);                 // address high byte
            Wire.write(DC1 & 0xff);               // address low byte
            WireEndTransmission(false); // no stop before reading !
            // LOG << "r2: " <<result << LOG.endl;

            // read...
            byte readlen = 6; // 2x3
            Wire.requestFrom(_addr, readlen);
            int idx = 0;
            while (Wire.available()) // slave may send less than requested
            {
                data[idx++] = Wire.read(); // receive a byte as character
                if (idx > readlen)
                    break;
            }

            if (idx == 6)
            {
                // dcdone = true;
                // 24bit 5.19 fixpoint value. The overall shift is needed for negative values.
                int32_t value1 = (((int32_t)data[2] << 8) | ((int32_t)data[1] << 16) | ((int32_t)data[0] << 24)) >> 8;
                int32_t value2 = (((int32_t)data[5] << 8) | ((int32_t)data[4] << 16) | ((int32_t)data[3] << 24)) >> 8;

                // 5.19 fixpoint to float.
                float realval1 = -1.0f * (100.0f - (float)value1 / 5242.880f); // x / (1 << 19)
                float realval2 = -1.0f * (100.0f - (float)value2 / 5242.880f); // x / (1 << 19)

                if(dspReadCycle % 4 == 0)
                {
                    _levels.inL = realval1;
                    _levels.inR = realval2;
                }
                else
                {
                    if(dspReadCycle % 4 == 3)
                    {
                        _levels.postEQ[dspReadCycle/4] = realval1;
                    } 
                    _levels.distortion = realval2;
                }
            }
            else
            {
                // LOG << "ac: ??? idx:" << idx << LOG.endl;
            }

            dspReadCycle++; // next time read the others...

            if(dspReadCycle > 8*4)
            {
                dspReadCycle = 0;
            }

        } // dspEnabled

    }
    if (emDraw > 100)
    {
        emDraw = 0;
        uint32_t time = micros();
// #define DRAWBEBUG 1
#ifdef DRAWBEBUG

        display.fillScreen(frameCounter / 16);
        display.drawLine(0, 0, 31, 31, 0);
        display.drawLine(32, 0, 32 + 31, 31, 1);
        display.drawLine(64, 0, 64 + 31, 31, 2);
        display.drawLine(96, 0, 96 + 31, 31, 3);

        frameCounter = (frameCounter + 1) % 64;
        // display.drawRect(1,1,4,4,1);
        // display.drawRect(7,7,4,4,1);
        // LOG <<"-----------" <<LOG.endl;
#else
        /// draw random EQ
        // v2 ~ 31ms

        display.fillScreen(0);
        // display.drawLine(0, 10, 127, 10, 1);
        display.setTextSize(1);
        display.setCursor(textPos, 0);
        display.setTextWrap(false);
        display.setTextColor(1);
        display.print("suber");
        display.setTextColor(3);
        display.print("majo");
        display.setTextColor(2);
        display.print(" :-P");

        textPos -= 3;
        if (textPos < -120)
            textPos = 128;

        // for (int16_t x = 40; x < 120; x += 9)
        // {
        //     int16_t hi = random(16);
        //     // display.fillRect(x,16,7,31,0);
        //     display.fillRect(x, 32 - hi, 7, hi, 3);
        // }

        // volume
        int volx = 84;
        display.drawRoundRect(volx,0,44,20,3,1);
        display.setTextColor(2);
        display.setCursor(volx+4, 3);
        display.setTextSize(2);
        display.print(_volumeDB);

        // level from 0? to -100
        int16_t valL = max(_levels.inL, -32.0*2)  / -2; // should be within 0 - 32 ?
        display.fillRect(0, valL, 6, 32 - valL, 3);
        // LOG <<"draw l1 " <<_levels.inL <<" mapped vaL:" <<valL <<LOG.endl;
        int16_t valR = max(_levels.inR, -32.0*2)  / -2; // should be within 0 - 32 ?
        display.fillRect(8, valR, 6, 32 - valR, 3);

        // int16_t valE = max(_levels.postEQ[1], -32.0*2)  / -2; // should be within 0 - 32 ?
        // display.fillRect(20, valE, 3, 32 - valE, 3);
        int16_t valD = max(_levels.distortion, -32.0*2)  / -2; // should be within 0 - 32 ?
        display.fillRect(24, valD, 3, 32 - valD, 3);

        //EQ
        for(int i = 1; i < 8; i++)
        {
            // min -64, scaled down by -2 -> 32 pixel
            // min -64, scaled down by -3 -> 22 pixel
            // min -48, scaled down by -1.5 -> 32 pixel
            int16_t valE = max(_levels.postEQ[i], -48.0)  / -1.5; // should be within 0 - 32 ?
            display.fillRect(30 + 6*i, 8+ valE, 5, 2 /*32 - valE*/, 3);
        }

        //Stats
        display.drawRoundRect(96,21,32,11,3,1);
        display.setTextColor(_muteSPK?3:1);
        display.setCursor(101, 23);
        display.setTextSize(1);
        display.print("MUTE");


#endif

        display.swapBuffers();

        drawtime = micros() - time;
    }
    if (emLogger > 5000)
    {
        // display.fillScreen(0);
        emLogger = 0;

        LOG << LOG.dec << "gate time: " << display.getDisplayTime() 
            << "us, possible fps:" << 1000000 / display.getDisplayTime() / 44 
            << " current fps:" << 1000000 / display.getDisplayFps1() / 44 
            << " draw:" << drawtime << "us" 
            << LOG.endl;

        LOG << " L1: " << _levels.inL
            << "\tL2: " << _levels.inR
            << "\tEQ: " << _levels.postEQ
            << "\tDST: " << _levels.distortion
            << LOG.endl;
    }
}

void FDController::saveloadWrite(uint16_t address, uint32_t value)
{
    // 1. write data to 0x0810 (5 bytes for whatever reason - SigmaStudio does not show the 5th !)
    // 2. write target addr to 0x0815 - doc echt
    //    repeat for up to 5 val/addr pairs in to 0x0811,12,13,14 and 16,17,18,19
    // 3 write exec command to CoreRegister 0x081C < 0x0034 ? das default steht in so nem header.. 1C ? + IST bit 

    if(!dspEnabled) return;

    uint16_t coreregval = REG_COREREGISTER_IC_1_VALUE | R13_SAFELOAD_IC_1_MASK;

    // LOG <<LOG.hex <<"saveload hex: " <<LOG.endl;
    // LOG <<uint8_t(0x08);          // address high byte
    // LOG <<uint8_t(0x10);        // address low byte
    // LOG <<" ";
    // LOG <<uint8_t(value >> 24);   // address high byte
    // LOG <<uint8_t(value >> 16); // address low byte
    // LOG <<uint8_t(value >> 8);   // address high byte
    // LOG <<uint8_t(value); // address low byte
    // LOG <<LOG.endl;
    // LOG <<uint8_t(0x08);          // address high byte
    // LOG <<uint8_t(0x15);        // address low byte
    // LOG <<" ";
    // LOG <<uint8_t(address >> 8);   // address high byte
    // LOG <<uint8_t(address); // address low byte
    // LOG <<LOG.endl;
    // LOG <<uint8_t(REG_COREREGISTER_IC_1_ADDR >> 8);          // address high byte
    // LOG <<uint8_t(REG_COREREGISTER_IC_1_ADDR);        // address low byte
    // LOG <<" ";
    // LOG <<uint8_t(coreregval >> 8);   // address high byte
    // LOG <<uint8_t(coreregval); // address low byte
    // LOG <<LOG.dec <<LOG.endl;

    // return;

    uint8_t result = 0;
    Wire.beginTransmission(_addr);
    Wire.write(0x08);          // address high byte
    Wire.write(0x10);        // address low byte
    Wire.write(0);   // bits 39-32 ???
    Wire.write(value >> 24);   // address high byte
    Wire.write(value >> 16); // address low byte
    Wire.write(value >> 8);   // address high byte
    Wire.write(value & 0xff); // address low byte
    WireEndTransmission();

    Wire.beginTransmission(_addr);
    Wire.write(0x08);          // address high byte
    Wire.write(0x15);        // address low byte
    Wire.write(address >> 8);   // address high byte
    Wire.write(address); // address low byte
    WireEndTransmission();

    // // also write "step" for slew volumes ??? neee brauch mer nich
    // value = 0x00000800;
    // Wire.beginTransmission(_addr);
    // Wire.write(0x08);          // address high byte
    // Wire.write(0x11);        // address low byte
    // Wire.write(0);   // bits 39-32 ???
    // Wire.write(value >> 24);   // address high byte
    // Wire.write(value >> 16); // address low byte
    // Wire.write(value >> 8);   // address high byte
    // Wire.write(value & 0xff); // address low byte
    //  WireEndTransmission();
    // 
    // Wire.beginTransmission(_addr);
    // Wire.write(0x08);          // address high byte
    // Wire.write(0x16);        // address low byte
    // Wire.write(MOD_VOLUME_ALG0_STEP_ADDR >> 8);   // address high byte
    // Wire.write(MOD_VOLUME_ALG0_STEP_ADDR); // address low byte
    // WireEndTransmission();
    // 

    Wire.beginTransmission(_addr);
    Wire.write(REG_COREREGISTER_IC_1_ADDR >> 8);          // address high byte
    Wire.write(REG_COREREGISTER_IC_1_ADDR);        // address low byte
    Wire.write(coreregval >> 8);   // address high byte
    Wire.write(coreregval); // address low byte
    WireEndTransmission();
}

/// @brief Finish transmission, check result, stops DSP activity on error
void FDController::WireEndTransmission(boolean sendStop)
{
    uint8_t result = Wire.endTransmission(sendStop);
    if (result != 0)
    {
        LOG << "saveloadWrite:i2c error " << result << LOG.endl;
        dspEnabled = false;
    }
}