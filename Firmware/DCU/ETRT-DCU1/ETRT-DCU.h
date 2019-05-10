#include <Bounce2.h> //debouncer
//#include "Device/ZeroTimer.h"
//#include "Device/RTCZero.h"
#include <SamRTC.h>
//#include <SamDTimer.h>
#include <SPI.h>
//#include <SD.h>
#include <Adafruit_NeoPixel_ZeroDMA.h>
#include "wiring_private.h"

//SDFat
//#include <BlockDriver.h>
//#include <FreeStack.h>
//#include <MinimumSerial.h>
#include <SdFat.h>
//#include <SdFatConfig.h>
#include <sdios.h>
//#include <SysCall.h>
#include <MPU9250_RegisterMap.h>
#include <SparkFunMPU9250-DMP.h>
MPU9250_DMP imu;
inv_error_t initResult;

//Enactive Torch RT V2.0

//Serial port for printing error messages
#define ERRORS Serial
#define DEBUG Serial
#define DEBUGGING false
//#define WAIT_FOR_USB
//#define WAIT_FOR_BUTTON
#define USBS Serial
#define WIRELESS Serial1
#define SENSERIAL SENSOR_SERIAL
Bounce userButtonDB = Bounce();
SdFat SD;
SdFile root;
typedef enum dcuState_t{
  DCU_SPEEPING = 0,
  DCU_GOINGTOSLEEP,
  DCU_STARTING,
  DCU_STARTING_SDHC,
  DCU_STARTING_WIRELESS,
  DCU_STARTING_IMU,
  DCU_STARTING_OUTPUTS,
  DCU_STARTING_INPUTS,
  DCU_START_READING_SETTINGS,
  DCU_READING_SETTINGS,
  DCU_APPLYING_DEFAULTS,
  DCU_CONNECTING_WIFI,
  DCU_WAITING_FOR_WIFI,
  DCU_FINISHED_STARTUP,
  DCU_RESTARTING_SDHC,
  DCU_RUNNING_STANDBY,
  DCU_RUNNING_EXPERIMENT
} dcuState_t;

#define LRED 0
#define LGREEN 1
#define LBLUE 2
uint8_t randomLED[3] = {128,128,128};
const uint8_t whiteLED[3] = {128,128,128};
const uint8_t redLED[3] = {200,0,0};
const uint8_t greenLED[3] = {0,200,0};
const uint8_t blueLED[3] = {0,0,200};
const uint8_t purpleLED[3] = {200,0,200};
const uint8_t orangeLED[3] = {200,100,0};
const uint8_t yellowLED[3] = {200,200,0};




//Data Capture Unit Settings
typedef struct dcuSettings_t{
	int data1;
  const String DCU_FW_VERSION = "1.0.0 ALPHA";
  long gyroBias[3];
  long accelBias[3];
  long magBias[3];
  float batteryScale = BATTERY_VOLTAGE_SCALE;
  uint8_t dataPrecision = 3;
  bool printDebugMessages = true;
  String ssid = "";
  String pswd = "";
  bool wifiSSIDOK = false; //set to true if an ssid has been set
  bool wifiPassOK = false; //set to true if a password has been set
  unsigned long waitForReplyTimer = 0; //used during startup so that ESP32 replies can be waited for
  //ledcol_t ledIdle;
  //SDCard
  File settingsFile;
} dcuSettings_t;

  long tempGyroBias[3];
  long tempAccelBias[3];
  long tempMagBias[3];
//DCU operation data
//Includes internal sensor readings
#define ROLL 2
#define PITCH 1
#define YAW 0
#define GYRO_X  0
#define GYRO_Y  1
#define GYRO_Z  2
#define ACCEL_X 0
#define ACCEL_Y 1
#define ACCEL_Z 2
#define MAG_X   0
#define MAG_Y   1
#define MAG_Z   2
#define QW  0
#define QX  1
#define QY  2
#define QZ  3

#define DCU_Q0 device.quaternion[0]
#define DCU_Q1 device.quaternion[1]
#define DCU_Q2 device.quaternion[2]
#define DCU_Q3 device.quaternion[3]

#define DCU_YAW device.ypr[0]
#define DCU_PITCH device.ypr[1]
#define DCU_ROLL device.ypr[2]

#define DCU_GX device.gyro[0]
#define DCU_GY device.gyro[1]
#define DCU_GZ device.gyro[2]

#define DCU_AX device.accel[0]
#define DCU_AY device.accel[1]
#define DCU_AZ device.accel[2]

