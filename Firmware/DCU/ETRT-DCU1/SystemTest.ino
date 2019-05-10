void SDFileTest(){
  String filename = "";
  filename += getFilenameTime();
  filename += ".txt";

  char fname[filename.length()+1];
  filename.toCharArray(fname, filename.length()+1);
  File mf = SD.open(fname, O_RDWR|O_CREAT );
  if(!mf) {
    DEBUG.println("failed to open test file");
  }else{
    DEBUG.println("Opened file");
    mf.println("Hello world");
    mf.println(filename);
    mf.flush();
    mf.close();
    DEBUG.println("closed file");
  }
}

void testAnalogIn(){
  //sample and print from each sensor input
  int val;
  val = analogRead(PIN_A0);
  DEBUG.print("Analog0="); DEBUG.print(val);
  DEBUG.print(" Voltage="); DEBUG.println(analogToVoltage((uint16_t)val));
  val = analogRead(PIN_A1);
  DEBUG.print("Analog1="); DEBUG.print(val);
  DEBUG.print(" Voltage="); DEBUG.println(analogToVoltage((uint16_t)val));
  val = analogRead(PIN_A2);
  DEBUG.print("Analog2="); DEBUG.print(val);
  DEBUG.print(" Voltage="); DEBUG.println(analogToVoltage((uint16_t)val));
  val = analogRead(PIN_A3);
  DEBUG.print("Analog3="); DEBUG.print(val);
  DEBUG.print(" Voltage="); DEBUG.println(analogToVoltage((uint16_t)val));
  val = analogRead(PIN_A4);
  DEBUG.print("Analog4="); DEBUG.print(val);
  DEBUG.print(" Voltage="); DEBUG.println(analogToVoltage((uint16_t)val));
  val = analogRead(PIN_A5);
  DEBUG.print("Analog5="); DEBUG.print(val);
  DEBUG.print(" Voltage="); DEBUG.println(analogToVoltage((uint16_t)val));
  val = analogRead(PIN_A6);
  DEBUG.print("Analog6="); DEBUG.print(val);
  DEBUG.print(" Voltage="); DEBUG.println(analogToVoltage((uint16_t)val));
  val = analogRead(PIN_A7);
  DEBUG.print("Analog7="); DEBUG.print(val);
  DEBUG.print(" Voltage="); DEBUG.println(analogToVoltage((uint16_t)val));
}


/*void serialTesting(int data, serialBuffer_t &sBuff){
  //send some data to a seria port
  sBuff.port->println("Writing To SERIAL2");
  Serial2.print(data);
  while(Serial2.available()){
    sBuff.port->print("Recieved:");
    sBuff.port->println(Serial2.read());
  }
}

void initTest(){
  Serial2.begin(115200);
}
*/
