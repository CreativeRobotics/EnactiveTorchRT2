void printDeviceState(serialBuffer_t &sBuff){
  sBuff.port->println(settings.DCU_FW_VERSION);
  sBuff.port->println(getTimestamp());
  sBuff.port->print("BatVoltage: "); sBuff.port->println(device.batteryVoltage);

  sBuff.port->print("Q:");
  sBuff.port->print(DCU_Q0, settings.dataPrecision);
  sBuff.port->print(',');
  sBuff.port->print(DCU_Q1, settings.dataPrecision);
  sBuff.port->print(',');
  sBuff.port->print(DCU_Q2, settings.dataPrecision);
  sBuff.port->print(',');
  sBuff.port->println(DCU_Q3, settings.dataPrecision);
  //sBuff.port->print("Q0: "); sBuff.port->println(DCU_Q0);
  //sBuff.port->print("Q1: "); sBuff.port->println(DCU_Q1);
  //sBuff.port->print("Q2: "); sBuff.port->println(DCU_Q2);
  //sBuff.port->print("Q3: "); sBuff.port->println(DCU_Q3);
  sBuff.port->print("YPR:");
  sBuff.port->print(DCU_YAW, settings.dataPrecision);
  sBuff.port->print(',');
  sBuff.port->print(DCU_PITCH, settings.dataPrecision);
  sBuff.port->print(',');
  sBuff.port->println(DCU_ROLL, settings.dataPrecision);
  //sBuff.port->print("Yaw: "); sBuff.port->println(DCU_YAW);
  //sBuff.port->print("Pitch: "); sBuff.port->println(DCU_PITCH);
  //sBuff.port->print("Roll: "); sBuff.port->println(DCU_ROLL);

  sBuff.port->print("Gxyz:");
  sBuff.port->print(DCU_GX, settings.dataPrecision);
  sBuff.port->print(',');
  sBuff.port->print(DCU_GY, settings.dataPrecision);
  sBuff.port->print(',');
  sBuff.port->println(DCU_GZ, settings.dataPrecision);
  //sBuff.port->print("Gx: "); sBuff.port->println(DCU_GX);
  //sBuff.port->print("Gy: "); sBuff.port->println(DCU_GY);
  //sBuff.port->print("Gz: "); sBuff.port->println(DCU_GZ);
  sBuff.port->print("Axyz:");
  sBuff.port->print(DCU_AX, settings.dataPrecision);
  sBuff.port->print(',');
  sBuff.port->print(DCU_AY, settings.dataPrecision);
  sBuff.port->print(',');
  sBuff.port->println(DCU_AZ, settings.dataPrecision);
  //sBuff.port->print("Ax: "); sBuff.port->println(DCU_AX);
  //sBuff.port->print("Ay: "); sBuff.port->println(DCU_AY);
  //sBuff.port->print("Az: "); sBuff.port->println(DCU_AZ);
  sBuff.port->print("Mxyz:");
  sBuff.port->print(DCU_MX, settings.dataPrecision);
  sBuff.port->print(',');
  sBuff.port->print(DCU_MY, settings.dataPrecision);
  sBuff.port->print(',');
  sBuff.port->println(DCU_MZ, settings.dataPrecision);
  
  //sBuff.port->print("Mx: "); sBuff.port->println(DCU_MX);
  //sBuff.port->print("My: "); sBuff.port->println(DCU_MY);
  //sBuff.port->print("Mz: "); sBuff.port->println(DCU_MZ);

  sBuff.port->print("Compass Heading:   "); sBuff.port->println(device.cHeading, settings.dataPrecision);
  sBuff.port->print("IMU Initialised?:  "); device.IMU_Enabled        ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("IMU Errors:        "); sBuff.port->println(device.IMU_SetupError);
  sBuff.port->print("ESP Enabled?:      "); device.espEnabled         ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("ESP On Line?:      "); device.espOnline          ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Server Enabled?:   "); device.espServerEnabled   ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("WiFi Connected?:   "); device.espWiFiConnected   ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("USB Enabled?:      "); device.espBTEnabled       ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("UDP Enabled?:      "); device.espUDPEnabled      ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Card Present?:     "); device.cardPresent        ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Card Error?:       "); device.cardInitError      ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Settings File OK?: "); device.cardSettingsFile   ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Log BT?:           "); device.logBluetooth       ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Log USB?:          "); device.logUSB             ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Log SD?:           "); device.logSD              ? sBuff.port->println("True") : sBuff.port->println("False");

  sBuff.port->print("Aux Power:         "); device.v5PowerOn          ? sBuff.port->println("ON") : sBuff.port->println("OFF");
  sBuff.port->print("Usr Switch:        "); device.userSwitchState    ? sBuff.port->println("ON") : sBuff.port->println("OFF");
  sBuff.port->print("Usr Button:        "); device.userButtonState    ? sBuff.port->println("ON") : sBuff.port->println("OFF");
  
  sBuff.port->println();
}


