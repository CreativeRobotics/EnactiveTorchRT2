//Serial port command handlers
//List of commands
typedef bool (*commandHandler)(serialBuffer_t &); //command handler function pointer type
//Command handler array type - contains command string and function pointer
typedef struct commander_t{
  String commandString;
  commandHandler handler;
  String manualString;
} commander_t;

//An array of commands - each initialised with a command string and handler function
const commander_t commands[] = {
  {"get status",        getDeviceStatus,    "get device status information"},
  {"set time ",         setTime,            "set the time. Syntax: set time [HH]:[MM]:[SS]"},
  {"get time",          getTime,            "get the device time"},
  {"set date ",         setDate,            "Set the date Syntax: set time [DD]:[MM]:[YYYY]"},
  {"get date",          getDate,            "Get the device date"},
  {"get raw voltage",   getRawV,            "Get the device raw voltage reading"},
  {"get settings",      getSettings,        "Print out the device settings"},
  {"set precision",     setDataPrecision,   "Sets the number of decimal places for data logging. Example: set precision 5"},
  {"set aux power ",    set5V,              "Turn the auxiliary 5V power regulator on or off. Syntax: set aux power on"},
  {"sleep",             setSleep,           "Puts the device to sleep"},
  {"restart",           restartDCU,         "restarts the DCU"},
  {"set SSID",          setSSID,            "Sets the SSID for WiFI. Syntax: set SSID [WIFI NAME]"}, //setSSID0=
  {"set Pass",          setPassword,        "Sets the password for WiFI. Syntax: set Pass [WIFI PASS]"}, //setWiFiPass0=
  {"enable server",     enableServer,       "Starts the HTTP server"},
  {"disable server",    disableServer,      "Stope the HTTP server"},
  {"enable telnet",     enableTelnet,       "Starts the Telnet service"},
  {"disable telnet",    disableTelnet,      "Stops the Telnet service"},
  {"enable bluetooth",  enableBluetooth,    "Starts the Bluetooth service"},
  {"disable bluetooth", disableBluetooth,   "Stops the Bluetooth service"},
  {"enable UDP",        enableUDP,          "Starts the UDP service"},
  {"disable UDP",       disableUDP,         "Starts the UDP service"},
  {"startlog",          startLogging,       "Starts logging data"}, //start Logging Data
  {"stoplog",           stopLogging,        "Stops logging data"}, //stop Logging Data
  {"setlog ",           setloglist,         "Configure which types of data are to be logged and where to send it (SD, USB, UDP, Bluetooth). Examples: setlog quaternion true  setlog USB true"}, //configure which data is to be logged
  {"get log header",    getLogHeader,       "Gets the header for the log file with names for each column of data"}, //return the log header
  {"dir",               printSDDirectory,   "Print the filenames of files on the SD Card"}, //print the SD directory
  {"start haptest",     startHapticTest,    "Start testing the haptics"}, //start Logging Data
  {"stop haptest",      stopHapticTest,     "Stop testing the haptics"}, //stop Logging Data
  {"set waveform ",     setWaveform,        "Set the waveform type. Syntax:set waveform [channel] [TYPE]"}, //set the analog waveform type syntax = set waveform [channel] [TYPE]
  {"set gyro bias ",    setBias,            "Set the Gyro bias values with three comma seperated integers. Syntax:set gyro bias [x],[y],[z]"},
  {"set accel bias ",   setBias,            "Set the Accelerometer bias values with three comma seperated integers. Syntax:set gyro bias [x],[y],[z]"},
  {"set mag bias ",     setBias,            "Set the magnetometer bias values with three comma seperated integers. Syntax:set gyro bias [x],[y],[z]"},
  {"ESP Boot",          espBootloaderMode,  "Set the ESP32 to bootloader mode"}, //go into bootloader and disable UART
  {"ESP RESET",         espReset,           "Reset the ESP32"}, //restart ESP32 and UART
  {"ESP Status:",       espStatusMessage,   "An ESP32 WiFi module Status message"}, //Status message from ESP
  {"ESPGet:",           espSendClientData,  "Marks HTML data sent to the WiFI module for it to sent to a client"},  //get an HTTP page
  {"ESPTelnet:",        espSendTelnetData,  "Marks data as coming from a Telnet client"},  //
  {"ESP:",              espSendCommand,     "Route the command to the ESP32 WiFi Module"}, //route a command to the ESP module  
  {"testSD",            testSD,             "Test the SD by opening, writing and closing a file"},
  {"button action ",    buttonActionSet,    "Enable or disable various button actions"},
  {"DEBUG:",            debugMessage,       "Debug message"},
  {"ack",               ack,                "Acknowledge"},
  {"nack",              nack,               "NOT Acknowledge"},
  {"toggle print",      togglePrint,        "Toggle Printing (DEBUGGING)"},
  {"?",                 query,              "Query"},
  {"help",              help,               "Get Help"},
};

