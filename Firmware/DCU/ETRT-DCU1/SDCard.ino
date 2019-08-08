
/*
 * IMPORTANT: The SD Card is powered from the peripheral voltage regulator so it can be shut down when the system is sleeping (when only the RTC is running)
 * The SD Card will not work properly when the peripheral power supply is OFF.
 * 
 * The Settings file will be looked for on initialisation and read if found
 * The settings file gets passed to the command handler and is read just as if it is a series of commands sent over the serial port
*/
void setupSDHC(){
  //Set up the command buffer
  //if(DEBUGGING) DEBUG.println("Resetting Buffer");
  //if(DEBUGGING) delay(100);
  resetBuffer(bufferSD);
  //Set to read from a file instead of serial
  //if(DEBUGGING) DEBUG.println("Set port mode");
  setPortMode(bufferSD, FILEREADER_COMMAND);
  //Set the serial port so we can still print replies
  //if(DEBUGGING) DEBUG.println("Set Primary Port");
  bufferSD.port = &USBS;
  //if(DEBUGGING) DEBUG.println("Reserving buffer");
  bufferSD.bufferString.reserve(SBUFFER);
  //if(DEBUGGING) DEBUG.println("Reserving Pending Buffer");
  bufferSD.pendingCommandString.reserve(SBUFFER);
  //if(DEBUGGING) DEBUG.println("Checking Power");
  if(!device.peripheralPowerOn){
    printErln("SDCard not powered up");
    return;
  }
  //if(DEBUGGING) DEBUG.println("Setting Pins");
  pinMode(SDCD, INPUT_PULLUP);
  //if(DEBUGGING) DEBUG.println("Setting SDWP");
  //pinMode(SDWP, INPUT_PULLUP);
  pinMode(SDWP, OUTPUT);
  //if(DEBUGGING) DEBUG.println("Setting SS");
  pinMode(SS, OUTPUT);
  //if(DEBUGGING) DEBUG.println("Setting SDWP state");
  digitalWrite(SDWP, 0); //SDWP connects to SDCD when a card is inserted.

  //if(DEBUGGING) DEBUG.println("Attaching int");
  //attachInterrupt(digitalPinToInterrupt(SDCD), cardDetect_isr, CHANGE);
  if(DEBUGGING) DEBUG.println("Checking for card");
  //delay(100);
  startSD();
  
}

void startSD(){
  device.cardPresent = cardInserted();
  //if(DEBUGGING && device.cardPresent) DEBUG.println("Card detected");
  //if(DEBUGGING && !device.cardPresent) DEBUG.println("Card not detected");
  //if(DEBUGGING) DEBUG.println("Attempting to initialise card");
  if(device.cardPresent) tryInitSDCard();
  else printErln("SDCard not present");
}

void tryInitSDCard(){
  if(!device.peripheralPowerOn){
    printErln("DEBUG: SDCard not powered up");
    return;
  }
  if (!SD.begin(SS, SD_SCK_MHZ(50))) {
    device.cardInitError = true;
    printErln("DEBUG: SDCard Init Error");
  }
  else{
    //SD.initErrorHalt();
    device.cardInitError = false;
    DEBUG.println("DEBUG: SDCard is ready");
  }
}

bool checkForSettings(){
  if (SD.exists("settings.txt")) {
    DEBUG.println("DEBUG: settings file found.");
    device.cardSettingsFile = true;
  } else {
    DEBUG.println("DEBUG: settings file not found.");
    device.cardSettingsFile = false;
  }
  return device.cardSettingsFile;
}

void startReadDeviceSettings(){
  //Open the settings file
  bufferSD.fileReader = SD.open("settings.txt" , FILE_READ);
}
bool readDeviceSettings(){
  return checkPort(bufferSD); //reads a line and returns true if more are available or false if end of file reached
}
void endReadDeviceSettings(){
  //Closes the file
  bufferSD.fileReader.close();
}



bool cardInserted(){
  return !digitalRead(SDCD);
}

void cardDetect_isr(){
  if(cardInserted()) device.deviceState = DCU_RESTARTING_SDHC;
  else endSD();
}

void endSD(){
  if(device.dataFileOpen){
    closeLogFile();
  }
  device.cardPresent = false;
  device.cardInitError = false;
  device.dataFileOpen = false;
  device.cardSettingsFile = false;
  if(DEBUGGING)DEBUG.println("Card Removed");
  //SD.end();
}

bool sdUnavailable(){
  if(device.cardInitError == true || device.peripheralPowerOn == false){
    printEr("SD Unavailable");
    return true;
  }
  return false;
}



void printDirectory(Stream &port){
  //print contents of SD card root directory to USB Serial
  if(sdUnavailable()) return;
  port.println("SD Card Root:");
  //SD.ls(LS_R);
}
