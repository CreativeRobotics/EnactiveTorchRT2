#include "TOFLidarSensor.h"


void TOFLidarSensor::begin(){
	SENSOR_SERIAL.begin(SENSOR_BAUD);
	reset();
}
//-----------------------------------------------------

void TOFLidarSensor::begin(int adcCh){
	SENSOR_SERIAL.begin(SENSOR_BAUD);
	reset();
	setAuxADC(adcCh);
}
//-----------------------------------------------------

void TOFLidarSensor::reset(){
	lastUpdate = micros();
	deltaT = 0.01;
	range = 0.0; //combined value
	lastRange = 0.0;
	TOFRange = 0.0; //time of flight ranger value
	lastTOFRange = 0.0;
	TOFDelta = 0.0;
	rangeDelta = 0.0;
	
	serState = 0;
	newData = 0;
	dist_L = 0;
	dist_H = 0;
	strength_L = 0;
	strength_H = 0;
	rawQual = 0;
	checkSum_L = 0;
	checkSum = 0;
	checksumValid = 0;
	distance = 0;
	signalStrength = 0;
}
//-----------------------------------------------------

void TOFLidarSensor::update(){
	//Read the sensor, or update with the latest values
	//Process and serial data
	lastTOFRange = TOFRange;
	if(updateSerial()){
		//update everything because we have new data
		updateTimeDelta();
		newData = 1;
    updateChecksum();
    //if(checksumValid) updateData();
    unpackTOFData();
		updateAuxRanger();
		lastTOFRange = TOFRange;
		TOFRange = (float)distance; //distance collected from the TOF ranger is in cm and an int - convert to float
		TOFDelta = (TOFRange-lastTOFRange)/deltaT;
		merge();
		lastUpdate = millis();
	}
}
//-----------------------------------------------------

void TOFLidarSensor::merge(){
	//merge sensor readings
	//If the Aux ranger is closer than 30cm then use it, otherwise use the tof Lidar
	if( sharpRanger.output() < 30.0 ) range = sharpRanger.output();
	else range = TOFRange;
}
//-----------------------------------------------------

bool TOFLidarSensor::updateSerial(){
	//Parse serial port data
	char inByte;
  bool retVal = 0;
  while(SSerial.available()){
    inByte = SSerial.read();
    switch(serState){
      case 0: //waiting for frame header
        if(inByte == FRAME_HEADER) {
          serState++;
          checkSum = inByte;
        }
        break;
      case 1: //waiting for frameHeader2
        if(inByte == FRAME_HEADER) {
          serState++;
          checkSum += inByte;
        }
        else serState = 0;
        break;
      case 2: //Unpack Message
        dist_L = inByte;
        serState++;
        checkSum += inByte;
        break;
      case 3: //Unpack Message
        dist_H = inByte;
        serState++;
        checkSum += inByte;
        break;
      case 4: //Unpack Message
        strength_L = inByte;
        serState++;
        checkSum += inByte;
        break;
      case 5: //Unpack Message
        strength_H = inByte;
        serState++;
        checkSum += inByte;
        break;
      case 6: //Unpack Message
        serState++;
        checkSum += inByte;
      case 7: //Unpack Message
        rawQual = inByte;
        serState++;
        checkSum += inByte;
        break;
      case 8: //Unpack Message
        checkSum_L = inByte;
        serState= 0;
        retVal = 1;
        break;
      default:
        serState= 0;
        retVal = 0;
        //checkSum = 0;
        break;
    }
  }
  return retVal;
}
//-----------------------------------------------------


void TOFLidarSensor::setAuxADC(int adcCh){
	//specify an ADC chanel for the auxiluary Rangefinder (Used for short distances)
	rangerAnalogPin = adcCh;
}
//-----------------------------------------------------

void TOFLidarSensor::updateAuxRanger(){
	//update the auxilluary rangefinder
	if(rangerAnalogPin > -1)	sharpRanger.input(analogRead(rangerAnalogPin), deltaT);
	else sharpRanger.input(0, deltaT);

}
//-----------------------------------------------------

void TOFLidarSensor::updateTimeDelta(){
	deltaT =  ((float)micros()-(float)lastUpdate)/1000000.0;
	if(deltaT == 0.0) deltaT = 0.0000001; //.1 microsecond
}
//-----------------------------------------------------

void TOFLidarSensor::unpackTOFData(){

  uint16_t dTemp = dist_H;
  dTemp <<=8;
  distance = dTemp | dist_L;
  dTemp = strength_H;
  dTemp <<=8;
  signalStrength = dTemp | strength_L;
}

void TOFLidarSensor::updateChecksum(){
  //if(checkSum_L == checkSum) checksumValid = 1;
  if(checkSum_L == 0) checksumValid = 1;
  else checksumValid = 0;
}