void printDeviceSettings(serialBuffer_t &sBuff){
  sBuff.port->print("IMU Initialised?:    "); device.IMU_Enabled        ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("ESP Enabled?:        "); device.espEnabled         ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("ESP On Line?:        "); device.espOnline          ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Server Enabled?:     "); device.espServerEnabled   ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("WiFi Connected?:     "); device.espWiFiConnected   ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Card Present?:       "); device.cardPresent        ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Card Error?:         "); device.cardInitError      ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Settings File OK?:   "); device.cardSettingsFile   ? sBuff.port->println("True") : sBuff.port->println("False");
  
  sBuff.port->print("Log Quaternions?:    "); device.logQ             ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Log YPR?:            "); device.logYPR           ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Log Gyro?:           "); device.logGyro          ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Log Accellerometer?: "); device.logAccel         ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Log Magnetometer?:   "); device.logMag           ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Log Heading?:        "); device.logHeading       ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Log Inputs?:         "); device.logInputs        ? sBuff.port->println("True") : sBuff.port->println("False");
  sBuff.port->print("Log Outputs?:        "); device.logOutputs       ? sBuff.port->println("True") : sBuff.port->println("False");
  
  sBuff.port->print("Usr Switch:          "); device.userSwitchState    ? sBuff.port->println("ON") : sBuff.port->println("OFF");
  sBuff.port->print("Usr Button:          "); device.userButtonState    ? sBuff.port->println("ON") : sBuff.port->println("OFF");
  
  sBuff.port->print("IMU Errors:          "); sBuff.port->println(device.IMU_SetupError);
}








//print functions for debugging

void USBMessage(String msg){
  USBS.print("DCU Status:");
  USBS.println(msg);
}
void debugPrint(String message){
  String msg = String("DEBUG:" + message);
  Serial.print(msg);
}
void debugPrintln(String message){
  String msg = String("DEBUG:" + message);
  Serial.println(msg);
}
void debugWrite(char ch){
  Serial.write(ch);
}
void debugStartLine(){
  Serial.print("DEBUG:");
}
void debugEndLine(){
  Serial.println("");
}
void printShortState(){
  DEBUG.println(getTimestamp());
  DEBUG.print("BatVoltage: "); DEBUG.println(device.batteryVoltage);
  DEBUG.print("Card Present?:        "); device.cardPresent   ? DEBUG.println("True") : DEBUG.println("False");
  DEBUG.print("Card Error?:          "); device.cardInitError ? DEBUG.println("True") : DEBUG.println("False");
  DEBUG.print("Settings File Found?: "); device.cardSettingsFile ? DEBUG.println("True") : DEBUG.println("False");
  DEBUG.print("Usr Switch: "); device.userSwitchState ? DEBUG.println("ON") : DEBUG.println("OFF");
  DEBUG.print("Usr Button: "); device.userButtonState ? DEBUG.println("ON") : DEBUG.println("OFF");
}

