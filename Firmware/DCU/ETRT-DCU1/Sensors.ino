//Dual ranger sensor


void initTOFSensor(){
  Sensor.begin(A4);
}
void updateTOFSensor(){
  Sensor.update();
}

//#define TRIG_PIN PA03//ADC0
//#define PWM_PIN  PB05//ADC2
void initSonarSensor(){
  if(DEBUGGING) DEBUG.println("init Sonar ...");
  //setupSonar(); //PULSE IN on  A2
}

void updateSonar(){
  device.inputs[2] = getSonarRange();
}


/*
void setupSonar(){
  pinMode(pulsePin, INPUT);
  //pinMode(enPin, OUTPUT);
  //attachInterrupt(digitalPinToInterrupt(pulsePin), pulsePin_isr, CHANGE);
  
  //digitalWrite(enPin, HIGH);
}*/


float getSonarRange(){
  //147 microseconds per inch
  range = (float)pulseLength;
  range /=147.0;
  range *=25.4;
  return range;
}
void sonarPulsePin_isr(){
  if(digitalRead(pulsePin)) riseTime = micros();
  else pulseLength = micros()-riseTime;
  sonarInts++;
}
