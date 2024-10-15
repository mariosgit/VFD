//#ifdef RUN_FREEDSPCONTROLLER_MN12832L

#include "FDController.h"

#include "biquad.h" // copy from teensy audio, including from lib_deps draws other stuff in :(

// move to dspctrl..
#define SIGMASTUDIOTYPE_SPECIAL static_cast<int16_t> // ??? just an hex uint16_t, for the 12bit ProgCounter where the readback happens ?
#include "AMPx4-TDM-Test02/SigmaKram_IC_1_PARAM.h"
#include "AMPx4-TDM-Test02/SigmaKram_IC_1_REG.h"

const uint8_t pinSLat = 3;
const uint8_t pinSCLK = 4;
const uint8_t pinSIN  = 5;
const uint8_t pinFFblank = 6;

const uint8_t pinSCL = 16;
const uint8_t pinSDA = 17;

FDController::FDController() : display( 
        /* pinBLK = */ 0,
        /* pinLAT = */ 1,
        /* pinGCP = */ 2,
        /* MOSI_PIN = */ 11,
        /* SCK_PIN = */ 13),
        enc1(&input, 0x2000, 0x1000, 0x0800, 4, HIGH, HIGH),
        enc2(&input, 0x0080, 0x0040, 0x0020, 4, HIGH, HIGH),
        enc3(&input, 0x4000, 0x8000, 0x0008, 4, HIGH, HIGH),
        enc4(&input, 0x0100, 0x0200, 0x0400, 4, HIGH, HIGH),
        enc5(&input, 0x0002, 0x0001, 0x0004, 4, HIGH, HIGH)
{
    // encoder bits: rotation, button
    // enc1 lu           3000 0800
    // enc2 ld           00c0 0020
    // enc3 cc           c000 0008
    // enc4 ru           0300 0400
    // enc5 rd           0003 0004 
    enc1.setAccelerationEnabled(false);
    // enc2.setAccelerationEnabled(false); // dist
    enc4.setAccelerationEnabled(false);
    enc5.setAccelerationEnabled(false);

    // 74165 shift register with buttons/encoder inputs
    pinMode(pinSLat, OUTPUT);
    pinMode(pinSCLK, OUTPUT);
    pinMode(pinSIN, INPUT);
    
    // SDA/SCL avoid interference with DSP boot ?
    pinMode(16, INPUT);
    pinMode(17, INPUT);

    pinMode(pinFFblank, OUTPUT);
    digitalWrite(pinFFblank, HIGH);
    analogWriteFrequency(pinFFblank, 22000);
    analogWrite(pinFFblank, 200); // 50% duty, 488Hz flicker !!!
}

void FDController::setup()
{
    display.begin();
    randomSeed(analogRead(0));

    // Interrupt functions...
    // You need to trigger the refresh function regularly !
    // myTimer.begin(display.refresh, 1000000 / display.targetFps);
    refreshTimer.begin([this]{
        display.refresh();
        }, 400 ); // 1000000/ display.targetFps); // period in usec ~ 150, flicker stops around 500us that's 45fps
    inputTimer.begin([this]{
        if(!inputStuffEnabled) return; // sort off interrupt disable
        inputService();
        enc1.service();
        enc2.service();
        enc3.service();
        enc4.service();
        enc5.service();
        }, 1000);
}

/// @brief Read encoders via shift registers
void FDController::inputService() {
    // digitalWrite(pinDBG0, HIGH);
    uint32_t result = 0;
    digitalWrite(pinSLat, LOW);
    digitalWrite(pinSLat, HIGH);
    // only get 16 bit (2 shift registers)
    for (int bitt = 0; bitt < 16; bitt++)
    {
        // I get /Q as input !
        result |= ((digitalRead(pinSIN)) ? 0 : 1) << bitt;
        digitalWrite(pinSCLK, LOW);
        digitalWrite(pinSCLK, HIGH);
    }
    input = result;
    // digitalWrite(pinDBG0, LOW);

    inqueue[inqupos] = result;
    inqupos = (inqupos + 1) % 8;
}