const uint16_t numOfCmds = sizeof(commands) /  sizeof(commands[0]); //calculate the number of commands so we know the array bounds
/*
bool commandFunctionTemplate( serialBuffer_t &sBuff){

  return 0;
}
*/

//
int getCommandEndIndex(serialBuffer_t &sBuff){
  //find the end of the characters in the command string - index AFTER the last character in the command string
  return commands[sBuff.commandVal].commandString.length();
}

bool getTrueOrFalse(String dataString){
  if(dataString.indexOf("true") > -1) return true;
  return false;
}
//--------------------------------------------------------------------------------------------------------
bool buttonActionSet( serialBuffer_t &sBuff){
  //button action 
  /* Sub Commands:
   *  enable haptics
   */
  int startOf = sBuff.bufferString.indexOf("action");
  startOf = sBuff.bufferString.indexOf(" ", startOf); //find the space after the "button action " command
  String subStr = sBuff.bufferString.substring(startOf +1);
  bool containsTrue = getTrueOrFalse(subStr);
  //parse substr
  if(subStr.indexOf("enable haptics") != -1){
    device.buttonEnableHaptics = containsTrue;
  }else if(subStr.indexOf("enable log") != -1){
    device.buttonStartLog = containsTrue;
  }else if(subStr.indexOf("enable toggle") != -1){
    device.buttonActionToggle = containsTrue;
  }
  return 0;
}

bool setBias( serialBuffer_t &sBuff){
  //extract three comma seperated values
  long biases[3];
  int idx = sBuff.bufferString.lastIndexOf(",");
  String tmp = sBuff.bufferString.substring(idx+1);
  biases[0] = tmp.toInt();

  idx = sBuff.bufferString.lastIndexOf(",", idx-1);
  tmp = sBuff.bufferString.substring(idx+1);
  biases[1] = tmp.toInt();
  
  idx = sBuff.bufferString.lastIndexOf(" ", idx-1);
  tmp = sBuff.bufferString.substring(idx+1);
  biases[2] = tmp.toInt();
  uint8_t biasType = 0;
  //Find out which group of biases are being set
  if(sBuff.bufferString.indexOf("gyro") > -1) biasType = 1;
  else if(sBuff.bufferString.indexOf("accel") > -1) biasType = 2;
  else if(sBuff.bufferString.indexOf("mag") > -1) biasType = 3;
  for(int n = 0; n < 3; n++) {
    if(biasType == 1)      settings.gyroBias[n] = biases[n];
    else if(biasType == 2) settings.accelBias[n] = biases[n];
    else if(biasType == 3) settings.magBias[n] = biases[n];
  }
  return 0;
}


bool setWaveform( serialBuffer_t &sBuff){
  String wavType = "";
  if(sBuff.bufferString.indexOf("SINE")) wavType = "SINE";
  if(sBuff.bufferString.indexOf("SQUARE")) wavType = "SQUARE";
  if(sBuff.bufferString.indexOf("TRIANGLE")) wavType = "TRIANGLE";
  if(sBuff.bufferString.indexOf("SAWF")) wavType = "SAWF";
  if(sBuff.bufferString.indexOf("SAWB")) wavType = "SAWB";
  if(sBuff.bufferString.indexOf("ONESHOT")) wavType = "ONESHOT";
  if(sBuff.bufferString.indexOf("HALFSHOT")) wavType = "HALFSHOT";
  if(sBuff.bufferString.indexOf("set waveform 0") > -1){
    //chanel 0
    setWaveType(0, wavType);
  }if(sBuff.bufferString.indexOf("set waveform 1") > -1){
    //chanel 1
    setWaveType(1, wavType);
  }
  return 0;
}


bool testSD( serialBuffer_t &sBuff){
  SDFileTest();
  return 0;
}

bool getSettings( serialBuffer_t &sBuff){
  printDeviceSettings(sBuff);
  return 0;
}


