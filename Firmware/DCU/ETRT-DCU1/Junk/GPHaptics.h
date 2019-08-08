//Driver for general purpose haptic board
//ETRT2

#ifndef GPHAPTICS_H
#define GPHAPTICS_H

#include "Arduino.h"
#include "DRV2603.h"
#include "SimpleWaves.h"
#include <SamDTimer.h>

#define PWM0_PIN 	PA20
#define PWM1_PIN	PA21
#define EN0_PIN 	PA22
#define EN1_PIN		PA23
#define MODE0_PIN PB16
#define MODE1_PIN	PB17

#define AUDIO_ENA_PIN PA18
#define AUDIO_ENB_PIN PA19
/*
TC Timer 5 is used for audio waveform updates
TCC0 is used by DRV2603 for high frequency Haptic PWM
*/

void audioTimerCallback()
{  
  
  //lastAudioTimer = micros();
  waveForm1.update(  );
  waveForm2.update(  );  
  //Get outputs, shift from +/- to unsigned range and scale to integer range.
  audioOut1 = getScaledAudio(waveForm1.output(), audioLimit);
  audioOut2 = getScaledAudio(waveForm2.output(), audioLimit);
  fastAnalogWrite(audioOut1, 0);
  fastAnalogWrite(audioOut2, 1);
  debugTimer1 = micros()-lastAudioTimer; //log the elapsed time since the last call
}


typedef enum waveform_t{
	SINE_WAVE,
	SWUARE_WAVE,
	SAWTOOTH_WAVE,
} waveform_t;

class GPHaptics {
public:
	void begin();
	void update();
	void setVib0(float intensity);
	void setVib1(float intensity);
	void setWave0(float frequency, float amplitude);
	void setWave1(float frequency, float amplitude);
	
private:
void setWaveCallBack()
	DRV2603 vibMotor0;
	DRV2603 vibMotor1;
	SimpleWaves wave0;
	SimpleWaves wave1;
	
	SamDTimer audioTimer = SamDTimer(5);
	waveform_t wave0Type = SINE_WAVE;
	waveform_t wave1Type = SINE_WAVE;
	
};

#endif // GPHAPTICS_H