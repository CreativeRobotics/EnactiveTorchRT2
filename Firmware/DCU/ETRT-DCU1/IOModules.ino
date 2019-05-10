
uint8_t detectSensorType(){
  //ADC1 can be tied to GND PB04
  //ADC3 can be tied to GND PB04
  //Set ADC1 to input pullup, then check state.
  //If high then you have a LIDAR
  //If low then you have Sonar
  uint8_t retVal = 0;
  pinMode(PB04, INPUT_PULLUP);
  delay(1);
  if(digitalRead(PB04)) bitSet(retVal, 0);
  return retVal;
}
void initInputModule(){
  //init sensors
  if(device.sensorType == 1){
    initTOFSensor(); //put whatever init function for whatever sensor module is in use here
  }else if(device.sensorType == 0){
    initSonarSensor(); //put whatever init function for whatever sensor module is in use here
  }
}

void initOutputModule(){
  initialiseHapticUnit(); //put whatever init function for whatever haptic module is in use here
}

void updateIO(){
  
  updateTOFSensor();
  //updateInputs(); //copy sensor data to DCU
  updateHaptics();
}

void updateInputs(){
  device.inputs[0] = Sensor.getRange();
  device.inputs[1] = Sensor.getDelta();
  device.inputs[2] = Sensor.getTOFRange();
  device.inputs[3] = Sensor.getTOFDelta();
  device.inputs[4] = Sensor.getAuxRange();
  device.inputs[5] = Sensor.getAuxDelta();
}


void setOutputs(){
  //outputs 0-1 are vibration Strength
  //outputs 2-3 and 4-5 are frequency and amplititude of analog output 0 and 1
  //vib0.setIntensity(device.outputs[0]);
  //vib1.setIntensity(device.outputs[1]);
  
  vib0.simpleSet( (int)(device.outputs[0]*1023 ) );
  vib1.simpleSet( (int)(device.outputs[1]*1023 ) );
  wave0.set(device.outputs[2], device.outputs[3]);
  wave1.set(device.outputs[4], device.outputs[5]);
}
