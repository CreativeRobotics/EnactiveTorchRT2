//Data logging functions
void startLog(){
  if(device.logActive == true) return; //don't start if you already started
  device.logActive = true;
  startSDLog();
}

void stopLog(){
  device.logActive = false;
  if(device.dataFileOpen){
    closeLogFile();
    if(DEBUGGING) DEBUG.println("DEBUG: Stopped Log");
  }
}
void startSDLog(){
  if(device.logSD == false || device.dataFileOpen) return;
  //if(device.dataFileOpen) return; //do nothing if the log has already been started by something else
  if(openLogFile()){
    //device.logActive = true;
    if(DEBUGGING) DEBUG.println("DEBUG: Started SD Log");
    writeSDLogHeader();
    if(DEBUGGING) DEBUG.println("DEBUG: Header Created");
  }else if(DEBUGGING) DEBUG.println("DEBUG: Failed to start SD log");
}

bool openLogFile(){
  if(sdUnavailable()) return false;
  if(DEBUGGING) {
    DEBUG.println("Opening Log File . . .");
  }
  //Open a new datalog file witha time stamped filename
  String filename = getFilenameTime();
  filename += ".csv";
  device.dataLogFilename = filename;

  char fname[device.dataLogFilename.length()+1];
  device.dataLogFilename.toCharArray(fname, device.dataLogFilename.length()+1);
  device.dataLogFile = SD.open(fname , O_WRONLY|O_CREAT);//O_RDWR|O_CREAT
  if(!device.dataLogFile) {
    printErln("Log file failed to open");
    device.dataFileOpen = false;
    return false;
  }
  device.dataFileOpen = true;
  if(DEBUGGING) {
    DEBUG.print("Starting log file: ");
    DEBUG.println(filename);
  }
  return true;
}

void closeLogFile(){
  if(DEBUGGING) {
    DEBUG.println("Closing Log File . . .");
  }
  //if(sdUnavailable()) return false;
  //Open a new datalog file witha time stamped filename
  device.dataLogFile.close();// = SD.close(filename , FILE_WRITE);
  device.dataFileOpen = false;
  device.dataLogFilename = "";
  if(DEBUGGING) {
    DEBUG.print("Stopped log file: ");
  }
}

void writeToLog(String dataLine){
  if(!device.dataFileOpen) {
    if(DEBUGGING)DEBUG.println("DEBUG: Err, file Not Open");
    return;
  }if(dataLine.length() == 0) {
    if(DEBUGGING)DEBUG.println("DEBUG: Err, No data to write");
    return;
  }
  char data[dataLine.length()+1];
  dataLine.toCharArray(data, dataLine.length()+1);
  device.dataLogFile.println(data);
  //device.dataLogFile.flush();
  if(DEBUGGING){
    DEBUG.print("Written to SD:");
    DEBUG.println(data);
    DEBUG.println("Bytes:");
    DEBUG.println(String(dataLine.length()));
  }
}
void writeSDLogHeader(){

  writeToLog( getLogHeader());
  writeToLog( getLogColumns());
}

String getLogHeader(){
  //if(!device.dataFileOpen) return;
  //put in a header for the log file after it is opened
  String header = "";
  header += "ETRT Data Log: ";
  header += getTimeAndDate();
  header += " Filename: ";
  header += device.dataLogFilename;
  //writeToLog(header);
  return header;
}
String getLogColumns(){
  String colums = "";
  colums.reserve(1024);
  //Column headings
  //batteryVoltage, Q0, Q1, Q2, Q3, Y, P, R, Gx, Gy, Gz, Ax, Ay, Az, Mx, My, Mz, CHeading,
  colums += "Timestamp,";
  colums += "Batt,";
  if(device.logQ){
    colums += "Q0,";
    colums += "Q1,";
    colums += "Q2,";
    colums += "Q3,";
  }
  if(device.logYPR){
    colums += "Y,";
    colums += "P,";
    colums += "R,";
  }
  if(device.logGyro){
    colums += "Gx,";
    colums += "Gy,";
    colums += "Gz,";
  }
  if(device.logAccel){
    colums += "Ax,";
    colums += "Ay,";
    colums += "Az,";
  }
  if(device.logMag){
    colums += "Mx,";
    colums += "My,";
    colums += "Mz,";
  }
  if(device.logHeading){
    colums += "Compass Heading,";
  }
  if(device.logInputs){
    for(int n = 0; n < INPUTS; n++){
      colums += "IN";
      colums += String(n);
      colums += ',';
    }
  }
  if(device.logOutputs){
    for(int n = 0; n < OUTPUTS; n++){
      colums += "OUT";
      colums += String(n);
      colums += ',';
    }
  }
  if(colums.endsWith(",") )  colums.remove( colums.lastIndexOf(",") ); //remove any trailing comma
  return colums;
}