void FDController::loop()
{
    taskChecker();
    taskInput();
    taskSerial();
    taskDisplay();
    taskLogger();
}

void FDController::taskChecker()
{
    if (emChecker > 1)
    {
        emChecker = 0;
        // levels read every 4th
        // eq reads once per 32

        dspctrl.readLevels();
    }
}

void FDController::taskInput()
{
    inputSlot = (inputSlot + 1) % 5; // 0-4 // Why ? Teensy sometimes crashes here, no idea ???

    // uuuhhh hangs some times ??? encoder readout disables interrupts !!! would be better to just disable the intervaltimer for it !? 
    if(emInput > 20)
    {
        emInput = 0;

        int16_t encval = 0;
        ClickEncoder::Button encbtn = ClickEncoder::Open;

        // encoder service runs within interrupt, this will stop it for a moment
        // read encoders
        //   1     4
        //      3 
        //   2     5
        inputStuffEnabled = false;
        if(inputSlot+1 == 1)
        {
            encval = enc1.getValue();
            encbtn = enc1.getButton();
        }
        if(inputSlot+1 == 2)
        {
            encval = enc2.getValue();
            encbtn = enc2.getButton();
        }
        if(inputSlot+1 == 3)
        {
            encval = enc3.getValue();
            encbtn = enc3.getButton();
        }
        if(inputSlot+1 == 4)
        {
            encval = enc4.getValue();
            encbtn = enc4.getButton();
        }
        if(inputSlot+1 == 5)
        {
            encval = enc5.getValue();
            encbtn = enc5.getButton();
        }
        inputStuffEnabled = true;

        if(inputSlot+1 == 2)
        {   // Encoder Left Bottom -> Distortion Level ?
            if(encval)
            {
                _distortion -= 0.1f * encval;
                if(_distortion > 10.0) _distortion = 10.0;
                if(_distortion < 0.1)  _distortion = 0.1;

                LOG <<"dist:" <<_distortion <<" dspval:" <<LOG.hex <<dspctrl.dsp.floatTo523(_distortion) <<LOG.dec <<LOG.endl;

                dspctrl.dsp.saveloadWrite(
                    MOD_SOFTCLIP1_ALG0_SOFTCLIPALGG21ALPHA_ADDR, dspctrl.dsp.floatTo523( _distortion ),
                    MOD_SOFTCLIP1_ALG0_SOFTCLIPALGG21ALPHAM1_ADDR, dspctrl.dsp.floatTo523( 1.0f / _distortion ) );

            }
        }
        if(inputSlot+1 == 1)
        {
            // Encoder Left Top -> select EQ
            if(encbtn == ClickEncoder::Clicked)
            {
            }
            if(encval)
            {
                if(encval > 0) dspctrl.eqBandNext();
                else dspctrl.eqBandPrev();
            }
        }
        if(inputSlot+1 == 4)
        {
            // Encoder Left Right -> change Gain
            if(encval)
            {
                dspctrl.eqVal(encval);
            }
        }

        if(inputSlot+1 == 3)
        {   // Encoder Center -> Volume
            if(encval)
            {
                _volumeDB += encval;
                if(_volumeDB > 12) _volumeDB = 12;
                if(_volumeDB < -90) _volumeDB = -90;

                dspctrl.setVolume(_volumeDB);
            }
        }

        if(inputSlot+1 == 5)
        {   // Encoder Right Bottom -> Volume, Muting
            if(encval)
            {
                _volumeDB += encval;
                if(_volumeDB > 12) _volumeDB = 12;
                if(_volumeDB < -90) _volumeDB = -90;

                dspctrl.setVolume(_volumeDB);
            }
            if(encbtn == ClickEncoder::Clicked)
            {
                // toggle muteSPK / muteHP / BOTH on
                _mute = (_mute + 1) % 3;
                // mute speaker
                dspctrl.dsp.saveloadWrite(MOD_MUTESPK_ALG0_MUTEONOFF_ADDR, dspctrl.dsp.floatTo523((_mute & MUTE_SPK_MASK ? 0.0 : 1.0)));
                // mute headphone
                dspctrl.dsp.saveloadWrite(MOD_MUTEHP_ALG0_MUTEONOFF_ADDR, dspctrl.dsp.floatTo523((_mute & MUTE_HP_MASK ? 0.0 : 1.0)));
            }
        }


    }
}

