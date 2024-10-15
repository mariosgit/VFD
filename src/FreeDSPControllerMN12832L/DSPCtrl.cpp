#include <mbLog.h>
#include "DSPCtrl.h"
#include "biquad.h"

#define SIGMASTUDIOTYPE_SPECIAL static_cast<int16_t> // ??? just an hex uint16_t, for the 12bit ProgCounter where the readback happens ?
#include "AMPx4-TDM-Test02/SigmaKram_IC_1_PARAM.h"
#include "AMPx4-TDM-Test02/SigmaKram_IC_1_REG.h"


DSPCtrl::DSPCtrl() : dsp()
{
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


        // We read 4x8 times
        // toggling between reading volume and others
        // could have used a second timer... :P

        // 0 1 2 3 4 5 6 7    8 9 10 11 12 13 14 15   16 17 18 19 20 21 22 23    24..29..31
        // |       |          |          |             |           |              |   |     read level meter
        //   0       1          2           3             4           5            6   7    set EQ to preset dspReadCycle/4
        //                                                                                  time for EQ to settle
        //       |       |            |           |             |           |        |    | read EQ

        if (dspReadCycle % 4 == 0) // as long as we use only these 2, setup only once
        {
            // setup DC to read LevelMeters
            dsp.setDataCapture(trapLevel1, trapLevel2);
        }
        else
        {
            // setup DC to read Secondary LevelMeters (post EQ and Distortion)
            dsp.setDataCapture(trapLevel3, trapLevel4);

            // select the index of the postEQ filter 0-8 / flat,100,200,400,800,1600,3200,6400,12800 Hz
            // no block write, -> sound crackls !!! useing saveload
            if(dspReadCycle % 4 == 1)
            {
                dspEnabled &= dsp.saveloadWrite(MOD_EQFILTERIDX_DCINPALG1_ADDR, dspReadCycle/4);
            }
        }

        int32_t value1, value2;
        dspEnabled &= dsp.readDataCapture(value1, value2);
        // if(random(50) == 10) { LOG <<"DC:" <<value1 <<", " <<value2 <<LOG.endl; }
        if (dspEnabled)
        {
            // magick conversion to db values !? Where did this come from FreeDSP demo code ?

            // (1<<19) = 524.288
            // 5242.880f = (1<<19) / 100

            // The captured data is in 5.19, 5.19 fixpoint to float.
            float realval1 = -1.0f * (100.0f - (float)value1 / 5242.880f); // x / (1 << 19)
            float realval2 = -1.0f * (100.0f - (float)value2 / 5242.880f); // x / (1 << 19)

            if(dspReadCycle % 4 == 0)
            {
                levels.inL = realval1;
                levels.inR = realval2;

                // try clean conversion !? Still strange value range !
                if(random(100) == 10)
                {
                    float from519 = dsp.from519(value1);
                    float logval1 = -100*(1.0f-from519); //20 * logf(from519); // nee is schon log ? Why times 100 ? SigmaSecrets??
                    // LOG <<"levels int:" <<LOG.dec <<value1 <<", db(old)" <<realval1 <<" float:" <<from519 <<" db(new):"<<logval1 <<LOG.dec <<LOG.endl;
                }
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

        dspReadCycle++;
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
    dspEnabled &= dsp.saveloadWrite(MOD_VOLUME_ALG0_TARGET_ADDR, dspval);
}

// from teensy audio filter_biquad, the one in biquad.h does not work
void setLowShelf(float frequency, float gain, float slope, float *coef)
{
    float A = powf(10.0, gain/40.0f);
    float w0 = frequency * (2.0f * 3.141592654f / 48000.0f);
    float sinW0 = sinf(w0);
    float cosW0 = cosf(w0);
    //generate three helper-values (intermediate results):
    float sinsq = sinW0 * sqrtf( (pow(a,2.0)+1.0)*(1.0/(double)slope-1.0)+2.0*a );
    float aMinus = (a-1.0)*cosW0;
    float aPlus = (a+1.0)*cosW0;
    float scale = 1.0 / ( (a+1.0) + aMinus + sinsq);
    /* b0 */ coef[0] =		A *	( (a+1.0) - aMinus + sinsq	) * scale;
    /* b1 */ coef[1] =  2.0*A * ( (a-1.0) - aPlus  			) * scale;
    /* b2 */ coef[2] =		A * ( (a+1.0) - aMinus - sinsq 	) * scale;
    /* a1 */ coef[3] = -2.0*	( (a-1.0) + aPlus			) * scale;
    /* a2 */ coef[4] =  		( (a+1.0) + aMinus - sinsq	) * scale;
}

void DSPCtrl::eqVal(int diff)
{
    int8_t current = eqValues[eqBand];
    current += diff;
    // LOG <<"current:" <<current <<LOG.endl;
    if(current < -16) current = -16;
    if(current > 16) current = 16;
    // LOG <<"current:" <<current <<LOG.endl;
    eqValues[eqBand] = current;

    const uint32_t adrs[4] = {
        MOD_ALLEQ1_ALG0_STAGE0_B0_ADDR,
        MOD_ALLEQ2_ALG0_STAGE0_B0_ADDR,
        MOD_ALLEQ3_ALG0_STAGE0_B0_ADDR,
        MOD_ALLEQ4_ALG0_STAGE0_B0_ADDR};

    float coeffs[5];
    auto algo = BiquadType::PEAKING;// (eqBand == 1 || eqBand == 2) ? BiquadType::PEAKING : (eqBand == 0) ? BiquadType::LOW_SHELF : BiquadType::HIGH_SHELF;
    getCoefficients<float>(coeffs, algo, (float)current, eqFreq[eqBand], 48000, .25f, (algo==BiquadType::PEAKING)?false:true);

    // if(eqBand == 0)    setLowShelf(eqFreq[eqBand], current, 1.0f, coeffs);

    LOG <<"algo:" <<(int)algo <<" [" <<eqBand <<"] " <<eqFreq[eqBand] <<"Hz g:" <<current <<" wq:" <<0.5f <<" bqs:";
    for(int i = 0; i < 6; i++) { LOG <<coeffs[i] <<", "; }
    LOG <<LOG.endl;
    if(dspEnabled)
    {
        dspEnabled &= dsp.saveloadWrite5(adrs[eqBand], coeffs);
    }

}
