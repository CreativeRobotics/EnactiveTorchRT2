//Linear Resonant Actuator object
#include "DRV2603.h"

void DRV2603::begin(int pwmPin, int enablePin){
	PWM_PIN = pwmPin;
	ENABLE_PIN = enablePin;
	
	//analogWriteResolution(10);
	//analogWriteFrequency(LRAPWM_pin, ANALOG_FREQUENCY);
	pinMode(ENABLE_PIN, OUTPUT); 
	pinMode(PWM_PIN, OUTPUT);
	delay(100);
	initialised = 1;
	
}

void DRV2603::begin(int pwmPin, int enablePin, int modePin){
	MODE_PIN = modePin;
	begin(pwmPin, enablePin);
}

void DRV2603::setPWMFunction(FUNC_PTR pwmFunc ){
  pwmSet = pwmFunc;
}

	

//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
void DRV2603::update(){
	updateHaptic();
}

void  DRV2603::setIntensity(float intensity){
	setLra(((uint16_t)intensity*1023.0));
}
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-
void  DRV2603::setLra(uint16_t intensity){
	if(millis() < nextLraUpdate) return; //prevent updates happening too fast
	nextLraUpdate = millis()+LRA_RESPONSE; //set the next minimum time between updates
	//Map intensity (0-1023) to viable WM (512-1023)
	lraDuty = map(intensity, 0, 1023, LRA_MIN, LRA_MAX);//(intensity/2)+LRA_MIN;
	
	//activate brake if intensity is zero, or exit of lra is already off
	if(lraState == LRA_OFF && lraDuty == LRA_MIN) return; //do nothing if everything is off
	else if(lraDuty == LRA_MIN){
    pwmSet(0);
		//analogWrite(PWM_PIN, 0); //zero PWM
		lraState = LRA_BRAKING; ////set state to braking
	}
	else{
		switch(lraState){
			case LRA_OFF: //enable the device then set the duty cycle with overdrive and change the state
				enableLra();
        pwmSet(constrain(lraDuty+LRA_OVERDRIVE, 0, LRA_MAX));
				//analogWrite(PWM_PIN, constrain(lraDuty+LRA_OVERDRIVE, 0, LRA_MAX));
				lraState = LRA_OVERDRIVING;
				break;
			case LRA_BRAKING: //switch back to overdrive - this should never happen because updates should always happen less frequently than braking period
				
        pwmSet(constrain(lraDuty+LRA_OVERDRIVE, 0, LRA_MAX));
        //analogWrite(PWM_PIN, constrain(lraDuty+LRA_OVERDRIVE, 0, LRA_MAX));
				lraState = LRA_OVERDRIVING;
				break;
			case LRA_OVERDRIVING: //change the duty but keep overdriving unless zero - this shoud never happen because the update frequency should always be less than the time spent overdriving
				
        pwmSet(constrain(lraDuty+LRA_OVERDRIVE, 0, LRA_MAX));
        //analogWrite(PWM_PIN, constrain(lraDuty+LRA_OVERDRIVE, 0, LRA_MAX));
				break;
			case LRA_ON: //just change the duty cycle
				
        pwmSet(constrain(lraDuty+LRA_OVERDRIVE, 0, LRA_MAX));
        //analogWrite(PWM_PIN, constrain(lraDuty, 0, LRA_MAX));
				break;
		}
	}
	//set the timeout variable
	lraTimer = millis()+LRA_RESPONSE;
}

void  DRV2603::simpleSet(uint16_t intensity){
	//just set the PWM, no overdrive or anything fancy
	if(intensity == 0){
		pwmSet(0);//analogWrite(PWM_PIN, 0);
		disableLra();
	}
	else{
		enableLra();
		pwmSet(intensity);//analogWrite(PWM_PIN, intensity);
	}
}


void DRV2603::pulseLra(int intensity, int milliTime){
	pulseLength = millis()+milliTime;
	pulseActive = 1;
	setLra(intensity);
}


//Update the haptic devices so they can be set and run in the background
void   DRV2603::updateHaptic(){
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
			case LRA_BRAKING: //end braking period, disable the LRA
				//Bluetooth.println("BrakeEnd");
				pwmSet(0);//analogWrite(PWM_PIN, 0);
				disableLra();
				lraState = LRA_OFF;
				break;
			case LRA_OVERDRIVING: //end overdrive period, set to the desired duty
				//Bluetooth.println("ODEnd");
				pwmSet(lraDuty);//analogWrite(PWM_PIN, lraDuty);
				lraState = LRA_ON;
				break;
		}
	}
}
