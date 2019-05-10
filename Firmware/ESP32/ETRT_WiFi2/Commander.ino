//connect to VM4789902 vrtqvfDxr8hq
//connect to AndroidAP hvpm2495

//bool query(serialBuffer_t &sBuff);
//bool help(serialBuffer_t &sBuff);

const commander_t commands[] = {
  {"?", query},
  {"help", help},
  {"ack", ack},
  {"nack", nack},
  {"set debug", setDebugState},
  {"get status", printStatus},
  {"get network time", getTime},
  {"set time server ", setTimeServer},
  {"set gmt offset ", setGMTOffset},
  {"enable server", startEnableServer},
  {"disable server", disableServer},
  {"enable telnet", startEnableTelnet},
  {"disable telnet", disableTelnet},
  {"enable bluetooth", startEnablebluetooth},
  {"disable bluetooth", disablebluetooth},
  {"enable UDP", startEnableUDP},
  {"disable UDP", disableUDP},
  {"UDPSend:", udpSendData},
  {"serverSend:", serverSendLine},
  {"telnetSend:", telnetSendLine},
  {"BTSend:", btSendLine},
  {"serverEnd", serverEndLine},
  {"set UDPPort", setUDPPort},
  {"set UDPAddress", setUDPAddress},
  {"set SSID", setSSID}, //setSSID0=
  {"set Pass", setPassword}, //setWiFiPass0=
  {"set Server Timeout", setServerTimeout}, //setWiFiPass0=
  {"set BTName", setBluetoothName}, //setWiFiPass0=
  {"connect to", wifiConnectTo},
  {"set WiFi Timeout", setWiFiTimeout}, //setWiFiPass0=
  {"disconnect", wifiDisconnect}
};
const uint16_t numOfCmds = sizeof(commands) /  sizeof(commands[0]); //calculate the number of commands so we know the array bounds

//Serial port command handlers ======================================================================================================
//startEnableTelnet
//disableTelnet
//telnetSendLine

bool setDebugState(serialBuffer_t &sBuff){
  //DEBUG.println("Enabled Server");
  if(sBuff.bufferString.indexOf("on") > -1){
    DEBUGGING = true;
    sBuff.port->println("ESP Status: Debug messages on");
  }else{
    DEBUGGING = false;
    sBuff.port->println("ESP Status: Debug messages off");
  }
  return 0;
}
//======================================================================================================
bool telnetSendLine(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Sending to telnet client");
  uint8_t startOf = 0, endOf = 0;
  //find a space
  startOf = sBuff.bufferString.indexOf(':');
  endOf = sBuff.bufferString.length();
  if( device.telnetEnabled ){
    sendToTelnet(sBuff.bufferString.substring(startOf+1, endOf+1)); //strip any newlines
  }
  if(DEBUGGING) debugPrintln(sBuff.bufferString.substring(startOf+1));
  return 0;
}

//======================================================================================================
bool startEnableTelnet(serialBuffer_t &sBuff){
  //DEBUG.println("Enabled Server");
  //sBuff.port->println("ESP: Server Enabled");
  device.startTelnet = true;
  return 0;
}

bool disableTelnet(serialBuffer_t &sBuff){
  //DEBUG.println("Enabled Server");
  sBuff.port->println("ESP Status: Telnet Disabled");
  device.startTelnet = false;
  disableTelnet();
  return 0;
}



bool printStatus(serialBuffer_t &sBuff){
  //print out the device state
    if(device.wifiConnected) {
    Serial.println("ESP Status: WiFi Connected");
    delay(10);
    sBuff.port->print("ESP Status: WiFiIP=");
    Serial.println(WiFi.localIP());
  }else sBuff.port->println("ESP Status: WiFi Disconnected");
  delay(10);
  
  if(device.serverEnabled)  sBuff.port->println("ESP Status: Server Enabled");
  else                      sBuff.port->println("ESP Status: Server Disabled");
  delay(10);
  if(device.udpEnabled)     sBuff.port->println("ESP Status: UDP Enabled");
  else                      sBuff.port->println("ESP Status: UDP Disabled");
  delay(10);
  if(device.btEnabled)      sBuff.port->println("ESP Status: BT Enabled");
  else                      sBuff.port->println("ESP Status: BT Disabled");
}


