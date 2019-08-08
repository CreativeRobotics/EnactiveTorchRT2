//Driver for general purpose Sensor Head 
//ETRT2
#ifndef GPSENSOR_H
#define GPSENSOR_H

#include "Arduino.h"

#define SSerial SENSOR_SERIAL //serial2
#define SENSOR_BAUD 115200 //needs to be different for different sensors
class GPSensor {
public:
	typedef enum sensorMode_t{
		NO_SENSOR = 0,
    TOF_LIDAR,
		IR_RANGERS,
		MAX_SONAR,
		TERABEE_SOLO,
		TERABEE_64,
		VISIBLE_LIGHT
	} sensorMode_t;
	
	void begin(sensorMode_t sMode = NO_SENSOR);
	void reset();
	void update(float deltaT);
	
	float getPrimaryValue();
	
private:
	sensorMode_t primarySensor;
	sensorMode_t secondarySensor;
	
};

#endif // GPSENSOR_H