/*
  Example Code for VFD Level Meter

  See description on https://www.instructables.com/member/mariosis/instructables/
  
  This example code is in the public domain.
 */

#include <math.h>
#include <SPI.h>

#define RCK 2
#define SCK 3
#define SDA 4
SPISettings spiset(16000000, MSBFIRST, SPI_MODE0);

uint32_t gate = 100000;
byte     igate = 0;
uint16_t vgate[21] = {0, 0,50,100,150,200, 250,300,350,400,450, 500,550,600,650,700, 750,800,850,900,950};

//uint32_t sample = 512;
uint16_t asample = 0;
uint16_t rsample = 0;
const uint16_t oversample = 256*2;

uint16_t adcnt = 0;
uint32_t startMillis;

// the setup routine runs once when you press reset:
void setup() {
  SPI.begin();         
  Serial.begin(9600);
  // initialize the digital pin as an output.
  pinMode(RCK, OUTPUT);

  digitalWrite(RCK, LOW);

  pinMode(A0, INPUT);
  randomSeed(analogRead(0));

  delay(1000);

  // calculate a log scale (vgate)
  for(int i = 0; i < 21; i++)
  {
    double slope = 5;  // 10 will give lovest digit ~ -40db
    double vmax = 1024.0;
    double res = vmax/pow(5,((double)i)/slope);
    vgate[21-i] = (uint16_t)res;
    // log
    //Serial.print(i); Serial.print(": "); Serial.print(val); Serial.print(": "); Serial.println(20.*log10(vmax/val) );
    // invlog = pow
    Serial.print(20-i); Serial.print(": "); Serial.print((vmax-(vmax/20.)*i+1)); Serial.print(": "); Serial.println(res);
  }
  Serial.println();
  
  // Set ADIE in ADCSRA (0x7A) to enable the ADC interrupt.
  // Without this, the internal interrupt will not trigger.
  ADCSRA |= B00001000;
  // Set ADSC in ADCSRA (0x7A) to start the ADC conversion
  ADCSRA |=B01000000;
  ADMUX |= B11000000; //2.5V ref
}

void render(uint16_t peak, bool top = false)
{
  uint32_t val = 1;
  while(gate > 0) // over all gates
  {
    val = ~gate;
    if(peak > vgate[igate])
    {
      val = val & 0x0fffff;           //  ober 4 bits 8=obenlinks, 4=obenrechts, unten hat kurzschluss
    }
    if(top && peak > vgate[igate+1]) // darken frames below the peak
    {
      val = val | 0xf00000;           //  ober 4 bits 8=obenlinks, 4=obenrechts, unten hat kurzschluss
    }
    byte b3 = (val&0xff0000)>>16;
    byte b2 = (val&0x00ff00)>>8;
    byte b1 = (val&0x0000ff);
    SPI.beginTransaction(spiset);
    SPI.transfer(b3);
    SPI.transfer(b2);
    SPI.transfer(b1);
    SPI.endTransaction();
    // final act, RCK riseing edge
    digitalWrite(RCK, HIGH);
    digitalWrite(RCK, LOW);
    
    //Serial.print("gate:");
    //Serial.print(b3, HEX);
    //Serial.print(b2, HEX);
    //Serial.print(b1, HEX);
    //Serial.print(" igate:");
    //Serial.print(igate);
    //Serial.println();
    //delay(10);
    
    gate = gate >> 1;
    igate++;
  }

  if(gate == 0)
  {
    gate = 0x100000;
    igate = 0;
  }
  //clear
  SPI.beginTransaction(spiset);
  SPI.transfer(0xff);
  SPI.transfer(0xff);
  SPI.transfer(0xff);
  SPI.endTransaction();
  // final act, RCK riseing edge
  digitalWrite(RCK, HIGH);
  digitalWrite(RCK, LOW);
}

// the loop routine runs over and over again forever:
void loop() {
  //uint16_t randNumber = random(1024);
  //sample = (sample + 1) % (1024*8);
  render(rsample);
  render(rsample, true); // max only
  //render(rsample, true); // max only
  //Serial.print("val:"); Serial.print(sample/8); Serial.println();

  if(adcnt < oversample)
  {
    //if(asample < sample)
    //  asample = sample;
  }
  else
  {
    adcnt = 0;
    rsample = asample;// / oversample;
    asample = asample/2;
    uint32_t endMillis = micros() - startMillis;
    Serial.print("fps:"); Serial.print(1000000/endMillis); Serial.print(" "); Serial.print(rsample); Serial.println();
    startMillis = micros();
  }
}

// Interrupt service routine for the ADC completion
ISR(ADC_vect){

  // Done reading
  //readFlag = 1;
  
  // Must read low first
  uint16_t tmp = ADCL | (ADCH << 8);
  if(tmp > asample)
    asample = tmp;
  
  // Not needed because free-running mode is enabled.
  // Set ADSC in ADCSRA (0x7A) to start another ADC conversion
  ADCSRA |= B01000000;

  adcnt++;
}

