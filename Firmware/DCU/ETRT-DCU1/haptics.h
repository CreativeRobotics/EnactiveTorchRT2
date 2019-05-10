//haptics.h

#include <SamDTimer.h>

#include "src/Drivers/SimpleWaves.h"
#include "src/Drivers/DRV2603.h"

#define PWM0_PIN   PA20
#define PWM1_PIN  PA21
#define EN0_PIN   PA22
#define EN1_PIN   PA23
#define MODE0_PIN PB16
#define MODE1_PIN PB17

#define TONE_PIN PA17
#define AUDIO_ENA_PIN PA18
#define AUDIO_ENB_PIN PA19

const uint16_t PWMMAX = 0x3FF;

SamDTimer audioTimer = SamDTimer(3);

DRV2603 vib0;
DRV2603 vib1;
SimpleWaves wave0;
SimpleWaves wave1;


volatile uint16_t audioOut0, audioOut1;

uint16_t audioLimit = 2047; //the maximum output of the DAC - anything over this is capped

void beepOn(){
  tone(TONE_PIN, 2000);
}

void beepOff(){
  noTone(TONE_PIN);
}

void beepFor(unsigned long toneDuration){
  tone(TONE_PIN, 2000, toneDuration);
}

void updateHaptics(){
  vib0.update();
  vib1.update();
}

void setWaveType(bool channel, String wType){
  waveType_t type;
  if(wType == "SINE") type = SINE;
  else if(wType == "SQUARE") type = SQUARE;
  else if(wType == "TRIANGLE") type = TRIANGLE;
  else if(wType == "SAWF") type = SAWF;
  else if(wType == "SAWB") type = SAWB;
  else if(wType == "ONESHOT") type = ONESHOT;
  else if(wType == "HALFSHOT") type = HALFSHOT;
  if(channel == 0){
    wave0.setType(type);
  }else{
    wave1.setType(type);
  }
}

void TC3_Handler(){
  SamDTimer::timerHandler(3);
}



void initDAC(){
  analogWriteResolution(12);
  analogWrite(DAC0, audioLimit/2); //initialise the DACs
  analogWrite(DAC1, audioLimit/2);
  //Options:DACR_EXT, DACR_VDDANA, DACR_EXT_BUFFERED, DACR_INT_BANDGAP
  setDACRef(DACR_INT_BANDGAP);
}

void audioTimerCallback()
{  
  
  //lastAudioTimer = micros();
  wave0.update(  );
  wave1.update(  );  
  //Get outputs, shift from +/- to unsigned range and scale to integer range.
  audioOut0 = wave0.getScaledAudio(audioLimit);
  audioOut1 = wave1.getScaledAudio(audioLimit);
  fastAnalogWrite(audioOut0, 0);
  fastAnalogWrite(audioOut1, 1);
  //debugTimer1 = micros()-lastAudioTimer; //log the elapsed time since the last call
}

void initAudioTimer(){
  audioTimer.configure(TC_CLOCK_PRESCALER_DIV256, // prescaler 64
                TC_COUNTER_SIZE_8BIT,   // bit width of timer/counter
                TC_WAVE_GENERATION_MATCH_PWM  // match style
                );
  audioTimer.setPeriodMatch(250, 46, 0); // With prescaler at 256 this interrupts approx every 250ms - for 4Khz jobs
  audioTimer.setCallback(true, TC_CALLBACK_CC_CHANNEL0, audioTimerCallback);  // set DAC in the callback
}



void enableAudio(bool chanel){
  if(chanel == 0) digitalWrite(AUDIO_ENA_PIN, HIGH);
  if(chanel == 1) digitalWrite(AUDIO_ENB_PIN, HIGH);
}

void disableAudio(bool chanel){
  if(chanel == 0) digitalWrite(AUDIO_ENA_PIN, LOW);
  if(chanel == 1) digitalWrite(AUDIO_ENB_PIN, LOW);
}

void enableAudio(){
  digitalWrite(AUDIO_ENA_PIN, HIGH);
  digitalWrite(AUDIO_ENB_PIN, HIGH);
}

void disableAudio(){
  digitalWrite(AUDIO_ENA_PIN, LOW);
  digitalWrite(AUDIO_ENB_PIN, LOW);
}

void enableAudioTimer(){
  audioTimer.enable(true);
}
void disableAudioTimer(){
  audioTimer.enable(false);
}
void shutdownAudio(){
  disableAudio();
  disableAudioTimer();
}

