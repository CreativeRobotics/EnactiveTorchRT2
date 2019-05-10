//Linear Resonant Actuator object
//ETRT2

#ifndef DRV2603_H
#define DRV2603_H

#include "Arduino.h"

typedef void (*FUNC_PTR)(uint16_t);

class DRV2603{
public:
	void begin(int pwmPin, int enablePin);
	void begin(int pwmPin, int enablePin, int modePin);
  void setPWMFunction(FUNC_PTR pwmFunc);
	void update();
	void setIntensity(float intensity);
	void enableLra()		{digitalWrite(ENABLE_PIN, 1);}
	void disableLra()		{digitalWrite(ENABLE_PIN, 0);}
	void setLra(uint16_t intensity);
	void simpleSet(uint16_t intensity);
	void pulseLra(int intensity, int milliTime);
	void setMode(bool hMode) {digitalWrite(MODE_PIN, hMode);} //ERM mode = 0, LRA mode = 1
	
private:
	typedef enum lraState_t{
		LRA_OFF = 0,
		LRA_OVERDRIVING,
		LRA_ON,
		LRA_BRAKING,
	} lraState_t;
	void updateHaptic();
	FUNC_PTR pwmSet;
	const uint8_t LRA_RESPONSE = 30; //minimum time for LRA response
	const uint8_t LRA_OVERDRIVE = 128; //ammount of PWM to overdrive the LRA when starting
	const uint16_t LRA_MAX = 1023; //maxumum PWM value
	const uint16_t LRA_MIN = 512; //minimum PWM needed to start LRA
	const uint16_t ANALOG_FREQUENCY = 35156.25;
	
	int PWM_PIN = -1;
	int ENABLE_PIN = -1;
	int MODE_PIN = -1;
	bool mode = 0; //0 = LRA, 1 = ERM
	lraState_t lraState = LRA_OFF;
	float intensityPercent;
	
	uint16_t lraDuty = 0;
	unsigned long lraTimer = 0;
	unsigned long nextLraUpdate = 0;
	unsigned long pulseLength = 0;
	bool pulseActive = 0;
	bool initialised = 0;
	
};

#endif // DRV2603