#define DCU_MX device.mag[0]
#define DCU_MY device.mag[1]
#define DCU_MZ device.mag[2]
#define INPUTS  6
#define OUTPUTS 6
typedef struct dcuData_t{
	float batteryVoltage;
  bool logQ = false;
	float quaternion[4];
  bool logYPR = false;
	float ypr[3];
  bool logGyro = true;
  bool calibratedGyro = false;
  float gyro[3];
  bool logAccel = true;
  //bool calibratedAccel = true;
  float accel[3];
  bool logMag = true;
  float mag[3];
  bool logHeading = false;
  float cHeading;
  bool logInputs = true;
  float inputs[INPUTS];
  bool logOutputs = true;
  float outputs[OUTPUTS];
  String analogOutWavetype0 = "SINE";
  String analogOutWavetype1 = "SINE";
  float audioFreq0 = 100.0; //100Hz
  float audioFreq1 = 100.0; //100Hz
  //float dataValues[32]; //a single array for storing DCU Data
  unsigned long clockLastMillis;
  byte day;
  byte month;
  byte year;
  byte seconds;
  byte minutes;
  byte hours;
  bool cardPresent = false;
  //bool cardProtected = false;
  bool cardInitError = false;
  bool dataFileOpen = false;
  bool cardSettingsFile = false; //true if the settings file is found


  
  bool peripheralPowerOn = false;
  bool espEnabled = false;
  bool espOnline = false;
  bool espBootMode = false;
  bool espServerEnabled = false;
  bool espTelnetEnabled = false;
  bool espWiFiConnected = false;
  uint8_t espWiFiConnectionErrors = 0;
  String localIPAddress = "";
  bool espUDPEnabled = false;
  bool logUDP = false;
  bool espBTEnabled = false;
  String BTName = "ETRT2";
  bool logBluetooth = false;
  bool logUSB = true;
  bool logSD = false;
  bool logActive = false;
  
  bool v5PowerOn = false;
  //Control flags
  bool IMU_Enabled = false; //IMU enable state - enables when setup is complete
  uint8_t IMU_SetupError = 0; //Record an error during setup
  bool IMU_Interrupted = false; //IMU int state
  uint8_t IMUMode = 0; //mode 0 is normal, mode 1 spits out uncalibrated raw data for calibration run

  bool userSwitchState = false;
  bool oldUserSwitchState = false;
  bool userSwitchChanged = false;
  bool userButtonState = false;
  bool oldUserButtonState = false;
  bool userButtonChanged = false;

  uint8_t sensorType = 1;  //default to LIDAR
  String sensorName = "LIDAR";
  //Button Actions:
  bool buttonEnableHaptics = true; //map sensors to haptics when button pressed
  bool buttonStartLog = true; //Log data when button pressed - Streams data over enabled ports and if SD logging enabled, starts a log file, stops when button released
  //SDCard
  File dataLogFile;
  String dataLogFilename;
  
  dcuState_t deviceState = DCU_STARTING;
  unsigned long debugTimer1 = 0;
  
  bool printStartupState = true;
  unsigned long nextUpdate = 0; //Time of the next update for the data log system
  unsigned long updateInterval = 20; //update interval in milliseconds
  unsigned long sdLogEndDelay = 500; //A delay before ending the SD log - for when the user button is used
  unsigned long sdLogEndTime = 0; //for storing the time when the end command was triggered
} dcuData_t;

typedef struct wifiSettings_t{
	String ssid = "";
	String pswd = "";
} wifiSettings_t;

#define SBUFFER 1024
typedef enum bufferState_t{
  WAITING_FOR_START = 0,
  BUFFERING_PACKET,
  PACKET_RECEIVED
} bufferState_t;

typedef enum serialPortMode_t{
  NO_OPERATION = 0,
  COMMAND_MODE,
  PASSTHROUGH,
  PASSTHROUGH_COMMAND,
  FILEREADER_TOSERIAL,
  FILEREADER_COMMAND
} serialPortMode_t;

typedef struct serialBuffer_t{
  Stream *port = &Serial; //initialised to something sensible . . .
  String bufferString = "";
  unsigned long bytesWritten = 0;
  bool newData = false;
  bool newLine = false;
  bool bufferFull = false;
  serialPortMode_t mode = COMMAND_MODE;
  Stream *altPort = &Serial1; //for pass through - initialised to something sensible
  uint8_t commandVal = 0;
  bufferState_t parseState = WAITING_FOR_START;
  const uint8_t END_OF_PACKET = '\n';
  //const uint8_t PACKET_START = '#';
  File fileReader;
  bool endOfFile = false;
  String pendingCommandString = "";
  bool isCommandPending = false;
} serialBuffer_t;

typedef struct dataLog_t{
  String buffer[2] = {"",""};
  uint8_t i = 0;
  uint16_t bytesWritten[2] = {0,0};
} dataLog_t;

dataLog_t logBuffer;
/* Create an rtc object */
SamRTC Clock;


dcuSettings_t settings;
dcuData_t device;
wifiSettings_t wifiSettings;
//Serial command buffers
serialBuffer_t bufferUSB, bufferWireless;
serialBuffer_t bufferSD;