bool printSDDirectory( serialBuffer_t &sBuff){

  SD.ls(sBuff.port, LS_R);
  //sBuff.port->println
  return 0;
}
bool getLogHeader( serialBuffer_t &sBuff){
  sBuff.port->println(getLogHeader());
  sBuff.port->println(getLogColumns());
  return 0;
}
bool setloglist( serialBuffer_t &sBuff){
  //detect boolean values for different data types
  if(sBuff.bufferString.indexOf("quaternion") > -1){
    device.logQ = getTrueOrFalse(sBuff.bufferString);
  }else if(sBuff.bufferString.indexOf("YPR") > -1){
    device.logYPR = getTrueOrFalse(sBuff.bufferString);
  }else if(sBuff.bufferString.indexOf("gyro") > -1){
    device.logGyro = getTrueOrFalse(sBuff.bufferString);
  }else if(sBuff.bufferString.indexOf("accel") > -1){
    device.logAccel = getTrueOrFalse(sBuff.bufferString);
  }else if(sBuff.bufferString.indexOf("mag") > -1){
    device.logMag = getTrueOrFalse(sBuff.bufferString);
  }else if(sBuff.bufferString.indexOf("heading") > -1){
    device.logHeading = getTrueOrFalse(sBuff.bufferString);
  }else if(sBuff.bufferString.indexOf("inputs") > -1){
    device.logInputs = getTrueOrFalse(sBuff.bufferString);
  }else if(sBuff.bufferString.indexOf("outputs") > -1){
    device.logOutputs = getTrueOrFalse(sBuff.bufferString);
  }
  //these select which outputs to log data to
  else if(sBuff.bufferString.indexOf("SD") > -1){
    device.logSD = getTrueOrFalse(sBuff.bufferString);
  }else if(sBuff.bufferString.indexOf("UDP") > -1){
    device.logUDP = getTrueOrFalse(sBuff.bufferString);
  }else if(sBuff.bufferString.indexOf("bluetooth") > -1){
    device.logBluetooth = getTrueOrFalse(sBuff.bufferString);
  }else if(sBuff.bufferString.indexOf("USB") > -1){
    device.logUSB = getTrueOrFalse(sBuff.bufferString);
  }
  return 0;
}
bool startHapticTest( serialBuffer_t &sBuff){
  testHapticsStart();
  return 0;
}

bool stopHapticTest( serialBuffer_t &sBuff){
  testHapticsStop();
  return 0;
}
bool startLogging( serialBuffer_t &sBuff){
  startLog();
  return 0;
}
bool stopLogging( serialBuffer_t &sBuff){
  stopLog();
  return 0;
}
bool setDataPrecision( serialBuffer_t &sBuff){
  uint8_t startOf = 0;
  uint8_t endOf = 0;
  int result = 0;
  //set command - find the second space char
  startOf = sBuff.bufferString.indexOf("precision");
  startOf = sBuff.bufferString.indexOf(' ', startOf);
  String subStr = sBuff.bufferString.substring(startOf+1, sBuff.bufferString.length());
  result = subStr.toInt();
  if(result > 0 && result < 8)  settings.dataPrecision = result;
}
bool setSSID(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Setting SSID");
  uint8_t startOf = 0;
  uint8_t endOf = 0;
  //set command - find the second space char
  startOf = sBuff.bufferString.indexOf("SSID");
  startOf = sBuff.bufferString.indexOf(' ', startOf);
  endOf = sBuff.bufferString.indexOf('\n');
  //SSID is after the space
  settings.ssid = sBuff.bufferString.substring(startOf+1,endOf);
  settings.wifiSSIDOK = true;
  if(DEBUGGING){
    debugStartLine();
    DEBUG.print("Set SSID to: ");
    DEBUG.println(settings.ssid);
  }
  return 0;
}

bool setPassword(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Setting Password");
  uint8_t startOf = 0;
  uint8_t endOf = 0;
  //set command - find the second space char
  startOf = sBuff.bufferString.indexOf("Pass");
  startOf = sBuff.bufferString.indexOf(' ', startOf);
  endOf = sBuff.bufferString.indexOf('\n');
  //SSID is after the space
  settings.pswd = sBuff.bufferString.substring(startOf+1,endOf);
  settings.wifiPassOK = true;
  if(DEBUGGING){
    debugStartLine();
    DEBUG.print("Set Password to: ");
    DEBUG.println(settings.pswd);
  }
  return 0;
}

bool enableServer(serialBuffer_t &sBuff){
  WIRELESS.println("enable server");
  if(DEBUGGING) DEBUG.print("DEBUG: Sent Server start command");
  settings.waitForReplyTimer = 1000;
  return 0;
}
bool disableServer(serialBuffer_t &sBuff){
  WIRELESS.println("disable server");
  if(DEBUGGING) DEBUG.print("DEBUG: Sent Server stop command");
  
  settings.waitForReplyTimer = 1000;
  return 0;
}