void createLogLine(){
    //if(!device.dataFileOpen) return;
  //copy the DCU data into the current log String
  addToBuffer(getTimestamp());
  addCommaToBuffer();
  addToBuffer(String(device.batteryVoltage, settings.dataPrecision));
  if(device.logQ){
    for(int n = 0; n < 4; n++){
      addCommaToBuffer();
      addToBuffer(String(device.quaternion[n], settings.dataPrecision));
    }
  }
  if(device.logYPR){
    for(int n = 0; n < 3; n++){
      addCommaToBuffer();
      addToBuffer(String(device.ypr[n], settings.dataPrecision));
    }
  }
  if(device.logGyro){
    for(int n = 0; n < 3; n++){
      addCommaToBuffer();
      addToBuffer(String(device.gyro[n], settings.dataPrecision));
    }
  }
  if(device.logAccel){
    for(int n = 0; n < 3; n++){
      addCommaToBuffer();
      addToBuffer(String(device.accel[n], settings.dataPrecision));
    }
  }
  if(device.logMag){
    for(int n = 0; n < 3; n++){
      addCommaToBuffer();
      addToBuffer(String(device.mag[n], settings.dataPrecision));
    }
  }
  addCommaToBuffer();
  addToBuffer(String(device.cHeading, settings.dataPrecision));
  //Now the inputs and outputs
  if(device.logInputs){
    for(int n = 0; n < INPUTS; n++){
      addCommaToBuffer();
      addToBuffer(String(device.inputs[n], settings.dataPrecision));
    }
  }
  if(device.logOutputs){
    for(int n = 0; n < OUTPUTS; n++){
      addCommaToBuffer();
      addToBuffer(String(device.outputs[n], settings.dataPrecision));
    }
  }
  
  //device.inputs[4], settings.dataPrecision
  //device.outputs[4], settings.dataPrecision
}

void updateDataLogs(){
  //Create a line of data to log
  createLogLine();
  //send the latest line of data to anything configured to receive it
  if(device.dataFileOpen) updateSDLog();
  if(device.logUDP) sendUDPLog();
  if(device.logBluetooth) sendBTLog();
  if(device.logUSB) sendUSBLog();
  switchBuffers();
}
void updateSDLog(){
  if(DEBUGGING) DEBUG.println("Writing data line to SD");
  writeToLog(logBuffer.buffer[logBuffer.i]);
}
void sendUDPLog(){
  if(!device.espUDPEnabled) return;
  String line = "UDPSend:";
  line += logBuffer.buffer[logBuffer.i];
  WIRELESS.println( line );
}

void sendBTLog(){
  if(!device.espBTEnabled) return;
  String line = "BTSend:";
  line += logBuffer.buffer[logBuffer.i];
  WIRELESS.println( line );
}
void sendUSBLog(){
  if(!USBS) return;
  USBS.println( logBuffer.buffer[logBuffer.i] );
}
void switchBuffers(){
  logBuffer.i = 1-logBuffer.i;
  logBuffer.bytesWritten[logBuffer.i] = 0;
  logBuffer.buffer[logBuffer.i] = "";
}
bool addToBuffer(String newDataString){
  logBuffer.bytesWritten[logBuffer.i] += newDataString.length();
  logBuffer.buffer[logBuffer.i] += newDataString;
  if(logBuffer.bytesWritten[logBuffer.i] > 511) return true;
  return false;
}
bool addCommaToBuffer(){
  logBuffer.bytesWritten[logBuffer.i] ++;
  logBuffer.buffer[logBuffer.i] += ',';
  if(logBuffer.bytesWritten[logBuffer.i] > 511) return true;
  return false;
}
