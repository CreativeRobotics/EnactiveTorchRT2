#include "GPSensor.h"


void GPSensor::begin(sensorMode_t sMode = NO_SENSOR){
	primarySensor = sMode;
	secondarySensor = NO_SENSOR;
	SENSOR_SERIAL.begin(SENSOR_BAUD);
}

void GPSensor::reset(){
	
}

void GPSensor::update(float deltaT){
	//Read the sensor, or update with the latest values
	//Process and serial data
	updateSerial();
}

void GPSensor::updateSerial(){
	
}