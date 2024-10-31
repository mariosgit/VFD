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
        // We read 5 times ? 
        //  0 is left/right input levels
        //  1,2,3,4, is Spectrum Analiysers 2 values each time, frequencies 64...8192
        //  5 is distortion level and post volume level
        // toggling between reading volume and others

        // todo: fix 8th frequency, post level could be sum behind speaker eq to track max level ?!
        //       merge input level as 0 in the mux and remove the 2 inputLevels, frees 60 more instructions

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

            if(dspReadCycle == 0)
            {
                levels.inL = realval1;
                levels.inR = realval2;

                // experiment... try clean conversion !? Still strange value range !
                if(random(100) == 10)
                {
                    float from519 = dsp.from519(value1);
                    float logval1 = -100*(1.0f-from519); //20 * logf(from519); // nee is schon log ? Why times 100 ? SigmaSecrets??
                    // LOG <<"levels int:" <<LOG.dec <<value1 <<", db(old)" <<realval1 <<" float:" <<from519 <<" db(new):"<<logval1 <<LOG.dec <<LOG.endl;
                }
            }
            else if(dspReadCycle == 5)
            {
                levels.distortion = realval1;
                levels.postall = realval2;
            }
            else
            {
                levels.postEQ[dspReadCycle*2-2] = realval1;
                levels.postEQ[dspReadCycle*2-1] = realval2;
            }
        }
        else
        {
            // LOG << "ac: ??? idx:" << idx << LOG.endl;
        }

        dspReadCycle++;
        if(dspReadCycle > 5)
        {
            dspReadCycle = 0;
        }


        if (dspReadCycle == 0) // as long as we use only these 2, setup only once
        {
            // setup DC to read LevelMeters
            dsp.setDataCapture(MOD_LVIN1_ALG0_SINGLEBANDLEVELDET3_VALUES,
                 MOD_LVIN2_ALG0_SINGLEBANDLEVELDET5_VALUES);
        }
        else
        {
            // setup DC to read Secondary LevelMeters (post EQ and Distortion)
            dsp.setDataCapture(MOD_LVPOSTEQ1_ALG0_SINGLEBANDLEVELDET2_VALUES,
                 MOD_LVPOSTEQ2_ALG0_SINGLEBANDLEVELDET1_VALUES);

            // select the index of the postEQ filter 0-8 / flat,100,200,400,800,1600,3200,6400,12800 Hz
            // no block write, -> sound crackls !!! useing saveload
            dspEnabled &= dsp.saveloadWrite(MOD_EQFILTERIDX_DCINPALG1_ADDR, dspReadCycle-1);
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

void DSPCtrl::eqVal(int diff)
{
    int8_t current = eqValues[eqBand];
    current += diff;
    // LOG <<"current:" <<current <<LOG.endl;
    if(current < -16) current = -16;
    if(current > 16) current = 16;
    // LOG <<"current:" <<current <<LOG.endl;
    eqValues[eqBand] = current;

    const uint32_t adrs[8] = {
        MOD_ALLEQ1_ALG0_STAGE0_B0_ADDR,
        MOD_ALLEQ2_ALG0_STAGE0_B0_ADDR,
        MOD_ALLEQ3_ALG0_STAGE0_B0_ADDR,
        MOD_ALLEQ4_ALG0_STAGE0_B0_ADDR,
        MOD_ALLEQ5_ALG0_STAGE0_B0_ADDR,
        MOD_ALLEQ6_ALG0_STAGE0_B0_ADDR,
        MOD_ALLEQ7_ALG0_STAGE0_B0_ADDR,
        MOD_ALLEQ8_ALG0_STAGE0_B0_ADDR
    };

    float coeffs[5];

    switch(eqBand)
    {
        // case 0:
        //     setLowShelf(coeffs, eqFreq[eqBand], eqValues[eqBand], 1.0f);
        //     break;
        // case 3:
        //     setHighShelf(coeffs, eqFreq[eqBand], eqValues[eqBand], 1.0f);
        //     break;
        default:
            getCoefficients<float>(coeffs, BiquadType::PEAKING, eqValues[eqBand], eqFreq[eqBand], AUDIO_SAMPLE_RATE_EXACT, .25f, false);
            break;
    }

    LOG <<"algo:" <<" [" <<eqBand <<"] " <<eqFreq[eqBand] <<"Hz g:" <<current <<" wq:" <<0.5f <<" bqs:";
    for(int i = 0; i < 5; i++) { LOG <<coeffs[i] <<", "; }
    LOG <<LOG.endl;
    if(dspEnabled)
    {
        dspEnabled &= dsp.saveloadWrite5(adrs[eqBand], coeffs);
    }
}

// from teensy audio filter_biquad, the one in biquad.h does not work
void DSPCtrl::setLowShelf(float *coef, float frequency, float gain, float slope)
{
    double a = powf(10.0, gain/40.0f);
    double w0 = frequency * (2.0f * 3.141592654f / AUDIO_SAMPLE_RATE_EXACT);
    double sinW0 = sinf(w0);
    double cosW0 = cosf(w0);
    //generate three helper-values (intermediate results):
    double sinsq = sinW0 * sqrtf( (powf(a,2.0)+1.0)*(1.0/(double)slope-1.0)+2.0*a );
    double aMinus = (a-1.0)*cosW0;
    double aPlus = (a+1.0)*cosW0;
    double scale = 1.0 / ( (a+1.0) + aMinus + sinsq);
    /* b0 */ coef[0] =		a *	( (a+1.0) - aMinus + sinsq	) * scale;
    /* b1 */ coef[1] =  2.0*a * ( (a-1.0) - aPlus  			) * scale;
    /* b2 */ coef[2] =		a * ( (a+1.0) - aMinus - sinsq 	) * scale;
    /* a1 */ coef[3] = -2.0*	( (a-1.0) + aPlus			) * scale;
    /* a2 */ coef[4] =  		( (a+1.0) + aMinus - sinsq	) * scale;
}

void DSPCtrl::setHighShelf(float *coef, float frequency, float gain, float slope = 1.0f)
{
    double a = powf(10.0, gain/40.0f);
    double w0 = frequency * (2.0f * 3.141592654f / AUDIO_SAMPLE_RATE_EXACT);
    double sinW0 = sinf(w0);
    //double alpha = (sinW0 * sqrt((a+1/a)*(1/slope-1)+2) ) / 2.0;
    double cosW0 = cosf(w0);
    //generate three helper-values (intermediate results):
    double sinsq = sinW0 * sqrtf( (powf(a,2.0)+1.0)*(1.0/(double)slope-1.0)+2.0*a );
    double aMinus = (a-1.0)*cosW0;
    double aPlus = (a+1.0)*cosW0;
    double scale = 1.0 / ( (a+1.0) - aMinus + sinsq);
    /* b0 */ coef[0] =		a *	( (a+1.0) + aMinus + sinsq	) * scale;
    /* b1 */ coef[1] = -2.0*a * ( (a-1.0) + aPlus  			) * scale;
    /* b2 */ coef[2] =		a * ( (a+1.0) + aMinus - sinsq 	) * scale;
    /* a1 */ coef[3] =  2.0*	( (a-1.0) - aPlus			) * scale;
    /* a2 */ coef[4] =  		( (a+1.0) - aMinus - sinsq	) * scale;
}