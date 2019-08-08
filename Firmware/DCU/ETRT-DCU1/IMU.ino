


long gyroBias[3] = {0, 0, 0};
long accelBias[3] = {0, 0, 0};

//Sparkfun firmware:

#define DMP_SAMPLE_RATE    50 // Logging/DMP sample rate(4-200 Hz)
#define IMU_COMPASS_SAMPLE_RATE 100 // Compass sample rate (4-100 Hz)
#define IMU_AG_SAMPLE_RATE 100 // Accel/gyro sample rate Must be between 4Hz and 1kHz
#define IMU_GYRO_FSR       2000 // Gyro full-scale range (250, 500, 1000, or 2000)
#define IMU_ACCEL_FSR      2 // Accel full-scale range (2, 4, 8, or 16)
#define IMU_AG_LPF         5 // Accel/Gyro LPF corner frequency (5, 10, 20, 42, 98, or 188 Hz)
#define ENABLE_GYRO_CALIBRATION false

// Defaults all set above
unsigned short accelFSR = IMU_ACCEL_FSR;
unsigned short gyroFSR = IMU_GYRO_FSR;
unsigned short fifoRate = DMP_SAMPLE_RATE;


bool initIMU(void){
  // imu.begin() should return 0 on success. Will initialize
  // I2C bus, and reset MPU-9250 to defaults.
  //attachInterrupt(digitalPinToInterrupt(PIN_IMU_INT), imu_isr, FALLING);
  //attachInterrupt(PIN_IMU_INT, imu_isr, FALLING);
  if(imu.begin() != INV_SUCCESS) {
    if(DEBUGGING) DEBUG.println("INIT Fail Unable to communicate with MPU-9250");
    //device.IMU_Enabled = false;
    device.IMU_SetupError = 1; //failed to communicate
    return false;
  }
  if(DEBUGGING) DEBUG.println("Configuring MPU-9250");
  
  // Set up MPU-9250 interrupt:
  imu.enableInterrupt(); // Enable interrupt output
  imu.setIntLevel(1);    // Set interrupt to active-low
  imu.setIntLatched(1);  // Latch interrupt output

  // Configure sensors:
  // Set gyro full-scale range: options are 250, 500, 1000, or 2000:
  imu.setGyroFSR(gyroFSR);
  // Set accel full-scale range: options are 2, 4, 8, or 16 g 
  imu.setAccelFSR(accelFSR);
  // Set gyro/accel LPF: options are5, 10, 20, 42, 98, 188 Hz
  imu.setLPF(IMU_AG_LPF); 
  // Set gyro/accel sample rate: must be between 4-1000Hz
  // (note: this value will be overridden by the DMP sample rate)
  imu.setSampleRate(IMU_AG_SAMPLE_RATE); 
  // Set compass sample rate: between 4-100Hz
  imu.setCompassSampleRate(IMU_COMPASS_SAMPLE_RATE); 

  if(device.IMUMode == 0){
    if(DEBUGGING) DEBUG.println("Applying MPU-9250 Calibration");
    applyIMUCalibration();
    // Configure digital motion processor. Use the FIFO to get
    // data from the DMP.
    unsigned short dmpFeatureMask = 0;
    if ( device.calibratedGyro ) dmpFeatureMask |= DMP_FEATURE_SEND_CAL_GYRO;
    else dmpFeatureMask |= DMP_FEATURE_SEND_RAW_GYRO;

    // Add accel and quaternion's to the DMP
    //if( device.calibratedAccel ) dmpFeatureMask |= DMP_FEATURE_SEND_RAW_ACCEL;
    //else dmpFeatureMask |= DMP_FEATURE_SEND_C_ACCEL;
    dmpFeatureMask |= DMP_FEATURE_SEND_RAW_ACCEL;
    dmpFeatureMask |= DMP_FEATURE_6X_LP_QUAT;

    if(DEBUGGING) DEBUG.println("Starting DMP");
    // Initialize the DMP, and set the FIFO's update rate:
    imu.dmpBegin(dmpFeatureMask, fifoRate);
    if(DEBUGGING)DEBUG.println("IMU Initialised");
  }else if(device.IMUMode == 1){
    unsigned short dmpFeatureMask = 0;
    dmpFeatureMask |= DMP_FEATURE_SEND_RAW_GYRO;
    dmpFeatureMask |= DMP_FEATURE_SEND_RAW_ACCEL;
    dmpFeatureMask |= DMP_FEATURE_6X_LP_QUAT;
    if(DEBUGGING) DEBUG.println("Starting DMP for calibration");
    // Initialize the DMP, and set the FIFO's update rate:
    imu.dmpBegin(dmpFeatureMask, fifoRate);
    if(DEBUGGING)DEBUG.println("IMU Initialised");
  }
  return true; // Return success
}