bool setBluetoothName(serialBuffer_t &sBuff){
  uint8_t startOf = 0;
  uint8_t endOf = 0;
  //set command - find the second space char
  startOf = sBuff.bufferString.indexOf(' ');
  startOf = sBuff.bufferString.indexOf(' ', startOf+1);
  endOf = sBuff.bufferString.length();
  String substr = sBuff.bufferString.substring(startOf+1, endOf);
  //ignore if there is only one character
  if(substr.length() > 1)  device.btName = substr;
  return 0;
}

bool setServerTimeout(serialBuffer_t &sBuff){
  uint8_t startOf = 0;
  uint8_t endOf = 0;
  int val = 0;
  //set command - find the final space char
  startOf = sBuff.bufferString.indexOf("Timeout");
  startOf = sBuff.bufferString.indexOf(' ', startOf);
  endOf = sBuff.bufferString.length();
  String substr = sBuff.bufferString.substring(startOf+1, endOf);
  val = substr.toInt();
  //Don't set it to less than 10ms
  if(val < 10) device.clientConnectionTimeout = 10;
  else device.clientConnectionTimeout = val;
  return 0;
}

bool setWiFiTimeout(serialBuffer_t &sBuff){
  uint8_t startOf = 0;
  uint8_t endOf = 0;
  int val = 0;
  //set command - find the final space char
  startOf = sBuff.bufferString.indexOf("Timeout");
  startOf = sBuff.bufferString.indexOf(' ', startOf);
  endOf = sBuff.bufferString.length();
  String substr = sBuff.bufferString.substring(startOf+1, endOf);
  val = substr.toInt();
  //Don't set it to less than 1000 ms
  if(val < 1000) device.wifiConnectTimeout = 1000;
  else device.wifiConnectTimeout = val;
  return 0;
}



bool wifiConnectTo(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Trying to connect to ...");
  uint8_t startOf = 0;
  uint8_t endOf = 0;

  //set command - find the second space char
  startOf = sBuff.bufferString.indexOf(' ');
  startOf = sBuff.bufferString.indexOf(' ', startOf+1);
  endOf = sBuff.bufferString.indexOf(' ', startOf+1);
  ctSSID = sBuff.bufferString.substring(startOf+1,endOf);
  startOf = endOf+1;
  endOf = sBuff.bufferString.length();//sBuff.bufferString.indexOf('\n');
  ctPswd = sBuff.bufferString.substring(startOf, endOf);

  if(DEBUGGING) {debugPrint("SSID=["); DEBUG.print(ctSSID); DEBUG.println(']');}
  if(DEBUGGING) {debugPrint("PSWD=["); DEBUG.print(ctPswd); DEBUG.println(']');}
  wifiLogins[WIFI_LIST-1].ssid = ctSSID;
  wifiLogins[WIFI_LIST-1].pswd = ctPswd;
  device.wifiListIndex = WIFI_LIST-1;
  device.connectToWiFi = true;
  return 0;
}

bool wifiDisconnect(serialBuffer_t &sBuff){
  disconnectWiFi();
  device.wifiConnected = false;
  sBuff.port->println("ESP Status: WiFi Disconnected");
}


bool setSSID(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Setting SSID");
  uint8_t startOf = 0;
  uint8_t endOf = 0;
  int item = -1;
  //set command - find the second space char
  startOf = sBuff.bufferString.indexOf(' ');
  startOf = sBuff.bufferString.indexOf(' ', startOf+1);
  endOf = sBuff.bufferString.length();
  //Password is after the space
  String ssidText = sBuff.bufferString.substring(startOf+1,endOf);
  //The SSID item number will be before the space
  String itm = sBuff.bufferString.substring(startOf-1);
  item = itm.toInt();
  //addAccessPoint(
  if( item > -1 && item < WIFI_LIST){
    wifiLogins[item].ssid = ssidText;
  }
  if(DEBUGGING){
    debugStartLine();
    DEBUG.print("Set SSID to: ");
    DEBUG.println(wifiLogins[item].ssid);
  }
  return 0;
}

