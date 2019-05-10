//Sharp IR Rangefinder object
//Handles linearisation, scale to metric values, and calculating deltas
#ifndef SHARPIR_H
#define SHARPIR_H

#include "Arduino.h"

class SharpIR {
public:
	typedef enum sensorType_t{
		UNDEFINED = 0,
		S30,
		S80,
		S120,
		S500,
	} sensorType_t;
	void begin();
	void begin(sensorType_t type);
	void input(int adcValue, float deltaT);
	void setType(sensorType_t type);
	float output() {return range;}
	float delta() {return rangeDelta;}
private:
	sensorType_t sensorType;
	int rawValue;
	float range;
	float lastRange;
	float rangeDelta;
	//https://acroname.com/articles/linearizing-sharp-ranger-data
	//Equation: 
	/*
	*range = ( 1.0 / (m*rawValue+b) ) - k
	*
	*/
	const float m = 0.00014734050390452336;
	const float b = 0.00044202151171357008;
	const float k = 4.0;
	
	float linearise(int v) {return (1.0 / (m*(float)v+b)) - k;}
};
#endif //SHARPIR_H