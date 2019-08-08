
#include "ETRT-DCU.h"
#include "haptics.h"
#include "src/Drivers/TOFLidarSensor.h"
//#include "src/Drivers/EZSonar.h"
//#include "Drivers/GPHaptics.h"
TOFLidarSensor Sensor;

Adafruit_NeoPixel_ZeroDMA leds(1, PIN_NEOPIXEL, NEO_GRB);

int testVar;
int delayTime = 10;
int randomVar = 0;
bool ledState = HIGH;

bool printData = false;
uint8_t ledR = 0, ledG = 0, ledB = 0;
//bool forceESPBoot = false;

int sonarInts;
void setup() {
  // put your setup code here, to run once:
  //pinMode(PB12, OUTPUT);
  //Start here then the main loop is a state machine
  
  //For some reason these need to be here now or it will crash the MCU
  attachInterrupt(digitalPinToInterrupt(PIN_IMU_INT), imu_isr, FALLING);
  attachInterrupt(digitalPinToInterrupt(SDCD), cardDetect_isr, CHANGE);

  //SONAR:
  
  pinMode(pulsePin, INPUT);
  attachInterrupt(digitalPinToInterrupt(pulsePin), sonarPulsePin_isr, CHANGE);
  device.deviceState = runStartup();  //Call this until it changes the state
  //device.deviceState = DCU_FINISHED_STARTUP;
  //device.deviceState = DCU_STARTING_SDHC;
  enablePeripheralPower();
}

byte secnd = 0;
unsigned int loopCount = 0;

void loop() {
  //This is a state machine
  /*
   * The device runs the initial startup code in setup()
   * and then goes to the state machine where it runs through a startup sequence
   */
  switch(device.deviceState){
    case DCU_SPEEPING:
      //do nothing
      //If you wake up you will be in this state
      
      setSleepInt(0);
      device.deviceState = runStartup();  //Call this until it changes the state
        //device.deviceState = DCU_FINISHED_STARTUP;
        //device.deviceState = DCU_STARTING_SDHC;
      enablePeripheralPower();
      break;
    case DCU_GOINGTOSLEEP:
      //Set clock to sleep mode
      device.deviceState = DCU_SPEEPING;
      Clock.standbyMode();
      break;
    case DCU_STARTING_SDHC:
      setLEDs(redLED);
      device.deviceState = startSDHC();  //Call this until it changes the state
      break;
    case DCU_STARTING_WIRELESS:
      setLEDs(redLED);
      if(device.printStartupState) printOnce("DCU State: Starting ESP32");
      device.deviceState = startWireless();
      updateComms();
      //Start the ESP32
      break;
    case DCU_STARTING_OUTPUTS:
      //if(device.printStartupState) printOnce("DCU State: Starting Output Module");
      enable5VPower();
      setLEDs(redLED);
      device.deviceState = startOutputs();
      updateComms();
      break;
    case DCU_STARTING_INPUTS:
      //if(device.printStartupState) printOnce("DCU State: Starting Input Module");
      setLEDs(redLED);
      device.deviceState = startInputs();
      updateComms();
      break;
    case DCU_START_READING_SETTINGS:
      //if(device.printStartupState) printOnce("DCU State: Reading Settings File");
      setLEDs(greenLED);
      device.deviceState = startReadingSettings();  //Start reading the settings file
      break;
    case DCU_READING_SETTINGS:
      setLEDs(greenLED);
      device.deviceState = readSettings();  //Call this until it changes the state
      break;
    case DCU_APPLYING_DEFAULTS:
      setLEDs(redLED);
      //if(device.printStartupState) printOnce("DCU State: Apply default settings");
      device.deviceState = applyDefaults();
      updateComms();
      break;
    case DCU_STARTING_IMU:
      setLEDs(redLED);
      //if(device.printStartupState) printOnce("DCU State: Starting IMU");
      device.deviceState = startIMU();
      updateComms();
      //Start the MPU9250
      break;
    case DCU_CONNECTING_WIFI:
      setLEDs(redLED);
      //if(device.printStartupState) printOnce("DCU State: Connecting to WiFI");
      device.deviceState = connectWiFi();
      updateComms();
      //Attempt a connection - send the wifi ssid and password to the ESP32
      break;
    case DCU_WAITING_FOR_WIFI:
      setLEDs(redLED);
      //if(device.printStartupState) printOnce("DCU State: Waiting For WiFI");
      device.deviceState = waitForWiFi();
      updateComms();
      //Wait for ESP32 to connect
      break;
    case DCU_RESTARTING_SDHC:
      setLEDs(redLED);
      //if(device.printStartupState) printOnce("DCU State: Restarting SDHC");
      device.deviceState = restartSDHC();
      updateComms();
      //Wait for ESP32 to connect
      break;
    case DCU_FINISHED_STARTUP:
      //randomiseLED(randomLED);
      //setLEDs(randomLED);
      //beepFor(500);
      enableAudio();
      setLEDs(whiteLED);
      //SDFileTest();
      if(device.printStartupState) printOnce("DCU State: Finished - ready to go");
      device.deviceState = DCU_RUNNING_STANDBY;
      updateComms();
      //Wait for ESP32 to connect
      break;
    case DCU_RUNNING_STANDBY:
      //setLEDs(blueLED);
      if(DEBUGGING) printOnce("DCU State: Running Standby");
      device.deviceState = updateIdle();
      break;
    case DCU_RUNNING_EXPERIMENT:
      //setLEDs(whiteLED);
      if(DEBUGGING) printOnce("DCU State: Running Experiment");
      device.deviceState = updateExperiment();
      break;
    default:
      //setLEDs(redLED);
      device.deviceState = updateIdle();
      break;
  }
}