void tryStartIMU(){
  device.IMU_Enabled = initIMU();
}

void applyIMUCalibration(){
  //int mpu_set_accel_bias_6500_reg(const long *accel_bias)
  //int mpu_set_gyro_bias_reg(long *gyro_bias)
  //imu.dmpSetGyroBias(settings.gyroBias)
  //mpu_set_accel_bias_6500_reg(settings.accelBias);

  if( mpu_set_gyro_bias_reg(settings.gyroBias) != INV_SUCCESS){
    if(DEBUGGING)DEBUG.println("Failed to set Gyro Bias");;
  }
  if( mpu_set_accel_bias_6500_reg(settings.accelBias) != INV_SUCCESS){
    if(DEBUGGING)DEBUG.println("Failed to set Accel Bias");;
  }
}



bool collectIMUCalibrationData(){
  #define SAMPLES 2048
  long samplesG[3] = {0,0,0};
  long samplesA[3] = {0,0,0};
  long samplesM[3] = {0,0,0};
  uint16_t index = 0;
  int timeout = 0;
  for(uint16_t n = 0; n < SAMPLES; n++){
    while(device.IMU_Interrupted == false){
      if(timeout > 500) return false;
      delay(1);
      timeout++;
    } //wait for the IMU to interrupt
    updateIMU();
    samplesG[GYRO_X] += imu.gx;
    samplesG[GYRO_Y] += imu.gy;
    samplesG[GYRO_Z] += imu.gz;
    
    samplesA[ACCEL_X] += imu.ax;
    samplesA[ACCEL_Y] += imu.ay;
    samplesA[ACCEL_Z] += imu.az;
    
    samplesM[MAG_X] += imu.mx;
    samplesM[MAG_Y] += imu.my;
    samplesM[MAG_Z] += imu.mz;
    while(device.IMU_Interrupted == false){}
  }
  tempGyroBias[GYRO_X] = samplesG[GYRO_X]/SAMPLES;
  tempGyroBias[GYRO_Y] = samplesG[GYRO_Y]/SAMPLES;
  tempGyroBias[GYRO_Z] = samplesG[GYRO_Z]/SAMPLES;
  tempAccelBias[ACCEL_X] = samplesA[ACCEL_X]/SAMPLES;
  tempAccelBias[ACCEL_Y] = samplesA[ACCEL_Y]/SAMPLES;
  tempAccelBias[ACCEL_Z] = samplesA[ACCEL_Z]/SAMPLES;
  tempMagBias[MAG_X] = samplesM[MAG_X]/SAMPLES;
  tempMagBias[MAG_Y] = samplesM[MAG_Y]/SAMPLES;
  tempMagBias[MAG_Z] = samplesM[MAG_Z]/SAMPLES;
  return true;
}

void updateIMU(){
  if(device.IMU_Enabled == false) return;
// Check for new data in the FIFO imu.fifoAvailable()
  if ( imu.fifoAvailable() || device.IMU_Interrupted ){
    device.IMU_Interrupted = false;
    if ( imu.dmpUpdateFifo() == INV_SUCCESS) processIMUData();
    else if(DEBUGGING) DEBUG.println("DEBUG: Failed to read IMU");
    
    if ( imu.updateCompass() == INV_SUCCESS ) processMagData();
    else if(DEBUGGING) DEBUG.println("DEBUG: Failed to read compass");
  }
}


void processIMUData(){
  //unpack all the IMU Data and place in the device struct
  device.gyro[GYRO_X] = imu.calcGyro(imu.gx);
  device.gyro[GYRO_Y] = imu.calcGyro(imu.gy);
  device.gyro[GYRO_Z] = imu.calcGyro(imu.gz);

  device.accel[ACCEL_X] = imu.calcAccel(imu.ax);
  device.accel[ACCEL_Y] = imu.calcAccel(imu.ay);
  device.accel[ACCEL_Z] = imu.calcAccel(imu.az);
  
  device.quaternion[QW] =  imu.calcQuat(imu.qw);
  device.quaternion[QX] =  imu.calcQuat(imu.qx);
  device.quaternion[QY] =  imu.calcQuat(imu.qy);
  device.quaternion[QZ] =  imu.calcQuat(imu.qz);

  imu.computeEulerAngles();

  device.ypr[ROLL]  = imu.roll;
  device.ypr[PITCH] = imu.pitch;
  device.ypr[YAW]   = imu.yaw;
}

