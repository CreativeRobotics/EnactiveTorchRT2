//Linear Resonant Actuator object
//ETRT2

#ifndef LRA_H
#define LRA_H

#include "Arduino.h"


class LRA {
public:
	begin();
	void update();
	void setIntensity(float intensity);
	
private:
	typedef enum lraState_t{
		LRA_OFF = 0,
		LRA_OVERDRIVING,
		LRA_ON,
		LRA_BRAKING,
	} lraState_t;
	const int LRA_MIN;
	const int LRA_MAX;
	const int LRA_OVERDRIVE;
	bool mode = 0; //0 = LRA, 1 = ERM
	bool initialised;
	lraState_t driveState = LRA_OFF;
	float intensityPercent;
	
};

#endif // LRA_H