bool setPassword(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Setting SSID");
  uint8_t startOf = 0;
  uint8_t endOf = 0;
  int item = -1;
  //set command - find the second space char
  startOf = sBuff.bufferString.indexOf(' ');
  startOf = sBuff.bufferString.indexOf(' ', startOf+1);
  endOf = sBuff.bufferString.length();
  //Password will be after the space
  String pswdText = sBuff.bufferString.substring(startOf+1, endOf);
  //The SSID item number will be before the space
  String itm = sBuff.bufferString.substring(startOf-1);
  item = itm.toInt();
  if( item > -1 && item < WIFI_LIST){
    wifiLogins[item].pswd = pswdText;
  }
  if(DEBUGGING){
    debugStartLine();
    Serial.print("Set Password to: ");
    Serial.println(wifiLogins[item].pswd);
  }
  return 0;
}
bool query(serialBuffer_t &sBuff){
  sBuff.port->println("ESP Status: ESP32 OK");
  //sBuff.port->println(settings.DCU_FW_VERSION);
  //printCommandList(sBuff);
  //printDeviceState(sBuff);
  return 0;
}

bool setTimeServer(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Setting SSID");
  uint8_t startOf = 0;
  uint8_t endOf = 0;
  int item = -1;
  //set command - find the second space char
  startOf = sBuff.bufferString.indexOf("server");
  startOf = sBuff.bufferString.indexOf(' ', startOf);
  endOf = sBuff.bufferString.length();
  //Password will be after the space
  String tsText = sBuff.bufferString.substring(startOf+1, endOf);
  //The SSID item number will be before the space
  device.timeServer = sBuff.bufferString.substring(startOf-1);
  if(DEBUGGING){
    debugStartLine();
    Serial.print("Set Timeserver to: ");
    Serial.println(device.timeServer);
  }
  return 0;
}
bool setGMTOffset(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Setting SSID");
  uint8_t startOf = 0;
  int item = -1;
  //set command - find the second space char
  startOf = sBuff.bufferString.indexOf("offset");
  startOf = sBuff.bufferString.indexOf(' ', startOf);
  //Password will be after the space
  String ofsText = sBuff.bufferString.substring(startOf+1);
  item = ofsText.toInt();
  if( item > -1){
    device.gmtOffset_sec = item;
  }
  if(DEBUGGING){
    debugStartLine();
    Serial.print("Set GMT Offset to: ");
    Serial.println(device.gmtOffset_sec);
  }
  return 0;
}

bool getTime(serialBuffer_t &sBuff){
  //get network time
  if(device.wifiConnected == false){
    sBuff.port->println("ESP Status: Get Time Error: Not Connected");
    return 0;
  }
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    sBuff.port->println("ESP Status: Failed to get time");
    return 0;
  }
  sBuff.port->print("ESP Status: ");
  sBuff.port->println(&timeinfo, "Date %A:%B:%d:%Y Time %H:%M:%S");
  //Returns "Sunday, January 06 2019 04:55:48

  /*
    Worldwide       pool.ntp.org
    Asia            asia.pool.ntp.org
    Europe          europe.pool.ntp.org
    North America   north-america.pool.ntp.org
    Oceania         oceania.pool.ntp.org
    South America   south-america.pool.ntp.org
  */
  return 0;
}
//======================================================================================================
bool help(serialBuffer_t &sBuff){
  printCommandList(sBuff);
  return 0;
}
//======================================================================================================
bool ack(serialBuffer_t &sBuff){
  if(DEBUGGING) DEBUG.println("ACK received");
  return 0;
}
//======================================================================================================
bool nack(serialBuffer_t &sBuff){
  if(DEBUGGING) DEBUG.println("NACK received");
  return 0;
}
//======================================================================================================
bool startEnableServer(serialBuffer_t &sBuff){
  //DEBUG.println("Enabled Server");
  //sBuff.port->println("ESP: Server Enabled");
  device.startServer = true;
  return 0;
}

bool disableServer(serialBuffer_t &sBuff){
  //DEBUG.println("Enabled Server");
  sBuff.port->println("ESP Status: Server Disabled");
  device.startServer = false;
  disableServer();
  return 0;
}
//======================================================================================================
bool startEnablebluetooth(serialBuffer_t &sBuff){
  //DEBUG.println("Enabled BT");
  
  //sBuff.port->println("ESP: BT Enabled");
  device.startBT = true;
  return 0;
}

