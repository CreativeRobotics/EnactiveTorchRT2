//Simple Waves

#include "SimpleWaves.h"

void SimpleWaves::begin(waveType_t wave){
	waveType = wave;
}
//-----------------------------------------------------------

//-----------------------------------------------------------

void SimpleWaves::update(){
	updateTimeDelta(); //update the time delta so we now how much to increment the waveform
  if(paused) return;
	switch(waveType){
		case SINE:
			updateSin();
			break;
		case SQUARE:
			updateSquare();
			break;
		case TRIANGLE:
			updateTriangle();
			break;
		case SAWF:
			updateSawF();
			break;
		case SAWB:
			updateSawB();
     break;
   case ONESHOT:
      updateOneshot();
      break;
   case HALFSHOT:
      updateHalfshot();
      break;
	}
}

void SimpleWaves::setType(waveType_t wave){
	if(waveType != wave) waveOut = 0.0; //reset the output first
	waveType = wave;
}
//-----------------------------------------------------------
void SimpleWaves::set(float frequency, float amplitude){
	freq = frequency;
	if(freq <= 0.0) {
	  freq = 0.001; //cannot be zero or < 0.0
    paused = true;
	} else paused = false;
	amp = amplitude;
	if(waveType == SQUARE) updateSquarePeriod();
  else if(waveType == ONESHOT || waveType == HALFSHOT){
    unsigned long per = ( 1000000 / (unsigned long)freq ) / 2;
    oneShotChange = micros() + per;
    oneShotEnd = oneShotChange + per;
    oneShotState = 1;
    waveOut = 1.0;
  }
}

//-----------------------------------------------------------
void SimpleWaves::set(float frequency, float amplitude, waveType_t wave){
	if(waveType != wave) waveOut = 0.0; //reset the output first
	waveType = wave;
	set(frequency, amplitude);
}
//-----------------------------------------------------------


uint16_t  SimpleWaves::getScaledAudio(uint16_t maxVal){
  float tmp;
  float wave = waveOut*amp;
  float halfMax = (float)maxVal/2.0;
  tmp = wave * halfMax;
  tmp += halfMax;
  if(tmp < 0.0) return 0;
  else if(tmp > (float)maxVal ) return maxVal;
  return (uint16_t)tmp;
}

void SimpleWaves::updateTimeDelta(){
	deltaT = ((float)micros() - (float)lastUpdate)/1000000.0;
	lastUpdate = micros();
}
//-----------------------------------------------------------


void SimpleWaves::updateSin(){
	//count up using time delta and frequency
	//Use a float to count then convert to an int - 
	//incriments of < 1 will be handled in the float and only shift the sin table to a new value when they accumulate by 1.0
	sinStep += ((float)SIN_TABLE_LENGTH * freq * deltaT);
	if(sinStep >= (float)SIN_TABLE_LENGTH ) sinStep = 0.0 + (sinStep - (float)SIN_TABLE_LENGTH);
	waveOut = getSin((uint16_t)sinStep);
}
//-----------------------------------------------------------
void SimpleWaves::updateSquare(){
	if(micros() > nextSquareUpdate){
		squareState = -squareState;
		waveOut = squareState;
		nextSquareUpdate = micros() + squarePeriod;
	}
}

//-----------------------------------------------------------
void SimpleWaves::updateTriangle(){
	waveOut += ( (freq*deltaT) * 4.0 ) * triangleDirection;
	//bounce
	if(waveOut > 1.0){
		waveOut = 1.0 - (waveOut-1.0);
		triangleDirection = -1.0;
	}else if(waveOut < -1.0){
		waveOut = -1.0 - (waveOut + 1.0);
		triangleDirection = 1.0;
	}
}
//-----------------------------------------------------------
//These should really use an int16 and rely on the variable wrapping around naturally
//Use an int16 for the incriment but divide it by 10 or more so you get fractional rates (alternative to using the float above)
void SimpleWaves::updateSawF(){
	waveOut += (freq*deltaT) * 2.0;
	//flip
	//if(waveOut > 1.0) waveOut =-1.0;
	//Should be:
	if(waveOut > 1.0) waveOut =-1.0 + (waveOut - 1.0);
}
//-----------------------------------------------------------
void SimpleWaves::updateSawB(){
	waveOut -= (freq*deltaT) * 2.0;
	//flip
	//if(waveOut < -1.0) waveOut =1.0;
	
	if(waveOut < -1.0) waveOut =1.0 + (waveOut + 1.0);
}
//-----------------------------------------------------------

void SimpleWaves::updateOneshot(){
  if(micros() > oneShotChange && oneShotState == 1){
    waveOut = -1.0;
    oneShotState = 2;
  }
  else if(micros() > oneShotEnd && oneShotState == 2){
    waveOut = 0.0;
    paused = true;
    oneShotState = 0;
  }
}


void SimpleWaves:: updateHalfshot(){
  if(micros() > oneShotEnd ){
    waveOut = 0.0;
    paused = true;
  }
}
float SimpleWaves::getSin(uint16_t index){
	uint16_t tIndex = index;
	int tmp;
	if(tIndex > SIN_TABLE_LENGTH-1) tIndex = 0;
	tmp = sinTable[tIndex] - SIN_TABLE_ZERO;
	return ((float)tmp)/((float)SIN_TABLE_ZERO); //scale to -1.0:1.0
}

void SimpleWaves::updateSquarePeriod(){
	//Convert htz to microseconds
	squarePeriod =  ( 1000000 / (unsigned long)freq ) / 2;
}
