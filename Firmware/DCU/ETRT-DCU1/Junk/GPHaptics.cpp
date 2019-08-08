#include "GPHaptics.h"

void GPHaptics::begin(){
	vibMotor0.begin(PWM0_PIN, EN0_PIN, MODE0_PIN);
	vibMotor1.begin(PWM1_PIN, EN1_PIN, MODE1_PIN);
	wave0.begin(SINE);
	wave0.begin(SINE);
}


void GPHaptics::update(){
	vibMotor0.update();
	vibMotor1.update();
}


void GPHaptics::setVib0(float intensity){
	vibMotor0.setIntensity(intensity);
}


void GPHaptics::setVib1(float intensity){
	vibMotor1.setIntensity(intensity);
}
