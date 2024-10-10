#include <Wire.h>

#include <mbLog.h>
#include "DSPCtrl.h"

#define SIGMASTUDIOTYPE_SPECIAL static_cast<int16_t> // ??? just an hex uint16_t, for the 12bit ProgCounter where the readback happens ?
#include "AMPx4-TDM-Test02/SigmaKram_IC_1_PARAM.h"
#include "AMPx4-TDM-Test02/SigmaKram_IC_1_REG.h"


DSPCtrl::DSPCtrl()
{
    // DSP default adr
    addr = 0x68 >> 1;


}

void DSPCtrl::readLevels()
{
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
            Wire.beginTransmission(addr);
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
            Wire.beginTransmission(addr);
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
        Wire.beginTransmission(addr);
        Wire.write(DC1 >> 8);                 // address high byte
        Wire.write(DC1 & 0xff);               // address low byte
        WireEndTransmission(false); // no stop before reading !
        // LOG << "r2: " <<result << LOG.endl;

        // read...
        byte readlen = 6; // 2x3
        Wire.requestFrom(addr, readlen);
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
                levels.inL = realval1;
                levels.inR = realval2;
            }
            else
            {
                if(dspReadCycle % 4 == 3)
                {
                    levels.postEQ[dspReadCycle/4] = realval1;
                } 
                levels.distortion = realval2;
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

void DSPCtrl::setVolume(int16_t volumeDB)
{
    float fraction = pow10f((float)volumeDB/20.0);
    uint32_t dspval = (uint32_t)(fraction * (float)0x0800000);
    LOG <<"Volume:" <<LOG.dec <<volumeDB <<" fract:" <<fraction <<" dsp:" <<LOG.hex <<dspval <<LOG.endl;
    saveloadWrite(MOD_VOLUME_ALG0_TARGET_ADDR, dspval);

}

void DSPCtrl::saveloadWrite(uint16_t address, uint32_t value)
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
    Wire.beginTransmission(addr);
    Wire.write(0x08);          // address high byte
    Wire.write(0x10);        // address low byte
    Wire.write(0);   // bits 39-32 ???
    Wire.write(value >> 24);   // address high byte
    Wire.write(value >> 16); // address low byte
    Wire.write(value >> 8);   // address high byte
    Wire.write(value & 0xff); // address low byte
    WireEndTransmission();

    Wire.beginTransmission(addr);
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

    Wire.beginTransmission(addr);
    Wire.write(REG_COREREGISTER_IC_1_ADDR >> 8);          // address high byte
    Wire.write(REG_COREREGISTER_IC_1_ADDR);        // address low byte
    Wire.write(coreregval >> 8);   // address high byte
    Wire.write(coreregval); // address low byte
    WireEndTransmission();
}

void DSPCtrl::saveloadWrite(uint16_t address1, uint32_t value1, uint16_t address2, uint32_t value2)
{
    // 1. write data to 0x0810 (5 bytes for whatever reason - SigmaStudio does not show the 5th !)
    // 2. write target addr to 0x0815 - doc echt
    //    repeat for up to 5 val/addr pairs in to 0x0811,12,13,14 and 16,17,18,19
    // 3 write exec command to CoreRegister 0x081C < 0x0034 ? das default steht in so nem header.. 1C ? + IST bit 

    if(!dspEnabled) return;

    uint16_t coreregval = REG_COREREGISTER_IC_1_VALUE | R13_SAFELOAD_IC_1_MASK;

    uint8_t result = 0;
    Wire.beginTransmission(addr);
    Wire.write(0x08);          // address high byte
    Wire.write(0x10);        // address low byte
    Wire.write(0);   // bits 39-32 ???
    Wire.write(value1 >> 24);   // address high byte
    Wire.write(value1 >> 16); // address low byte
    Wire.write(value1 >> 8);   // address high byte
    Wire.write(value1 & 0xff); // address low byte
    WireEndTransmission();

    Wire.beginTransmission(addr);
    Wire.write(0x08);          // address high byte
    Wire.write(0x15);        // address low byte
    Wire.write(address1 >> 8);   // address high byte
    Wire.write(address1); // address low byte
    WireEndTransmission();

    Wire.beginTransmission(addr);
    Wire.write(0x08);          // address high byte
    Wire.write(0x11);        // address low byte
    Wire.write(0);   // bits 39-32 ???
    Wire.write(value2 >> 24);   // address high byte
    Wire.write(value2 >> 16); // address low byte
    Wire.write(value2 >> 8);   // address high byte
    Wire.write(value2 & 0xff); // address low byte
    WireEndTransmission();

    Wire.beginTransmission(addr);
    Wire.write(0x08);          // address high byte
    Wire.write(0x16);        // address low byte
    Wire.write(address2 >> 8);   // address high byte
    Wire.write(address2); // address low byte
    WireEndTransmission();

    Wire.beginTransmission(addr);
    Wire.write(REG_COREREGISTER_IC_1_ADDR >> 8);          // address high byte
    Wire.write(REG_COREREGISTER_IC_1_ADDR);        // address low byte
    Wire.write(coreregval >> 8);   // address high byte
    Wire.write(coreregval); // address low byte
    WireEndTransmission();
}


/// @brief Finish transmission, check result, stops DSP activity on error
void DSPCtrl::WireEndTransmission(boolean sendStop)
{
    uint8_t result = Wire.endTransmission(sendStop);
    if (result != 0)
    {
        LOG << "saveloadWrite:i2c error " << result << LOG.endl;
        dspEnabled = false;
    }
}
