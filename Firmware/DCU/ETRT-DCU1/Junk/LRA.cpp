//Linear Resonant Actuator object
#include "LRA.h"

void begin(){
	setupPins();
	delay(100);
	initialised = 1;
	
}



void Haptics::setupPins(){
	
	//analogWriteResolution(10);
	//analogWriteFrequency(LRAPWM_pin, ANALOG_FREQUENCY);
	
	pinMode(LRAEN_pin, OUTPUT); 
	pinMode(LRAPWM_pin, OUTPUT); 

}
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
void Haptics::update(){
	updateHaptic();
}

//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-
void  Haptics::setLra(uint16_t intensity){
	if(millis() < nextLraUpdate) return; //prevent updates happening too fast
	nextLraUpdate = millis()+LRA_RESPONSE; //set the next minimum time between updates
	//Map intensity (0-1023) to viable WM (512-1023)
	lraDuty = map(intensity, 0, 1023, LRA_MIN, LRA_MAX);//(intensity/2)+LRA_MIN;
	
	//activate brake if intensity is zero, or exit of lra is already off
	if(lraState == LRA_OFF && lraDuty == LRA_MIN) return; //do nothing if everything is off
	else if(lraDuty == LRA_MIN){
		analogWrite(LRAPWM_pin, 0); //zero PWM
		lraState = LRA_BRAKING; ////set state to braking
	}
	else{
		switch(lraState){
			case LRA_OFF: //enable the device then set the duty cycle with overdrive and change the state
				enableLra();
				analogWrite(LRAPWM_pin, constrain(lraDuty+LRA_OVERDRIVE, 0, LRA_MAX));
				lraState = LRA_OVERDRIVING;
				break;
			case LRA_BRAKING: //switch back to overdrive - this should never happen because updates should always happen less frequently than braking period
				analogWrite(LRAPWM_pin, constrain(lraDuty+LRA_OVERDRIVE, 0, LRA_MAX));
				lraState = LRA_OVERDRIVING;
				break;
			case LRA_OVERDRIVING: //change the duty but keep overdriving unless zero - this shoud never happen because the update frequency should always be less than the time spent overdriving
				analogWrite(LRAPWM_pin, constrain(lraDuty+LRA_OVERDRIVE, 0, LRA_MAX));
				break;
			case LRA_ON: //just change the duty cycle
				analogWrite(LRAPWM_pin, constrain(lraDuty, 0, LRA_MAX));
				break;
		}
	}
	//set the timeout variable
	lraTimer = millis()+LRA_RESPONSE;
}

void  Haptics::simpleSet(uint16_t intensity){
	//just set the PWM, no overdrive or anything fancy
	if(intensity == 0){
		analogWrite(LRAPWM_pin, 0);
		disableLra();
	}
	else{
		enableLra();
		analogWrite(LRAPWM_pin, intensity);
	}
}


void Haptics::pulseLra(int intensity, int milliTime){
	pulseLength = millis()+milliTime;
	pulseActive = 1;
	setLra(intensity);
}


//Update the haptic devices so they can be set and run in the background
void   Haptics::updateHaptic(){
	//If the haptic is set to zero then the device must have a braking period of 30ms
	//If it is set to non zero then it has an overdrive period of 30ms
	//Check to see if a pulse command was sent - disable if pulse period has ended
	if(pulseActive && millis() > pulseLength){
		//end the pulse1Active
		setLra(0);
		pulseActive = 0;
	}
	//if the current time has gone past the time when we need to update, then update
	if(lraTimer < millis()){
		switch(lraState){
			lraTimer = millis()+LRA_RESPONSE; //reset the timer
			case LraBraking: //end braking period, disable the LRA
				//Bluetooth.println("BrakeEnd");
				analogWrite(LRAPWM_pin, 0);
				disableLra();
				lraState = LraOff;
				break;
			case LraOverdriving: //end overdrive period, set to the desired duty
				//Bluetooth.println("ODEnd");
				analogWrite(LRAPWM_pin, lraDuty);
				lraState = LraOn;
				break;
		}
	}
}