void FDController::taskSerial()
{
    if(emSerial > 100)
    {
        emSerial = 0;
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
                    <<"  m/M\tmute speakers" <<LOG.endl
                    <<"  qa ws ed rf - EQ settings up down"
                    <<"--------------------------------------------------------------------------" <<LOG.endl;
            }
            if(incomingByte == ' ')
            {
                dspOffCounter = 2*6*3; // one cycle is 5 sec
                dspctrl.dspEnabled = !dspctrl.dspEnabled;
                LOG <<"DSP connection " <<((dspctrl.dspEnabled)?"enabled":"disabled") <<LOG.endl;
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
                dspctrl.dsp.saveloadWrite(MOD_VOLUME_ALG0_TARGET_ADDR, dspval);
            }
            if(incomingByte == 'q' || incomingByte == 'a')
            {
                dspctrl.eqBand = 0;
                dspctrl.eqVal( (incomingByte=='q') ? 1 : -1 );
            }
            if(incomingByte == 'w' || incomingByte == 's')
            {
                dspctrl.eqBand = 1;
                dspctrl.eqVal( (incomingByte=='w') ? 1 : -1 );
            }
            if(incomingByte == 'm' || incomingByte == 'M')
            {
                // _muteSPK = !_muteSPK;
                // // mute speaker
                // uint32_t dspval = (uint32_t)( (_muteSPK ? 0.0 : 1.0) * (float)0x0800000);
                // saveloadWrite(MOD_MUTESPK_ALG0_MUTEONOFF_ADDR, dspval);
            }
        }
    }
}

void FDController::taskDisplay()
{
    if (emDraw > 100)
    {
        emDraw = 0;
        uint32_t time = micros();

        display.fillScreen(0);
        // display.drawLine(0, 10, 127, 10, 1);
        // display.setTextSize(1);
        // display.setCursor(textPos, 0);
        // display.setTextWrap(false);
        // display.setTextColor(1);
        // display.print("suber");
        // display.setTextColor(3);
        // display.print("majo");
        // display.setTextColor(2);
        // display.print(" :-P");

        textPos -= 3;
        if (textPos < -120)
            textPos = 128;

        // volume
        int volx = 84;
        int16_t  x1, y1;
        uint16_t w, h;
        char buf[5];
        sprintf(buf, "%d", _volumeDB);
        display.setTextColor(3);
        display.setTextSize(2);
        display.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
        // if(random(10) > 8) { LOG <<LOG.dec <<"x1:" <<x1  <<" x2:" <<y1  <<" w:"<<w  <<" h:" <<h <<LOG.endl; }
        display.drawRoundRect(volx,0,44,20,3,1);
        display.setCursor(volx+5+36-w, 3);
        display.print(buf);

        if(dspctrl.dspEnabled)
        {
            // levels from 0? to -100
            int16_t valL = max(dspctrl.levels.inL, -32.0*2)  / -2; // should be within 0 - 32 ?
            display.fillRect(0, valL, 6, 32 - valL, 3);
            // LOG <<"draw l1 " <<_levels.inL <<" mapped vaL:" <<valL <<LOG.endl;
            int16_t valR = max(dspctrl.levels.inR, -32.0*2)  / -2; // should be within 0 - 32 ?
            display.fillRect(8, valR, 6, 32 - valR, 3);
            // levels - distortion - todo rms it or read more often
            // int16_t valE = max(_levels.postEQ[1], -32.0*2)  / -2; // should be within 0 - 32 ?
            // display.fillRect(20, valE, 3, 32 - valE, 3);
            int16_t valD = max(dspctrl.levels.distortion, -32.0*2)  / -2; // should be within 0 - 32 ?
            display.fillRect(24, valD, 3, 32 - valD, 3);

            // Speki
            for(int i = 1; i < 8; i++)
            {
                // min -64, scaled down by -2 -> 32 pixel
                // min -64, scaled down by -3 -> 22 pixel
                // min -48, scaled down by -1.5 -> 32 pixel
                int16_t valE = max(dspctrl.levels.postEQ[i], -48.0)  / -1.5; // should be within 0 - 32 ?
                display.fillRect(30 + 6*i, 8+ valE, 5, 2 /*32 - valE*/, 3);
            }

            // EQ Settings
            for(int i = 0; i < 4; i++)
            {
                display.fillRect(36 + 12*i, 15-dspctrl.eqValues[i], 10, 2, (dspctrl.eqBand==i)?3:1);
            }
        }
        else
        {
            display.setTextColor(3);
            display.setTextSize(1);
            display.setCursor(0, 5);
            display.print("DSP\ncon\noff");
        }

        //Stats
        display.drawRoundRect(84,21,18,11,3,1);
        display.setTextColor(_mute & MUTE_HP_MASK ? 1:2);
        display.setCursor(88, 23);
        display.setTextSize(1);
        display.print("HP");
        if(_mute & MUTE_HP_MASK) display.drawLine(86, 30, 100, 23, 3); // strike through
        display.drawRoundRect(102,21,26,11,3,1);
        display.setTextColor(_mute & MUTE_SPK_MASK ?1:2);
        display.setCursor(107, 23);
        display.setTextSize(1);
        display.print("SPK");
        if(_mute & MUTE_SPK_MASK) display.drawLine(104, 30, 126, 23, 3); // strike through

        display.swapBuffers();

        drawtime = micros() - time;
    }
}

