
/*
void setupWiFi(){
  //Add any wifi settings
  for(int n = 0; n < WIFI_LIST; n++){
    if(wifiLogins[n].ssid.length() > 1 && wifiLogins[n].pswd.length() > 1){
      //only add if the ssid and password exist in the array
      addAccessPoint(wifiLogins[n].ssid, wifiLogins[n].pswd);
      DEBUG.print("Added AP for ");
      DEBUG.print(wifiLogins[n].ssid);
      DEBUG.print(" : ");
      DEBUG.println(wifiLogins[n].pswd);
    }
  }
}

void addAccessPoint(String mySsid, String myPassword){
  char ss[mySsid.length()+1];
  char ps[myPassword.length()+1];
  mySsid.toCharArray(ss, mySsid.length()+1);
  myPassword.toCharArray(ps, myPassword.length()+1);
  //wifiMulti.addAP(ss, ps);
}*/

void initTime(String timeServer){
  
  char ntpServer[timeServer.length()+1];
  timeServer.toCharArray(ntpServer, timeServer.length()+1);
  configTime(device.gmtOffset_sec, device.daylightOffset_sec, ntpServer);
}
bool attemptWiFi(){
  return connectToWiFi(wifiLogins[device.wifiListIndex].ssid, wifiLogins[device.wifiListIndex].pswd);
}
bool attemptWiFi(String mySsid, String myPassword){
  return connectToWiFi(mySsid, myPassword);
}
bool connectWiFi(){
  return connectToWiFi(wifiLogins[WIFI_LIST-1].ssid, wifiLogins[WIFI_LIST-1].pswd);
}

void disconnectWiFi(){
  WiFi.mode(WIFI_OFF);
}
bool connectToWiFi(String mySsid, String myPassword){
  unsigned long counter = 0;
  if(DEBUGGING) debugPrintln("Connecting Wifi...");

  char ss[mySsid.length()+1];
  char ps[myPassword.length()+1];
  mySsid.toCharArray(ss, mySsid.length()+1);
  myPassword.toCharArray(ps, myPassword.length()+1);
  if(DEBUGGING){ DEBUG.print('['); DEBUG.print(ss); DEBUG.println(']');}
  if(DEBUGGING){ DEBUG.print('['); DEBUG.print(ps); DEBUG.println(']');}
  WiFi.begin(ss, ps);
  bool trying = true;
  while (trying) {
      delay(1);
      //if(DEBUGGING) Serial.print(".");
      counter++;
      if(WiFi.status() == WL_CONNECTED){
        if(DEBUGGING) DEBUG.println("Connected");
        return true;
      }
      if(counter > device.wifiConnectTimeout) {
        trying = false;
        if(DEBUGGING) DEBUG.println();
        if(DEBUGGING) DEBUG.println("Unable to connect to anything");
      }
  }
  return false;
}

void sendTestPage(){
  sendToClient(header);
  sendToClient(pageMenu);
  sendToClient(testLine1);
  sendToClient(testLine2);
  sendToClient(testLine3);
  sendToClient(pageEnd);
}


void startClientReply(){
  if(device.clientConnected){
    serverClient.println("HTTP/1.1 200 OK");
    serverClient.println("Content-type:text/html");
    serverClient.println();
  }
}
void sendToClient(String cLine){
  if(device.clientConnected) serverClient.print(cLine);
  //if(DEBUGGING) debugPrintln(cLine);
}

void sendToTelnet(String cLine){
  if(device.telnetConnected) telnetClient.println(cLine);
  //if(DEBUGGING) debugPrintln(cLine);
}

void endClient(){
  if(device.clientConnected){
    serverClient.println();
    serverClient.println();
    serverClient.stop();
    device.clientConnected = false;
    device.serverSending = false;
    device.clientDisconnectTime = 0;
  }
  if(DEBUGGING) debugPrintln("Client Disconnected.");
}
void enableServer(){
  device.serverEnabled = true;
  device.serverSending = false;
  server.begin();
}
void disableServer(){
  device.serverEnabled = false;
  device.serverSending = false;
  server.end();
}
void enableTelnet(){
  device.telnetEnabled = true;
  device.telnetSending = false;
  telnetServer.begin();
  
  telnetServer.setNoDelay(true);
}
void disableTelnet(){
  device.telnetEnabled = false;
  device.telnetSending = false;
  telnetServer.end();
}

void checkTelnet(){
  if(device.telnetEnabled == false) return; //do nothing if not enabled
  if(!telnetClient.connected()){
    telnetClient = telnetServer.available();   // listen for incoming clients
    if (telnetClient) {                             // if you get a client,
      device.telnetConnected = true;
      //if(PRINT_CLIENT_DATA) debugPrintln("New telnet Client.");           // print a message out the serial port
      Serial.println("ESP Status: New telnet Client");
      telnetClient.println("Enactive Torch 2.0 Online");
    }else{
      if(device.telnetConnected == true)Serial.println("ESP Status: Telnet Client Disconnected"); //print once if you were connected
      device.telnetConnected = false;
    }
  }
  if(device.telnetConnected == true){
   //currently connected - check for data
   String currentLine = "ESPTelnet:"; // make a String to hold incoming data from the client - use Telnet: command to mark it as a telnet packet
   char inByte = 0;
   while(telnetClient.available()) {
      inByte = telnetClient.read();
      currentLine += inByte;
      //When you encounter a newline, send the data to the Serial port
      if(inByte == '\n'){
        Serial.print(currentLine);
        currentLine = "ESPTelnet:";
      }
    }
  }
}


void checkWiFi(){
  if(device.serverEnabled == false) return; //do nothing if not enabled
  if(device.serverSending == true) return; //already received , now replying
  serverClient = server.available();   // listen for incoming clients
  if (serverClient) {                             // if you get a client,
    device.clientConnected = true;

    if(PRINT_CLIENT_DATA) debugPrintln("New Client.");           // print a message out the serial port

    String currentLine = "";                // make a String to hold incoming data from the client
    char lastChar = 0;
    char c;

    if(PRINT_CLIENT_DATA)  debugStartLine();

    //bool readingClient = true;
    while (serverClient.connected()) {            // loop while the client's connected
      if (serverClient.available()) {            // if there's bytes to read from the client,
        if(c != '\r') lastChar = c;
        c = serverClient.read();             // read a byte, then

        if(PRINT_CLIENT_DATA){
          debugWrite(c);//Serial.write(c);                    // print it out the serial monitor
          if(c == '\n') debugStartLine();
        }

        if(c == '\r' && currentLine.startsWith("GET")) {
          //Serial.println("Found Form Data");
          webFormSubmit = currentLine;
          currentLine = "";
        }
        if(c != '\n' && c != '\r') currentLine += c;
        if(c == '\n' && lastChar == '\n') break; //End of page
      }
    }

    if(PRINT_CLIENT_DATA)      debugEndLine();

    startClientReply();
    //Ask the main unit for a web page
    Serial.print("ESPGet:");
    Serial.println(webFormSubmit);
    //TEST
    //sendTestPage();
    //endClient();
    device.serverSending = true;
    device.clientDisconnectTime = millis() + device.clientConnectionTimeout;
  }
}
/*
//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          //initializes the UDP state
          //This initializes the transfer buffer
          udp.begin(WiFi.localIP(),udpPort);
          Serial.print("UDP Started on port:");
          Serial.println(udpPort); 
          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
    }
}*/
