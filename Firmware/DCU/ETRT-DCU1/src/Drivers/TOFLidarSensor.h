//Driver for general purpose Sensor Head 
//ETRT2
#ifndef TOFLIDARSENSOR_H
#define TOFLIDARSENSOR_H

#include "Arduino.h"
#include "SharpIR.h"

#define SSerial SENSOR_SERIAL //serial2
#define SENSOR_BAUD 115200 
#define SHARP_IR0 A4
#define SHARP_IR1 A5
#define FRAME_HEADER 0x59
class TOFLidarSensor {
public:
	void begin();
	void begin(int adcCh);
	void reset();
	void update();
	
	float getRange()		{return range;}
	float getDelta()		{return rangeDelta;}
	float getTOFRange()	{return TOFRange;}
	float getTOFDelta()	{return TOFDelta;}
	float getAuxRange()	{return sharpRanger.output();}
	float getAuxDelta()	{return sharpRanger.delta();}
	void setAuxADC(int adcCh);
private:
	void merge();
	bool updateSerial();
	void updateAuxRanger();
	void updateTimeDelta();
	void unpackTOFData(); //unpack using the latest data packet
	void updateChecksum();
	
	
	int rangerAnalogPin = -1;
	SharpIR sharpRanger;
	unsigned long lastUpdate;
	float deltaT = 0.01;
	float range; //combined value
	float lastRange = 0;
	float TOFRange; //time of flight ranger value
	float lastTOFRange = 0;
	float TOFDelta;
	float rangeDelta;

	
	int serState = 0;
	bool newData = 0;
	uint8_t dist_L, dist_H, strength_L, strength_H, rawQual, checkSum_L;
	uint8_t checkSum = 0;
	bool checksumValid = 0;
	uint16_t distance = 0;
	uint16_t signalStrength = 0;
};

#endif // TOFLIDARSENSOR_H