void FDController::taskLogger()
{
    if (emLogger > 5000)
    {
        emLogger = 0;

        LOG << LOG.dec << "gate time: " << display.getDisplayTime() 
            << "us, possible fps:" << 1000000 / display.getDisplayTime() / 44 
            << " current fps:" << 1000000 / display.getDisplayFps1() / 44 
            << " draw:" << drawtime << "us" 
            << LOG.endl;

        // LOG << " L1: " << _levels.inL
        //     << "\tL2: " << _levels.inR
        //     << "\tEQ: " << _levels.postEQ
        //     << "\tDST: " << _levels.distortion
        //     << LOG.endl;

        // LOG <<"Input: " <<LOG.hex <<input <<LOG.dec <<LOG.endl;
        
        if(!dspctrl.dspEnabled && dspOffCounter == 0)
        {
            Wire.setSCL(pinSCL);
            Wire.setSDA(pinSDA);
            Wire.begin();
            dspctrl.dspEnabled = true; // initial turn on after DSP boot, or restart after bus collisions (with SigmaStudio)
            LOG << "DSP connection ON" <<LOG.endl;
        }
        else if (dspOffCounter > 0)
        {
            dspOffCounter--;
            LOG << "DSP OFF until..." <<dspOffCounter  <<LOG.endl;
        }


        // // experimental... bandpass, looks useable.
        // float freq = random(100,15000);
        // float widthQ = float(random(20,500))/100.0f;
        // float coeffs[5];
        // getCoefficients<float>(coeffs, BiquadType::BAND_PASS, 0, freq, 48000, widthQ, true);
        // LOG <<"bandpass: " <<freq <<"Hz wq:" <<widthQ <<" bqs:";
        // for(int i = 0; i < 6; i++) { LOG <<coeffs[i] <<", "; }
        // LOG <<LOG.endl;
        // if(dspctrl.dspEnabled)
        // {
        //     dspctrl.dsp.saveloadWrite5(MOD_HPEQ1_ALG0_STAGE0_B0_ADDR, coeffs);
        // }

    }
}


// #endif
