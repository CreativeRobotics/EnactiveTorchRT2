//Sharp ID
#include "SharpIR.h"

void SharpIR::begin(){
	sensorType = UNDEFINED;
}
void SharpIR::begin(sensorType_t type){
	sensorType = type;
}

void SharpIR:: setType(sensorType_t type){
	sensorType = type;
}
void SharpIR::input(int adcValue, float deltaT){
	lastRange = range;
	range = linearise(adcValue);
	rangeDelta = (range-lastRange)/deltaT;
}