//Dual ranger sensor


void initTOFSensor(){
  Sensor.begin(A4);
}
void updateTOFSensor(){
  Sensor.update();
}

#define TRIG_PIN PA03//ADC0
#define PWM_PIN  PB05//ADC2
void initSonarSensor(){
  pinMode(PWM_PIN, INPUT);
}