bool enableTelnet(serialBuffer_t &sBuff){
  WIRELESS.println("enable telnet");
  if(DEBUGGING) DEBUG.print("DEBUG: Sent telnet start command");
  settings.waitForReplyTimer = 1000;
  return 0;
}
bool disableTelnet(serialBuffer_t &sBuff){
  WIRELESS.println("disable telnet");
  if(DEBUGGING) DEBUG.print("DEBUG: Sent telnet stop command");
  settings.waitForReplyTimer = 1000;
  return 0;
}


bool enableBluetooth(serialBuffer_t &sBuff){
  WIRELESS.println("enable bluetooth");
  if(DEBUGGING) DEBUG.print("DEBUG: Sent bluetooth start command");
  settings.waitForReplyTimer = 1000;
  return 0;
}
bool disableBluetooth(serialBuffer_t &sBuff){
  WIRELESS.println("disable bluetooth");
  if(DEBUGGING) DEBUG.print("DEBUG: Sent bluetooth stop command");
  settings.waitForReplyTimer = 1000;
  return 0;
}

bool enableUDP(serialBuffer_t &sBuff){
  WIRELESS.println("enable UDP");
  if(DEBUGGING) DEBUG.print("DEBUG: Sent UDP start command");
  settings.waitForReplyTimer = 1000;
  return 0;
}
bool disableUDP(serialBuffer_t &sBuff){
  WIRELESS.println("disable UDP");
  if(DEBUGGING) DEBUG.print("DEBUG: Sent UDP stop command");
  settings.waitForReplyTimer = 1000;
  return 0;
}

bool setTime(serialBuffer_t &sBuff){
  //DEBUG.println("Setting the RTC time . . .");
  int res = 0;
  String subStr = "";
  //Command "set time " so find the second space char
  int startOf = 0;
  startOf = sBuff.bufferString.indexOf(' ');
  startOf = sBuff.bufferString.indexOf(' ', startOf+1);
  //Check its not an end of line
  if(sBuff.bufferString.length() == startOf) return 1;//error - nothing after the equals
  subStr = sBuff.bufferString.substring(startOf+1, sBuff.bufferString.length());
  res = subStr.toInt();
  //sBuff.port->print("Result=");
  //sBuff.port->println(res);
  if(res > -1) Clock.setHours((uint8_t)res); 
  
  startOf = sBuff.bufferString.indexOf(':', startOf+1);
  //Check its not an end of line
  if(sBuff.bufferString.length() <= startOf) return 1;//error - nothing after the equals
  subStr = sBuff.bufferString.substring(startOf+1, sBuff.bufferString.length());
  res = subStr.toInt();
  //sBuff.port->print("Result=");
  //sBuff.port->println(res);
  if(res > -1) Clock.setMinutes((uint8_t)res); 

  startOf = sBuff.bufferString.indexOf(':', startOf+1);
  //Check its not an end of line
  if(sBuff.bufferString.length() <= startOf) return 1;//error - nothing after the equals
  subStr = sBuff.bufferString.substring(startOf+1, sBuff.bufferString.length());
  res = subStr.toInt();
  //sBuff.port->print("Result=");
  //sBuff.port->println(res);
  if(res > -1) Clock.setSeconds((uint8_t)res);
  //reset the millisecond timer
  device.clockLastMillis = millis();
  delay(10); //for some reason the readback is off - perhaps he RTC takes too long to sync and the old seconds are printed
  //Clock.setTime(hrs, mins, secs);
  getTime(sBuff);
  //sBuff.port->print("Time: ");
  //sBuff.port->println(getTimestamp());
  //myString.indexOf(val);
  return 0;
}

bool getTime(serialBuffer_t &sBuff){
  sBuff.port->print("Time: ");
  sBuff.port->println(getTimestamp());
}