void processMagData(){
  device.mag[MAG_X] = imu.calcMag(imu.mx);
  device.mag[MAG_Y] = imu.calcMag(imu.my);
  device.mag[MAG_Z] = imu.calcMag(imu.mz);
  imu.computeCompassHeading();
}

void imu_isr(){
  device.IMU_Interrupted = true;
}


//From MPU9150 code:
/*
 * 
// uint8_t MPU9150::dmpSetLinearAccelFilterCoefficient(float coef);
// uint8_t MPU9150::dmpGetLinearAccel(long *data, const uint8_t* packet);
uint8_t MPU9150::dmpGetLinearAccel(VectorInt16 *v, VectorInt16 *vRaw, VectorFloat *gravity) {
    // get rid of the gravity component (+1g = +4096 in standard DMP FIFO packet)
    v -> x = vRaw -> x - gravity -> x*4096;
    v -> y = vRaw -> y - gravity -> y*4096;
    v -> z = vRaw -> z - gravity -> z*4096;
    return 0;
}
// uint8_t MPU9150::dmpGetLinearAccelInWorld(long *data, const uint8_t* packet);
uint8_t MPU9150::dmpGetLinearAccelInWorld(VectorInt16 *v, VectorInt16 *vReal, Quaternion *q) {
    // rotate measured 3D acceleration vector into original state
    // frame of reference based on orientation quaternion
    memcpy(v, vReal, sizeof(VectorInt16));
    v -> rotate(q);
    return 0;
}
// uint8_t MPU9150::dmpGetGyroAndAccelSensor(long *data, const uint8_t* packet);
// uint8_t MPU9150::dmpGetGyroSensor(long *data, const uint8_t* packet);
// uint8_t MPU9150::dmpGetControlData(long *data, const uint8_t* packet);
// uint8_t MPU9150::dmpGetTemperature(long *data, const uint8_t* packet);
// uint8_t MPU9150::dmpGetGravity(long *data, const uint8_t* packet);
uint8_t MPU9150::dmpGetGravity(VectorFloat *v, Quaternion *q) {
    v -> x = 2 * (q -> x*q -> z - q -> w*q -> y);
    v -> y = 2 * (q -> w*q -> x + q -> y*q -> z);
    v -> z = q -> w*q -> w - q -> x*q -> x - q -> y*q -> y + q -> z*q -> z;
    return 0;
}
// uint8_t MPU9150::dmpGetUnquantizedAccel(long *data, const uint8_t* packet);
// uint8_t MPU9150::dmpGetQuantizedAccel(long *data, const uint8_t* packet);
// uint8_t MPU9150::dmpGetExternalSensorData(long *data, int size, const uint8_t* packet);
// uint8_t MPU9150::dmpGetEIS(long *data, const uint8_t* packet);

uint8_t MPU9150::dmpGetEuler(float *data, Quaternion *q) {
    data[0] = atan2(2*q -> x*q -> y - 2*q -> w*q -> z, 2*q -> w*q -> w + 2*q -> x*q -> x - 1);   // psi
    data[1] = -asin(2*q -> x*q -> z + 2*q -> w*q -> y);                              // theta
    data[2] = atan2(2*q -> y*q -> z - 2*q -> w*q -> x, 2*q -> w*q -> w + 2*q -> z*q -> z - 1);   // phi
    return 0;
}
uint8_t MPU9150::dmpGetYawPitchRoll(float *data, Quaternion *q, VectorFloat *gravity) {
    // yaw: (about Z axis)
    data[0] = atan2(2*q -> x*q -> y - 2*q -> w*q -> z, 2*q -> w*q -> w + 2*q -> x*q -> x - 1);
    // pitch: (nose up/down, about Y axis)
    data[1] = atan(gravity -> x / sqrt(gravity -> y*gravity -> y + gravity -> z*gravity -> z));
    // roll: (tilt left/right, about X axis)
    data[2] = atan(gravity -> y / sqrt(gravity -> x*gravity -> x + gravity -> z*gravity -> z));
    return 0;
}
*/
