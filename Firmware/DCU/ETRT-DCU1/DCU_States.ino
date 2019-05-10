//State machine functions for startup and run

void sleepDevice(){
  shutdownAudio();
  prepareToSleep();
  setSleepInt(1);
}

void restartDevice(){
  shutdownAudio();
  prepareToSleep();
  device.deviceState = DCU_SPEEPING;
}

void checkForOffSwitch(){
  updateBatteryVoltage();
  
  if(device.batteryVoltage < 2.5){
    SENSERIAL.println("checked off switch, going to sleep . . .");
    //sleepDevice();
  }
}

//The two main update functions for when the device is active
//===============================================================================================
dcuState_t updateIdle(){
  updateSystem(); //update the core system
  //Check the timer - return if it isn't time for an update
  if(!updateNow()) return device.deviceState;
  updateUser();
  
  if(device.logActive) updateDataLogs();
  return device.deviceState; //this can be set through a command
}
//===============================================================================================
dcuState_t updateExperiment(){
  updateSystem();
  //Check the timer - return if it isn't time for an update
  if(!updateNow()) return device.deviceState;
  updateUser();
  
  if(device.logActive) updateDataLogs();
  return device.deviceState; //this can be set through a command
}




dcuState_t runStartup(){
  //Call this until it changes the state
  initDCU();
  initLEDs();
  delay(500);
  initSerialPorts();
  initClock();
  #ifdef WAIT_FOR_USB
    while(!Serial){;}
  #endif
  #ifdef WAIT_FOR_BUTTON
    while(device.userButtonChanged == false){
      refreshUserControls();
    }
    device.userButtonChanged = false;
    while(device.userButtonState == true){
      refreshUserControls();
      //wait for release
    }
    delay(20);
    //reset
    device.userButtonChanged = false;
  #endif
  delay(500);
  if(Serial) bufferUSB.port->println("DCU State: USB ONLNE");
  //printDeviceState(bufferUSB);
  return DCU_STARTING_SDHC;
}

dcuState_t startSDHC(){
  
  if(device.printStartupState) printOnce("DCU State: Starting SDHC");
  setupSDHC();
  return DCU_STARTING_WIRELESS;
}

dcuState_t restartSDHC(){
  delay(250); //wait for card to be fully inserted
  startSD();
  return DCU_FINISHED_STARTUP;
}
//===============================================================================================
dcuState_t startWireless(){
  //Call this until it changes the state
  if(device.printStartupState) printOnce("DCU State: Starting ESP32");
  enableESP();
  //Loop for a second and collect any ESP boot info
  //The moment it goes online skip out
  settings.waitForReplyTimer = millis()+2000;
  //unsigned long endTime = millis()+1000;
  while(millis() < settings.waitForReplyTimer){
    updateComms();
    if(device.espOnline) {
      settings.waitForReplyTimer = 0;
      return DCU_STARTING_OUTPUTS;
    }
  }
  if(device.printStartupState) printOnce("DCU State: ESP32 Startup Failed");
  return DCU_STARTING_OUTPUTS;
}
//===============================================================================================
dcuState_t startOutputs(){
  //Init haptics or whatever else is in use at that end
  if(device.printStartupState) printOnce("DCU State: Starting Outputs");
  initOutputModule();
  return DCU_STARTING_INPUTS;
}
//===============================================================================================
dcuState_t startInputs(){
  //Init Sensors or whatever else is in use at that end
  if(device.printStartupState) printOnce("DCU State: Starting Inputs");
  device.sensorType = detectSensorType();
  if(device.sensorType == 0){
    device.sensorName = "SONAR";
  }
  if(device.printStartupState){
    String txt = "DCU State: Sensor detected: ";
    printOnce(txt + device.sensorName);
  }
  initInputModule();
  return DCU_START_READING_SETTINGS;
}
//===============================================================================================
dcuState_t startReadingSettings(){
  //Init Sensors or whatever else is in use at that end
  //enableESP();
  if(checkForSettings()){
    
    if(device.printStartupState) printOnce("DCU State: Reading Settings");
    startReadDeviceSettings();
    return DCU_READING_SETTINGS;
  }
  else if(device.printStartupState) printOnce("DCU State: No Settings File");
  return DCU_STARTING_IMU;
}
//===============================================================================================
dcuState_t readSettings(){
  //Call this until it changes the state
  //if(DEBUGGING) printOnce("DCU State: Reading settings from SD Card");
  if(readDeviceSettings()){
    while(millis() < settings.waitForReplyTimer) updateComms(); //wait here and handle any comm events
    settings.waitForReplyTimer = 0;
  }else{
    endReadDeviceSettings();
    if(device.printStartupState) printOnce("DCU State: Settings read complete");
    return DCU_STARTING_IMU;
  }
  return DCU_READING_SETTINGS;
}
//===============================================================================================
dcuState_t startIMU(){
  
  if(device.printStartupState) printOnce("DCU State: Starting IMU");
  tryStartIMU();
  //Apply any settings here
  return DCU_CONNECTING_WIFI;
}
//===============================================================================================

dcuState_t connectWiFi(){
  //Call this until it changes the state
  if(!settings.wifiSSIDOK || !settings.wifiPassOK){
    //Don't connect to WiFi
    if(device.printStartupState) printOnce("DCU State: Skipping WiFi Connect");
    return DCU_FINISHED_STARTUP;
  }
  if(device.printStartupState) printOnce("DCU State: Connecting to WiFi");
  String connectCmd = "connect to ";
  connectCmd += settings.ssid;
  connectCmd += ' ';
  connectCmd += settings.pswd;
  WIRELESS.println(connectCmd);
  //Connect to wifi and wait for command
  return DCU_WAITING_FOR_WIFI;
}

//===============================================================================================
dcuState_t waitForWiFi(){
  if(device.espWiFiConnected){
    device.espWiFiConnectionErrors = 0; //reset the counter
    return DCU_FINISHED_STARTUP;
  }
  else if(device.espWiFiConnectionErrors > 1) {
    
    if(device.printStartupState) printOnce("DCU State: Wifi connection failed");
    return DCU_FINISHED_STARTUP; //timed out
  }
  else return DCU_WAITING_FOR_WIFI;
}
//===============================================================================================
dcuState_t applyDefaults(){
  return DCU_FINISHED_STARTUP;
}