void initTcc0(){
  //Serial.println("Initialising TCC0");
  PinDescription pinDesc = g_APinDescription[PWM0_PIN];
  pinPeripheral(PWM0_PIN, PIO_TCC_PDEC);
  pinPeripheral(PWM1_PIN, PIO_TCC_PDEC);

  GCLK->PCHCTRL[GCLK_CLKCTRL_IDs[0]].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos); //use clock generator 0
  // -- Configure TCC
  Tcc* TCCx = (Tcc*) GetTC(pinDesc.ulPWMChannel);

  TCCx->CTRLA.bit.SWRST = 1;
  while (TCCx->SYNCBUSY.bit.SWRST);

  // Disable TCCx
  TCCx->CTRLA.bit.ENABLE = 0;
  while (TCCx->SYNCBUSY.bit.ENABLE);
  /*#define TCC_CTRLA_PRESCALER_DIV1    (TCC_CTRLA_PRESCALER_DIV1_Val  << TCC_CTRLA_PRESCALER_Pos)
    #define TCC_CTRLA_PRESCALER_DIV2    (TCC_CTRLA_PRESCALER_DIV2_Val  << TCC_CTRLA_PRESCALER_Pos)
    #define TCC_CTRLA_PRESCALER_DIV4    (TCC_CTRLA_PRESCALER_DIV4_Val  << TCC_CTRLA_PRESCALER_Pos)
    #define TCC_CTRLA_PRESCALER_DIV8    (TCC_CTRLA_PRESCALER_DIV8_Val  << TCC_CTRLA_PRESCALER_Pos)
    #define TCC_CTRLA_PRESCALER_DIV16   (TCC_CTRLA_PRESCALER_DIV16_Val << TCC_CTRLA_PRESCALER_Pos)
    #define TCC_CTRLA_PRESCALER_DIV64   (TCC_CTRLA_PRESCALER_DIV64_Val << TCC_CTRLA_PRESCALER_Pos)
    #define TCC_CTRLA_PRESCALER_DIV256  (TCC_CTRLA_PRESCALER_DIV256_Val << TCC_CTRLA_PRESCALER_Pos)
    #define TCC_CTRLA_PRESCALER_DIV1024 (TCC_CTRLA_PRESCALER_DIV1024_Val << TCC_CTRLA_PRESCALER_Pos)*/
  // Set prescaler to 1/256
  TCCx->CTRLA.reg = TCC_CTRLA_PRESCALER_DIV8 | TCC_CTRLA_PRESCSYNC_GCLK; //14Khz
  // Set TCx as normal PWM
  TCCx->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
  while ( TCCx->SYNCBUSY.bit.WAVE );
  while (TCCx->SYNCBUSY.bit.CC0 || TCCx->SYNCBUSY.bit.CC1);
  // Set the initial value
  TCCx->CC[0].reg = (uint32_t) 0; //set starting value to zero
  TCCx->CC[1].reg = (uint32_t) 0; //set starting value to zero
  while (TCCx->SYNCBUSY.bit.CC0 || TCCx->SYNCBUSY.bit.CC1);
  // Set PER to maximum counter value (resolution : 0xFF)
  TCCx->PER.reg = PWMMAX; //0xFF;
  while (TCCx->SYNCBUSY.bit.PER);
  // Enable TCCx
  TCCx->CTRLA.bit.ENABLE = 1;
  while (TCCx->SYNCBUSY.bit.ENABLE);
}

void setPWMDuty0(uint16_t value){    
  PinDescription pinDesc = g_APinDescription[PWM0_PIN];
  Tcc* TCCx = (Tcc*) GetTC(pinDesc.ulPWMChannel);
  while (TCCx->SYNCBUSY.bit.CTRLB);
  while (TCCx->SYNCBUSY.bit.CC0 || TCCx->SYNCBUSY.bit.CC1);
  TCCx->CCBUF[0].reg = (uint32_t) value;
  while (TCCx->SYNCBUSY.bit.CC0 || TCCx->SYNCBUSY.bit.CC1);
  TCCx->CTRLBCLR.bit.LUPD = 1;
  while (TCCx->SYNCBUSY.bit.CTRLB);
}

void setPWMDuty1(uint16_t value){    
  PinDescription pinDesc = g_APinDescription[PWM1_PIN];
  Tcc* TCCx = (Tcc*) GetTC(pinDesc.ulPWMChannel);
  while (TCCx->SYNCBUSY.bit.CTRLB);
  while (TCCx->SYNCBUSY.bit.CC0 || TCCx->SYNCBUSY.bit.CC1);
  TCCx->CCBUF[1].reg = (uint32_t) value;
  while (TCCx->SYNCBUSY.bit.CC0 || TCCx->SYNCBUSY.bit.CC1);
  TCCx->CTRLBCLR.bit.LUPD = 1;
  while (TCCx->SYNCBUSY.bit.CTRLB);
}


void initialiseHapticUnit(){
  wave0.begin(SINE);
  wave1.begin(SINE);
  
  vib0.begin(PWM0_PIN, EN0_PIN, MODE0_PIN);
  vib0.setPWMFunction(setPWMDuty0);
  vib0.setMode(1); //ERM mode = 0, LRA mode = 1
  
  vib1.begin(PWM1_PIN, EN1_PIN, MODE1_PIN);
  vib1.setPWMFunction(setPWMDuty1);
  vib1.setMode(1); //ERM mode = 0, LRA mode = 1
  
  pinMode(AUDIO_ENA_PIN, OUTPUT);
  pinMode(AUDIO_ENB_PIN, OUTPUT);
  pinMode(TONE_PIN, OUTPUT);
  disableAudio();
  //Initialise the DAC
  initDAC();
  //Start the Audio Timer for the waveforms
  initAudioTimer();
  enableAudioTimer();
  //Start the PWM timer for the haptics
  initTcc0();

  
  wave0.set(100, 0);
  wave1.set(100, 0);
  setPWMDuty0(0);
  setPWMDuty1(0);
}
  