void enablePeripheralPower(){
  device.peripheralPowerOn = true;
  digitalWrite(PIN_PERIPHERAL_POWER_ENABLE, device.peripheralPowerOn);
}
void disablePeripheralPower(){
  device.peripheralPowerOn = false;
  digitalWrite(PIN_PERIPHERAL_POWER_ENABLE, device.peripheralPowerOn);
}

void enable5VPower(){
  device.v5PowerOn = true;
  digitalWrite(PIN_5V_ENABLE, device.v5PowerOn);
}
void disable5VPower(){
  device.v5PowerOn = false;
  digitalWrite(PIN_5V_ENABLE, device.v5PowerOn);
}

void updateBatteryVoltage(){
  device.batteryVoltage = (float)analogRead(BATTERYMON)* settings.batteryScale;// ( ((float)analogRead(BATTERYMON) ) / ADC_TO_VOLTAGE ) * BATTERY_VOLTAGE_SCALE;
}



float analogToVoltage(uint16_t analogValue){
  return (float)analogValue / ADC_TO_VOLTAGE;
}

void enableESP(){
  device.espEnabled = true;
  digitalWrite(PIN_ESP32_ENABLE, device.espEnabled);
}

void disableESP(){
  device.espEnabled = false;
  digitalWrite(PIN_ESP32_ENABLE, device.espEnabled);
}

void enableESPBootloader(){
  device.espBootMode = false;
  digitalWrite(PIN_ESP_BOOT, device.espBootMode);
}

void disableESPBootloader(){
  device.espBootMode = true;
  digitalWrite(PIN_ESP_BOOT, device.espBootMode);
}
void initDCU(){
  analogReadResolution(ADC_RESOLUTION);
  logBuffer.buffer[0].reserve(512);
  logBuffer.buffer[1].reserve(512);
  pinMode(PIN_5V_ENABLE, OUTPUT);
  pinMode(PIN_PERIPHERAL_POWER_ENABLE, OUTPUT);
  pinMode(PIN_ESP32_ENABLE, OUTPUT);
  pinMode(PIN_ESP_BOOT, OUTPUT);
  pinMode(PIN_IMU_INT, INPUT_PULLUP);
  pinMode(PIN_USER_SWITCH, INPUT_PULLUP);
  pinMode(PIN_USER_BUTTON, INPUT_PULLUP);
  //pinMode(PA27, INPUT_PULLUP); //test - user button
  userButtonDB.attach(PIN_USER_BUTTON);
  userButtonDB.interval(20); // interval in ms
  disablePeripheralPower();
  disable5VPower();
  //enablePeripherals();
  updateBatteryVoltage();

  disableESP();
  disableESPBootloader();
}

void prepareToSleep(){
  //Shut everything down and get ready to go to sleep mode
  device.deviceState = DCU_GOINGTOSLEEP;
  Clock.disableAlarm();
  if(device.dataFileOpen) device.dataLogFile.close();
  device.dataFileOpen = false;
  delay(10);
  disable5VPower();
  disablePeripheralPower();

  device.cardPresent = false;
  //bool cardProtected = false;
  device.cardInitError = false;
  device.dataFileOpen = false;
  device.cardSettingsFile = false; //true if the settings file is found

  device.peripheralPowerOn = false;
  device.espEnabled = false;
  device.espOnline = false;
  device.espBootMode = false;
  device.espServerEnabled = false;
  device.espWiFiConnected = false;
  device.espWiFiConnectionErrors = 0;
  device.localIPAddress = "";
  device.espUDPEnabled = false;
  device.logUDP = false;
  device.espBTEnabled = false;
  device.logBluetooth = false;
  device.logUSB = false;
  device.v5PowerOn = false;
  device.IMU_Enabled = false; //IMU enable state - enables when setup is complete
  device.IMU_SetupError = 0; //Record an error during setup
  device.IMU_Interrupted = false;

  
  delay(10);
}



void dcuWakeup_isr(){
  //wake the device up
  device.deviceState = DCU_SPEEPING;
}

void setSleepInt(bool state){
  //set the state of the sleep interrupt pin - when enabled the device will wake from sleep
  if(state){
    pinMode(PB08, INPUT);
    attachInterrupt(digitalPinToInterrupt(PB08), dcuWakeup_isr, RISING);
  }
  else{
    detachInterrupt(digitalPinToInterrupt(PB08));
  }
}

float calculateDelta(unsigned long lastMicros){
  unsigned long diff = micros() - lastMicros;
  return ((float)diff) / 1000000.0;
}

void printEr(String errorMessage){
  ERRORS.print("ERROR: ");
  ERRORS.print(errorMessage);
}

void printErln(String errorMessage){
  ERRORS.print("ERROR: ");
  ERRORS.println(errorMessage);
}