bool updateNow(){ //returns true if it is time for a log update
  if(millis() < device.nextUpdate) return false;
  device.nextUpdate = millis() + device.updateInterval;
  return true;
}

//UPDATE ALL THE THINGS ===============================================================================================
void updateSystem(){
  //updateBatteryVoltage();
  refreshUserControls();
  updateComms();
  updateIO();
  //checkForOffSwitch(); //check to see if the device is switched off
  updateBatteryVoltage();
  //Print every second
  if(secnd != Clock.getSeconds()){
    secnd = Clock.getSeconds();
    secondTicker();
    //testSDCard();
  }
}

void updateComms(){
  checkUSB();
  checkWireless();
  checkPendingCommands();
  updateIMU();
}

//===============================================================================================
void printOnce(String printMessage){
  static String lastMsg = "";
  if(lastMsg == printMessage) return;
  lastMsg = printMessage;
  DEBUG.println(lastMsg);
}

void checkPendingCommands(){
  /*
   * When a command is completed and the buffer is reset any pending command
   * is transferred to the data buffer for procesing next time around
   */
  if(bufferUSB.isCommandPending == true){
    //send to the command processor 
    checkUSB();
    bufferUSB.isCommandPending = false;
  }
  if(bufferWireless.isCommandPending == true){
    //send to the command processor 
    checkWireless();
    bufferWireless.isCommandPending = false;
  }
}

void testSDCard(){
  if(!device.dataFileOpen) return;
  DEBUG.println(millis());
  createLogLine(); //creates a formatted String of data for the log
  
  updateDataLogs(); //writes the string of data to anything configured to accept it
  DEBUG.println(millis());
}

void testHapticsStart(){
  
  beepOn();
  enableAudio();
  wave0.set(100.0, 1.0);
  wave1.set(100.0, 1.0);
  //vib0.setIntensity(0.8);
  //vib1.setIntensity(0.8);;
  vib0.simpleSet(1020);
  vib1.simpleSet(1020);
}
void testHapticsStop(){
  
  beepOff();
  wave0.set(100.0, 0.0);
  wave1.set(100.0, 0.0);
  //vib0.setIntensity(0.0);
  //vib1.setIntensity(0.0);
  vib0.simpleSet(0);
  vib1.simpleSet(0);
  disableAudio();
}
void secondTicker(){
  loopCount++;
}