bool setDate(serialBuffer_t &sBuff){
  //DEBUG.println("Setting the RTC date . . .");
  int res = 0;
  String subStr = "";
  //Command "set date " so find the second space char
  int startOf = 0;
  startOf = sBuff.bufferString.indexOf(' ');
  startOf = sBuff.bufferString.indexOf(' ', startOf+1);
  //Check its not an end of line
  if(sBuff.bufferString.length() == startOf) return 1;//error - nothing after the colon
  subStr = sBuff.bufferString.substring(startOf+1, sBuff.bufferString.length());
  res = subStr.toInt();
  //sBuff.port->print("Result=");
  //sBuff.port->println(res);
  if(res > -1) Clock.setDay((uint8_t)res); 
  
  startOf = sBuff.bufferString.indexOf(':', startOf+1);
  //Check its not an end of line
  if(sBuff.bufferString.length() <= startOf) return 1;//error - nothing after the colon
  subStr = sBuff.bufferString.substring(startOf+1, sBuff.bufferString.length());
  res = subStr.toInt();
  //sBuff.port->print("Result=");
  //sBuff.port->println(res);
  if(res > -1) Clock.setMonth((uint8_t)res); 

  startOf = sBuff.bufferString.indexOf(':', startOf+1);
  //Check its not an end of line
  if(sBuff.bufferString.length() <= startOf) return 1;//error - nothing after the colon
  subStr = sBuff.bufferString.substring(startOf+1, sBuff.bufferString.length());
  res = subStr.toInt();
  //sBuff.port->print("Result=");
  //sBuff.port->println(res);
  if(res > -1) Clock.setYear((uint8_t)res);
  delay(10); //for some reason the readback is off - perhaps he RTC takes too long to sync and the old year is printed
  //Clock.setTime(hrs, mins, secs);
  getDate(sBuff);
  //sBuff.port->print("Time: ");
  //sBuff.port->println(getTimestamp());
  //myString.indexOf(val);
  return 0;
}

bool getDate(serialBuffer_t &sBuff){
  sBuff.port->print("Date: ");
  sBuff.port->println(getDate());
  return 0;
}

bool getRawV(serialBuffer_t &sBuff){
  sBuff.port->print("Raw Voltage: ");
  sBuff.port->println(analogRead(BATTERYMON));
  return 0;
}


bool set5V(serialBuffer_t &sBuff){
  sBuff.bufferString.toLowerCase();
  if(sBuff.bufferString.indexOf("on") > -1){
    enable5VPower();
    //sBuff.port->println("5V ON");
  }
  else if(sBuff.bufferString.indexOf("off") > -1){
    disable5VPower();
    //sBuff.port->println("5V OFF");
  }
  return 0;
}

bool setSleep(serialBuffer_t &sBuff){
  //shut down peripherals
  sleepDevice();
  
  return 0;
}

bool restartDCU(serialBuffer_t &sBuff){
  //shut down peripherals
  restartDevice();
  
  return 0;
}
bool getDeviceStatus(serialBuffer_t &sBuff){
  printDeviceState(sBuff);
  return 0;
}

bool debugMessage(serialBuffer_t &sBuff){
  if(settings.printDebugMessages){
    sBuff.port->println(sBuff.bufferString);
  }
  return 0;
}

bool ack(serialBuffer_t &sBuff){
  DEBUG.println("ACK received");
  return 0;
}
bool nack(serialBuffer_t &sBuff){
  DEBUG.println("NACK received");
  return 0;
}


bool espReset(serialBuffer_t &sBuff){
  /*
   * Reset the ESP32
   */
   sBuff.port->println("DCU Status: Resetting ESP32");
   startESPApplication();
   
   //disableESP();
   //disableESPBootloader();
   //delay(1000); //wait a sec ...?
   //sBuff.port->println("Done, starting ESP32");
   //sBuff.port->flush();
   //delay(100);
   //settings.ESPBootMode = false;
   //enableESP();
   return 0;
}
bool espBootloaderMode(serialBuffer_t &sBuff){
  /*
   * Setup the ESP for bootloader mode
   * Set bootloader pin state then disable and re-enable
   * Set the USB Serial port to pass data straight to the ESP
   * Set the ESP Serial to pass data straight back
   */
   sBuff.port->println("DCU Status: Configuring ESP32 for booloader mode");
   startESPBootloader();
   return 0;
}
bool query(serialBuffer_t &sBuff){
  sBuff.port->print("DCU Status: ETRT DCU Firmware Version ");
  sBuff.port->println(settings.DCU_FW_VERSION);
  printCommandList(sBuff);
  printDeviceState(sBuff);
  return 0;
}

bool help(serialBuffer_t &sBuff){
  //ERRORS.println("Command List:");
  //sBuff.port->println("Command List:");
  printCommandList(sBuff);
  return 0;
}

bool togglePrint(serialBuffer_t &sBuff){
  //ERRORS.println("Serial handler 2");
  sBuff.port->println("DCU Status: Toggled Printing");
  printData = !printData;
  return 0;
}