bool disablebluetooth(serialBuffer_t &sBuff){
  //DEBUG.println("Enabled BT");
  stopBlueTooth();
  sBuff.port->println("ESP Status: BT Disabled");
  device.startBT = false;
  return 0;
}
//======================================================================================================
bool startEnableUDP(serialBuffer_t &sBuff){
  //DEBUG.println("Enabled UDP");
  
  
  device.startUDP = true;
  return 0;
}//======================================================================================================
bool disableUDP(serialBuffer_t &sBuff){
  //DEBUG.println("Enabled UDP");
  stopUDP();
  sBuff.port->println("ESP Status: UDP Disabled");
  device.startUDP = false;
  return 0;
}
//======================================================================================================
bool udpSendData(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Sending to UDP");
  uint8_t startOf = 0;
  //find a space
  startOf = sBuff.bufferString.indexOf(':');
  if( device.udpEnabled ){
    sendUDP(sBuff.bufferString.substring(startOf+1));
  }
  if(DEBUGGING) debugPrintln(sBuff.bufferString.substring(startOf+1));
  return 0;
}
//======================================================================================================
bool serverSendLine(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Sending to client");
  uint8_t startOf = 0, endOf = 0;
  //find a space
  startOf = sBuff.bufferString.indexOf(':');
  endOf = sBuff.bufferString.length();
  if( device.serverEnabled ){
    sendToClient(sBuff.bufferString.substring(startOf+1, endOf)); //strip any newlines
  }
  if(DEBUGGING) debugPrintln(sBuff.bufferString.substring(startOf+1));
  return 0;
}
//======================================================================================================
bool serverEndLine(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Finished sending to client");
  endClient();
  return 0;
}
//======================================================================================================
bool btSendLine(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Sending to Bluetooth");
  uint8_t startOf = 0;
  //find a space
  startOf = sBuff.bufferString.indexOf(':');
  if( device.btEnabled ){
    SerialBlueTooth.println(sBuff.bufferString.substring(startOf+1));
  }
  if(DEBUGGING) debugPrintln(sBuff.bufferString.substring(startOf+1));
  return 0;
}

bool setUDPPort(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Setting UDP Port");
  uint8_t startOf = 0;
  int port = -1;
  //set command - 'set UDPPort 1234' find the second space char
  startOf = sBuff.bufferString.indexOf("UDPPort");
  startOf = sBuff.bufferString.indexOf(' ', startOf);
  String prt = sBuff.bufferString.substring(startOf+1);
  port = prt.toInt();
  if( port > 0 ){
    device.udpPort = port;
  }
  if(DEBUGGING){
    debugStartLine();
    Serial.print("Set UDP Port to: ");
    Serial.println(device.udpPort);
  }
  return 0;
}
bool setUDPAddress(serialBuffer_t &sBuff){
  if(DEBUGGING) debugPrintln("Setting UDP Address");
  uint8_t startOf = 0;
  uint8_t endOf = 0;
  //set command - 'set UDPAddress 123.456.789.0' find the second space char
  startOf = sBuff.bufferString.indexOf("UDPAddress");
  startOf = sBuff.bufferString.indexOf(' ', startOf);
  endOf = sBuff.bufferString.length();
  device.UDPAddress = sBuff.bufferString.substring(startOf+1, endOf);
  if(DEBUGGING){
    debugStartLine();
    Serial.print("Set UDP Address to: ");
    Serial.println(device.UDPAddress);
  }
  return 0;
}
//======================================================================================================
//Standard command handlers
//======================================================================================================
bool unknownCommand(serialBuffer_t &sBuff){
  //ERRORS.println("Unknown Serial Command");
  //sBuff.port->println("nack");
  return 0;
}
//======================================================================================================
bool printCommandList(serialBuffer_t &sBuff){
  //Prints all the commands
  uint8_t n = 0;
   sBuff.port->println("ESP Status: Command List:");
  for(n = 0; n < numOfCmds; n++){
    sBuff.port->println(commands[n].commandString);
  }
  sBuff.port->println("ESP Status: Command List End");
  return 0;
}
//======================================================================================================

//SERIAL PORT PROCESSING --------------------------------------------------

