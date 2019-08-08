#include <Arduino.h>
#include "ZeroTimer.h"

unsigned long nextTime = 1000;
unsigned long wraps = 0;
unsigned long calls = 0;
unsigned long lastInt = 0;
unsigned long lastInterval = 0;
// timer tester
Adafruit_ZeroTimer zt4 = Adafruit_ZeroTimer(4);

//define the interrupt handlers
void TC4_Handler(){
  Adafruit_ZeroTimer::timerHandler(4);
}

// timer 4 callback, set dac output!
volatile uint16_t dacout=0;
void Timer4Callback0()
{
  calls++;
  lastInterval = micros()-lastInt;
  lastInt = micros();
  // we'll write the DAC by hand for speed
  // wait till it's ready
#if defined(__SAMD51__)
  while (DAC->SYNCBUSY.bit.DATA0);
  // and write the data
  DAC->DATA[0].reg = dacout++;
#else
  while (DAC->STATUS.reg & DAC_STATUS_SYNCBUSY);
  // and write the data
  DAC->DATA.reg = dacout++;
#endif

  // wraparound when we hit 10 bits
  if (dacout == 0x400) {
    dacout = 0;
    wraps++;
  }
}

void setup() {
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  analogWriteResolution(10);
  analogWrite(A0, 128); // initialize the DAC

  Serial.begin(115200);
  while(!Serial){};
  delay(1000);
  Serial.println("Timer callback tester");

  /********************* Timer #4, 8 bit, one callback with adjustable period */
  
  zt4.configure(TC_CLOCK_PRESCALER_DIV256, // prescaler
                TC_COUNTER_SIZE_8BIT,   // bit width of timer/counter
                TC_WAVE_GENERATION_MATCH_PWM  // match style
                );

  zt4.setPeriodMatch(250, 46, 0); // With prescaler at 256 this interrupts approx every 250ms - for 4Khz jobs
  //1st Arg: counter match?? - makes no difference. second arg: number of matches .. changes interval length
  zt4.setCallback(true, TC_CALLBACK_CC_CHANNEL0, Timer4Callback0);  // set DAC in the callback
  zt4.enable(true);

}

void loop() {
  if(millis() > nextTime){
    nextTime = millis()+500;
    Serial.print("Running . . . ");
    Serial.print("dacout = "); Serial.print(dacout);
    Serial.print(" Wraps = "); Serial.print(wraps);
    Serial.print(" Calls = "); Serial.print(calls);
    Serial.print(" Interval = "); Serial.print(lastInterval);
    Serial.println();
  }
}