bool espStatusMessage(serialBuffer_t &sBuff){
  //look through the string to see what if any status message came in
  /*if(DEBUGGING){
    DEBUG.print("DEBUG: ESP Status Message is: ");
    DEBUG.println(sBuff.bufferString);
  }*/
  if(sBuff.bufferString.indexOf("ONLINE") > -1)               device.espOnline = true;
  else if(sBuff.bufferString.indexOf("UDP Enabled") > -1)     device.espUDPEnabled = true;
  else if(sBuff.bufferString.indexOf("UDP Disabled") > -1)     device.espUDPEnabled = false;
  else if(sBuff.bufferString.indexOf("Server Enabled") > -1)  device.espServerEnabled = true;
  else if(sBuff.bufferString.indexOf("Server Disabled") > -1)  device.espServerEnabled = false;
  else if(sBuff.bufferString.indexOf("Telnet Enabled") > -1)  device.espTelnetEnabled = true;
  else if(sBuff.bufferString.indexOf("Telnet Disabled") > -1)  device.espTelnetEnabled = false;
  else if(sBuff.bufferString.indexOf("Bluetooth Enabled") > -1)      device.espBTEnabled = true;
  else if(sBuff.bufferString.indexOf("Bluetooth Disabled") > -1)      device.espBTEnabled = false;
  else if(sBuff.bufferString.indexOf("BlueTooth=") > -1){
    device.BTName = sBuff.bufferString.substring(sBuff.bufferString.indexOf("=")+1, sBuff.bufferString.indexOf("\n"));
  }
  else if(sBuff.bufferString.indexOf("WiFiIP") > -1){
    device.espWiFiConnected = true;
    uint8_t startOf = sBuff.bufferString.indexOf(' ');
    device.localIPAddress = sBuff.bufferString.substring(startOf);
  }
  else if(sBuff.bufferString.indexOf("WiFiConnErr") > -1){
    device.espWiFiConnected = false;
    device.espWiFiConnectionErrors++;
  }
  DEBUG.println(sBuff.bufferString);
  return 0;
}
bool espSendClientData(serialBuffer_t &sBuff){
  //Check the string to see what the client asked for then stream data back
  //sBuff.port->print("serverSend "); //send the command
  //Is it a command
  if(sBuff.bufferString.indexOf("Command") > -1){
    processServerCommandRequest(sBuff);
  }
  return 0;
}

bool espSendTelnetData(serialBuffer_t &sBuff){
  //Check the string to see what the client asked for then stream data back
  //sBuff.port->print("serverSend "); //send the command
  //Is it a command
  //if(sBuff.bufferString.indexOf("Command") > -1){
    //processServerCommandRequest(sBuff);
  //}
  //reply with telnetSend:
  sBuff.port->println("telnetSend:OK");
  //return 0;
}

bool espSendCommand(serialBuffer_t &sBuff){
  //Route a command to the ESP from source (USB or SD Card or even bluetooth?
  //Strip out the "ESP:" prefix and send to ESP
  int startIndex = sBuff.bufferString.indexOf(':');
  //String espCmd = sBuff.bufferString.substring(startIndex);
  WIRELESS.print(sBuff.bufferString.substring(startIndex+1)); //the substring will include the newline
  if(DEBUGGING){
    debugStartLine();
    DEBUG.print("Sent ESP: ");
    DEBUG.println(sBuff.bufferString.substring(startIndex+1));
  }
  return 0;
}
bool unknownCommand(serialBuffer_t &sBuff){
  //ERRORS.println("Unknown Serial Command");
  sBuff.port->println("Unknown Serial Command");
  return 0;
}
bool commentCommand(serialBuffer_t &sBuff){
  //ERRORS.println("Unknown Serial Command");
  sBuff.port->println("Command Comment");
  return 0;
}
bool printCommandList(serialBuffer_t &sBuff){
  //Prints all the commands
  uint8_t n = 0;
  //int length1 = 0;
  sBuff.port->println("#COMMAND LIST");
  for(n = 0; n < numOfCmds; n++){
    sBuff.port->print(commands[n].commandString);
    //length1 = commands[n].commandString.length();
    for(int i = 0; i < (32-commands[n].commandString.length()); i++){
      //add whitespace
      sBuff.port->print(" ");
    }
    sBuff.port->print("|");
    sBuff.port->println(commands[n].manualString);
  }
  sBuff.port->println("#END OF COMMANDS");
  return 0;
}
