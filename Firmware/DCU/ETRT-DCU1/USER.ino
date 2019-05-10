
void updateUser(){
  //Update function for user editing
  userButtonAction(); //check for activity on the user button
  userSwitchAction(); //check for activity on the user slide switch
  if(device.userButtonState == true)  mapSensors();
  else noMapSensors();
}

//Some useful constants for positions in the inut and output arrays:
#define LIDAR_RANGE       2
#define VIB0_OUT          0
#define VIB1_OUT          1
#define AUDIO0_FREQ       2
#define AUDIO0_AMPLITUDE  3
#define AUDIO1_FREQ       4
#define AUDIO1_AMPLITUDE  5

void mapSensors(){
  //the main function for mapping sensor inputs to haptic outputs
  /*
  There are six input chanels and six output channels
  outputs 0-1 are vibration Strength
  outputs 2-3 and 4-5 are frequency and amplititude of analog output 0 and 1
  */
  updateInputs(); //copy current sensor values to the inputs array   

  //map TOF lidar range to Vib motor and transducer
  float range = device.inputs[LIDAR_RANGE];//TOF range
  if(range > 150.0) range = 150.0;
  float vibIntensity = (150-range) * 0.0067;
  if(vibIntensity > 1.0) vibIntensity = 1.0;
  device.outputs[VIB0_OUT] = vibIntensity;
  device.outputs[VIB1_OUT] = vibIntensity;
  device.outputs[AUDIO0_FREQ] = device.audioFreq0;
  device.outputs[AUDIO1_FREQ] = device.audioFreq1;
  device.outputs[AUDIO0_AMPLITUDE] = vibIntensity;
  device.outputs[AUDIO1_AMPLITUDE] = vibIntensity;

  setOutputs(); //Apply the outputs array to the haptics system
}

void noMapSensors(){
  device.outputs[VIB0_OUT] = 0.0;
  device.outputs[VIB1_OUT] = 0.0;
  device.outputs[AUDIO0_AMPLITUDE] = 0.0;
  device.outputs[AUDIO1_AMPLITUDE] = 0.0;
  setOutputs(); //Apply the outputs array to the haptics system
}
//===============================================================================================

void userSwitchAction(){
  if(device.userSwitchChanged == false) return; //do nothing if it hasn't changed
  device.userSwitchChanged = false; //reset the change flag
  //Do something because the switch has changed
  if(device.userSwitchState == true){
    device.logSD = true;
  }else if(device.userSwitchState == false){
    device.logSD = false;
  }
}
//===============================================================================================
void userButtonAction(){
  if(!device.userButtonChanged) return;
  device.userButtonChanged = false;
  if(device.userButtonState)        buttonOnAction();
  else if(!device.userButtonState) buttonOffAction();
}
//===============================================================================================
void buttonOnAction(){
  setLEDs(redLED);
  if(device.buttonEnableHaptics){
    //testHapticsStart();
    enableAudio();
    if(DEBUGGING) DEBUG.println("DEBUG: Button: Haptics On");
  }
  if(device.buttonStartLog){
    startLog();
    if(DEBUGGING) DEBUG.println("DEBUG: Button: Log On");
  }
}
//===============================================================================================

void buttonOffAction(){
  setLEDs(whiteLED);
  if(device.buttonEnableHaptics){
    //testHapticsStop();
    //beepOff();
    disableAudio();
    if(DEBUGGING) DEBUG.println("DEBUG: Button: Haptics Off");
  }
  if(device.buttonStartLog){
    stopLog();
    if(DEBUGGING) DEBUG.println("DEBUG: Button: Log Off");
  }
}