void printSensors(){
  /*
   * device.inputs[0] = Sensor.getRange();
  device.inputs[1] = Sensor.getDelta();
  device.inputs[2] = Sensor.getTOFRange();
  device.inputs[3] = Sensor.getTOFDelta();
  device.inputs[4] = Sensor.getAuxRange();
  device.inputs[5] = Sensor.getAuxDelta();
  */
  DEBUG.print("Range: ");
  DEBUG.print(device.inputs[0]);
  DEBUG.print(" DT: ");
  DEBUG.print(device.inputs[1]);
  DEBUG.print(" TOFR: ");
  DEBUG.print(device.inputs[2]);
  DEBUG.print(" TOFDT: ");
  DEBUG.print(device.inputs[3]);
  DEBUG.print(" AuxR: ");
  DEBUG.print(device.inputs[4]);
  DEBUG.print(" AuxDT: ");
  DEBUG.println(device.inputs[5]);
}
void printIMUData(){
  DEBUG.print("YPR: ");
  DEBUG.print(device.ypr[YAW]);
  DEBUG.print(":");
  DEBUG.print(device.ypr[PITCH]);
  DEBUG.print(":");
  DEBUG.println(device.ypr[ROLL]);
  
  DEBUG.print("Gyro");
  for(int n = 0; n < 3; n++){
    DEBUG.print(":");
    DEBUG.print(device.gyro[n]);
  }
  DEBUG.println();
  DEBUG.print("Accelerometer");
  for(int n = 0; n < 3; n++){
    DEBUG.print(":");
    DEBUG.print(device.accel[n]);
  }
  DEBUG.println();
}

void printTime(){
  printFMillis(millis()-device.clockLastMillis);  DEBUG.print(':');
  printFClock(device.seconds);  DEBUG.print(':');
  printFClock(device.minutes);  DEBUG.print(':');
  printFClock(device.hours);    DEBUG.print(' ');
  printFClock(device.day);      DEBUG.print(':');
  printFClock(device.month);    DEBUG.print(':');
  printFClock(device.year);
}

void printFClock(int number) {
  if (number < 10) {
    DEBUG.print('0'); // print a 0 before if the number is < than 10
  }
  DEBUG.print(number);
}

void printFMillis(int number) {
  if (number < 10) {
    DEBUG.print('0'); // print a 0 before if the number is < 10 
  }else if (number < 100) {
    DEBUG.print("00"); // print a 00 before if the number is < 100
  }
  DEBUG.print(number);
}

void printDeviceState(){

  DEBUG.println(getTimestamp());
  DEBUG.print("BatVoltage: "); DEBUG.println(device.batteryVoltage);
  
  DEBUG.print("Q0: "); DEBUG.println(DCU_Q0);
  DEBUG.print("Q1: "); DEBUG.println(DCU_Q1);
  DEBUG.print("Q2: "); DEBUG.println(DCU_Q2);
  DEBUG.print("Q3: "); DEBUG.println(DCU_Q3);
  
  DEBUG.print("Yaw: "); DEBUG.println(DCU_YAW);
  DEBUG.print("Pitch: "); DEBUG.println(DCU_PITCH);
  DEBUG.print("Roll: "); DEBUG.println(DCU_ROLL);

  
  DEBUG.print("Gx: "); DEBUG.println(DCU_GX);
  DEBUG.print("Gy: "); DEBUG.println(DCU_GY);
  DEBUG.print("Gz: "); DEBUG.println(DCU_GZ);

  DEBUG.print("Ax: "); DEBUG.println(DCU_AX);
  DEBUG.print("Ay: "); DEBUG.println(DCU_AY);
  DEBUG.print("Az: "); DEBUG.println(DCU_AZ);
  
  DEBUG.print("Mx: "); DEBUG.println(DCU_MX);
  DEBUG.print("My: "); DEBUG.println(DCU_MY);
  DEBUG.print("Mz: "); DEBUG.println(DCU_MZ);

  
  DEBUG.print("Compass Heading: "); DEBUG.println(device.cHeading);
  //printTime();
  DEBUG.print("Card Present?:        "); device.cardPresent   ? DEBUG.println("True") : DEBUG.println("False");
  DEBUG.print("Card Error?:          "); device.cardInitError ? DEBUG.println("True") : DEBUG.println("False");
  DEBUG.print("Settings File Found?: "); device.cardSettingsFile ? DEBUG.println("True") : DEBUG.println("False");

  

  DEBUG.print("Usr Switch: "); device.userSwitchState ? DEBUG.println("ON") : DEBUG.println("OFF");
  DEBUG.print("Usr Button: "); device.userButtonState ? DEBUG.println("ON") : DEBUG.println("OFF");
  
  DEBUG.println();
}