void checkSerial(){
  while(Serial.available()){
    //DEBUG.println("buffering");
    bufferSerial(Serial.read(), sBuffer);
  }
  if(sBuffer.newLine){
    
    //DEBUG.println("Finding command");
    //sBuffer.commandVal = matchCommand(sBuffer);
    sBuffer.commandVal = matchCommand(sBuffer.bufferString);
    //DEBUG.println("Handling command");
    handleCommand(sBuffer);
    //if(!commands[commandIndex].handler(sBuffer)) Serial.println("Command Handled OK");
    //resetBuffer(sBuffer); //no need - resets in handler func
  }
}

void bufferSerial(uint8_t dataByte, serialBuffer_t &sBuff){
  switch(sBuff.parseState){
    case WAITING_FOR_START:
      //DEBUG.println("Waiting for Start");
      if( isCommandStart(dataByte) ) {
        sBuff.parseState = BUFFERING_PACKET;
        writeToBuffer(dataByte, sBuff);
        if(sBuff.bufferFull) resetBuffer(sBuff);
      }
      break;
    case BUFFERING_PACKET:
      //DEBUG.println("Buffering");
      writeToBuffer(dataByte, sBuff);
      if(sBuff.bufferFull) resetBuffer(sBuff);//dump the buffer
      if(sBuff.newLine == true) {
        sBuff.bufferString.trim(); //remove any whitespace
        //Add back the newline
        //sBuff.bufferString += sBuff.END_OF_PACKET;
        sBuff.parseState = PACKET_RECEIVED;
        //DEBUG.println("Newline Detected");
      } //unpack the data
      break;
    case PACKET_RECEIVED:
      //DEBUG.println("Packet received ...?");
      //if you get here then the packet has not been unpacked and more data is coming in. It may be a stray newline ... do nothing
      break;  
  }
}

void writeToBuffer(uint8_t dataByte, serialBuffer_t &sBuff){
  if(sBuff.bytesWritten == SBUFFER-1){
    sBuff.bufferFull = true; //buffer is full
    return;
  }
  //sBuff.buf[sBuff.bytesWritten] = dataByte;
  sBuff.bufferString += (char)dataByte;
  if(dataByte == sBuff.END_OF_PACKET) sBuff.newLine = true;
  sBuff.bytesWritten++;
}
void resetBuffer(serialBuffer_t &sBuff){
  //DEBUG.println("Resetting Buffer");
  //sBuff.readIndex = 0;
  //sBuff.bytesWritten = 0;
  sBuff.bytesWritten = 0;
  sBuff.newData = false;
  sBuff.newLine = false;
  sBuff.bufferFull = false;
  sBuff.parseState = WAITING_FOR_START;
  //sBuff.endOfFile = false;
  sBuff.bufferString = "";
}

uint8_t matchCommand(String cmd){
  //using Strings instead of char arrays
  //loop through the command list and see if it appears in the sBuff String
  for(uint8_t n = 0; n < numOfCmds; n++){
    //if( cmd.indexOf( commands[n].commandString ) > -1) return n;
    if( cmd.startsWith( commands[n].commandString ) ) return n;
  }
  return 255;
}

uint8_t matchCommand(serialBuffer_t &sBuff){
  //using Strings instead of char arrays
  //loop through the command list and see if it appears in the sBuff String
  for(uint8_t n = 0; n < numOfCmds; n++){
    //Serial.print("Checking command: ");
    //Serial.println(commands[n].commandString);
    //Serial.print("Serial Line is: ");
    //Serial.println( sBuff.bufferString );
    if( sBuff.bufferString.indexOf( commands[n].commandString ) > -1) return n;
  }
  return 255;
}


bool handleCommand(serialBuffer_t &sBuff){
  bool returnVal = false;
  if(sBuff.commandVal == 255) {
    //DEBUG.println("handleCommand: Commandval is 255 : Call unknown cmd handler");
    returnVal = unknownCommand(sBuff);
  } //unknown command
  //Call the function in the function pointer array that matches the command
  //Pass the port object so the command handler knows which port to reply to
  else {
    //DEBUG.print("handleCommand: Calling command handler. Index: "); 
    //DEBUG.println(sBuff.commandVal);
    returnVal =  commands[sBuff.commandVal].handler(sBuff);
    //returnVal =  (*commandHandlers[sBuff.commandVal])(sBuff);
  }
  resetBuffer(sBuff);
  return returnVal;
}

bool isCommandStart(char dataByte){
  //The start of the command must be a typeable character
  if( dataByte > 31 &&  dataByte < 127) return true;
  return false;
}
