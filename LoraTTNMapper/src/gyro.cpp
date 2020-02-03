#include "globals.h"
#include "gyro.h"

#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps_V6_12.h"

MPU6050 mpu;

volatile bool mpuInterrupt = false; // indicates whether MPU interrupt pin has gone high

#define OUTPUT_READABLE_YAWPITCHROLL
#define OUTPUT_READABLE_WORLDACCEL


#define LED_PIN 14               // (Arduino is 13, Teensy is 11, Teensy++ is 6)

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

// orientation/motion vars
Quaternion q;        // [w, x, y, z]         quaternion container
VectorInt16 aa;      // [x, y, z]            accel sensor measurements
VectorInt16 gy;      // [x, y, z]            gyro sensor measurements
VectorInt16 aaReal;  // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld; // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity; // [x, y, z]            gravity vector
float euler[3];      // [psi, theta, phi]    Euler angle container
float ypr[3];        // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

// packet structure for InvenSense teapot demo
uint8_t teapotPacket[14] = {'$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n'};

// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================



void IRAM_ATTR dmpDataReady()
{
    mpuInterrupt = true;
}

void gyro_dump_interrupt_source(uint8_t mpuIntStatus)
{
    if (mpuIntStatus & _BV(MPU6050_INTERRUPT_DATA_RDY_BIT))
    {
        Serial.println("DATA_RDY_BIT");
    }
    if (mpuIntStatus & _BV(MPU6050_INTERRUPT_DMP_INT_BIT))
    {
        Serial.println("DMP_INT_BIT");
    }
    if (mpuIntStatus & _BV(MPU6050_INTERRUPT_PLL_RDY_INT_BIT))
    {
        Serial.println("PLL_RDY_INT_BIT");
    }
    if (mpuIntStatus & _BV(MPU6050_INTERRUPT_I2C_MST_INT_BIT))
    {
        Serial.println("I2C_MST_INT_BIT ");
    }
    if (mpuIntStatus & _BV(MPU6050_INTERRUPT_FIFO_OFLOW_BIT))
    {
        Serial.println("FIFO_OFLOW_BIT ");
    }
    if (mpuIntStatus & _BV(MPU6050_INTERRUPT_ZMOT_BIT))
    {
        Serial.println("ZMOT_BIT");
    }
    if (mpuIntStatus & _BV(MPU6050_INTERRUPT_MOT_BIT))
    {
        Serial.println("MOT_BIT");
    }
    if (mpuIntStatus & _BV(MPU6050_INTERRUPT_FF_BIT))
    {
        Serial.println("FF_BIT ");
    }
}

void checkSettings()
{
    Serial.println();

    Serial.print(" * Sleep Mode:                ");
    Serial.println(mpu.getSleepEnabled() ? "Enabled" : "Disabled");

    Serial.print(" * Motion Interrupt:     ");
    Serial.println(mpu.getIntMotionEnabled() ? "Enabled" : "Disabled");

    Serial.print(" * Zero Motion Interrupt:     ");
    Serial.println(mpu.getIntZeroMotionEnabled() ? "Enabled" : "Disabled");

    Serial.print(" * Clock Source:              ");
    switch (mpu.getClockSource())
    {
    case MPU6050_CLOCK_KEEP_RESET:
        Serial.println("Stops the clock and keeps the timing generator in reset");
        break;
    case MPU6050_CLOCK_PLL_ZGYRO:
        Serial.println("PLL with Z axis gyroscope reference");
        break;
    case MPU6050_CLOCK_PLL_YGYRO:
        Serial.println("PLL with Y axis gyroscope reference");
        break;
    case MPU6050_CLOCK_PLL_XGYRO:
        Serial.println("PLL with X axis gyroscope reference");
        break;
    }

    Serial.print(" * Accelerometer:             ");

    Serial.println();
}

// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup_gyro()
{

    Wire.begin();
    Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties

if (!I2C_MUTEX_LOCK())
    ESP_LOGV(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
  else
  {
    // initialize device
    Serial.println(F("Initializing I2C devices..."));
    mpu.initialize();
    pinMode(GYRO_INT_PIN, INPUT);
 

    // verify connection
    Serial.println(F("Testing device connections..."));
    Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

    // load and configure the DMP
    Serial.println(F("Initializing DMP..."));
    devStatus = mpu.dmpInitialize();

    mpu.setIntEnabled(false);
    mpu.setIntZeroMotionEnabled(false);
    mpu.setIntDataReadyEnabled(false);
    mpu.setIntFIFOBufferOverflowEnabled(false);
    mpu.setIntDMPEnabled(false);
    mpu.setIntI2CMasterEnabled(false);
    mpu.setIntFreefallEnabled(false);

    mpu.setIntMotionEnabled(true);
    mpu.setMotionDetectionDuration(50);
    mpu.setMotionDetectionThreshold(3);

    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXGyroOffset(51);
    mpu.setYGyroOffset(8);
    mpu.setZGyroOffset(21);
    mpu.setXAccelOffset(1150);
    mpu.setYAccelOffset(-50);
    mpu.setZAccelOffset(1060);
    // make sure it worked (returns 0 if so)

    // Calibration Time: generate offsets and calibrate our MPU6050
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    Serial.println();
    mpu.PrintActiveOffsets();
    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    mpu.setAccelerometerPowerOnDelay(10);
    mpu.setDHPFMode(1);

    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();

    checkSettings();


    mpuIntStatus = mpu.getIntStatus();
    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    Serial.print(F("DMP ready! Waiting for first interrupt..."));
    gyro_dump_interrupt_source(mpuIntStatus);
    pinMode(GYRO_INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(GYRO_INT_PIN), dmpDataReady, RISING);
    I2C_MUTEX_UNLOCK(); // release i2c bus access
  }
   
}



void gyro_show_acc()
{


        //mpu.resetFIFO();
        fifoCount = mpu.getFIFOCount();
        Serial.print("Fifi Count"); Serial.println(fifoCount);
 
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize)
            fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);


#ifdef OUTPUT_READABLE_QUATERNION
        // display quaternion values in easy matrix form: w x y z
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        Serial.print("quat\t");
        Serial.print(q.w);
        Serial.print("\t");
        Serial.print(q.x);
        Serial.print("\t");
        Serial.print(q.y);
        Serial.print("\t");
        Serial.println(q.z);
#endif

#ifdef OUTPUT_READABLE_EULER
        // display Euler angles in degrees
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetEuler(euler, &q);
        Serial.print("euler\t");
        Serial.print(euler[0] * 180 / M_PI);
        Serial.print("\t");
        Serial.print(euler[1] * 180 / M_PI);
        Serial.print("\t");
        Serial.println(euler[2] * 180 / M_PI);
#endif

#ifdef OUTPUT_READABLE_YAWPITCHROLL
        // display Euler angles in degrees
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        Serial.print("ypr\t");
        Serial.print(ypr[0] * 180 / M_PI);
        Serial.print("\t");
        Serial.print(ypr[1] * 180 / M_PI);
        Serial.print("\t");
        Serial.print(ypr[2] * 180 / M_PI);
        /*
            mpu.dmpGetAccel(&aa, fifoBuffer);
            Serial.print("\tRaw Accl XYZ\t");
            Serial.print(aa.x);
            Serial.print("\t");
            Serial.print(aa.y);
            Serial.print("\t");
            Serial.print(aa.z);
            mpu.dmpGetGyro(&gy, fifoBuffer);
            Serial.print("\tRaw Gyro XYZ\t");
            Serial.print(gy.x);
            Serial.print("\t");
            Serial.print(gy.y);
            Serial.print("\t");
            Serial.print(gy.z);
            */
        Serial.println();

#endif

#ifdef OUTPUT_READABLE_REALACCEL
        // display real acceleration, adjusted to remove gravity
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetAccel(&aa, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
        Serial.print("areal\t");
        Serial.print(aaReal.x);
        Serial.print("\t");
        Serial.print(aaReal.y);
        Serial.print("\t");
        Serial.println(aaReal.z);
#endif

#ifdef OUTPUT_READABLE_WORLDACCEL
        // display initial world-frame acceleration, adjusted to remove gravity
        // and rotated based on known orientation from quaternion
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetAccel(&aa, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
        mpu.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
        Serial.print("aworld\t");
        Serial.print(aaWorld.x);
        Serial.print("\t");
        Serial.print(aaWorld.y);
        Serial.print("\t");
        Serial.println(aaWorld.z);
#endif
}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void gyro_handle_interrupt( void)
{
  mpuInterrupt = false;
  // block i2c bus access
  if (!I2C_MUTEX_LOCK())
    ESP_LOGV(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
  else
  {
        mpuIntStatus = mpu.getIntStatus();
        //mpu.setIntMotionEnabled(false);
        I2C_MUTEX_UNLOCK(); // release i2c bus access
 
        Serial.println(mpuIntStatus);
        dataBuffer.data.MotionCounter = 60;

        gyro_dump_interrupt_source(mpuIntStatus);
        //show();
        //mpu.setIntMotionEnabled(true);
     }
}