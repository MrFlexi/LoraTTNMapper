#include "globals.h"
#include "gyro.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps_V6_12.h"

MPU6050 mpu;

volatile bool mpuInterrupt = false; // indicates whether MPU interrupt pin has gone high

#define OUTPUT_READABLE_YAWPITCHROLL
//#define OUTPUT_READABLE_WORLDACCEL

#define LED_PIN 14 // (Arduino is 13, Teensy is 11, Teensy++ is 6)

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
Quaternion quat;     // [w, x, y, z]         quaternion container
VectorInt16 aa;      // [x, y, z]            accel sensor measurements
VectorInt16 gy;      // [x, y, z]            gyro sensor measurements
VectorInt16 aaReal;  // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld; // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity; // [x, y, z]            gravity vector
float euler[3];      // [psi, theta, phi]    Euler angle container
float ypr[3];        // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector


// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

void gyro_dump_interrupt_source(uint8_t mpuIntStatus)
{
    Serial.println("");
    Serial.println("Gyro Interrupt:");
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
        ESP_LOGE(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
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
        mpu.setMotionDetectionDuration(70);
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
        //gyro_dump_interrupt_source(mpuIntStatus);
        
        I2C_MUTEX_UNLOCK(); // release i2c bus access

        #if (WAKEUP_BY_MOTION)
        pinMode(GYRO_INT_PIN, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(GYRO_INT_PIN), GYRO_IRQ, RISING);
        #endif
        
    }
}

void gyro_get_values()
{
         //mpu.resetFIFO();
        fifoCount = mpu.getFIFOCount();
        Serial.print("Fifi Count");
        Serial.println(fifoCount);

        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize)
            fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);

        mpu.dmpGetQuaternion(&quat, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &quat);
        mpu.dmpGetYawPitchRoll(ypr, &quat, &gravity);  

        dataBuffer.data.yaw = ypr[0] * 180 / M_PI;
        dataBuffer.data.pitch = ypr[1] * 180 / M_PI;
        dataBuffer.data.roll = ypr[2] * 180 / M_PI;
  
}

void gyro_show_acc()
{

        fifoCount = mpu.getFIFOCount();
    Serial.print("Fifi Count");
    Serial.println(fifoCount);

    // wait for correct available data length, should be a VERY short wait
    while (fifoCount < packetSize)
        fifoCount = mpu.getFIFOCount();

    // read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);

#ifdef OUTPUT_READABLE_YAWPITCHROLL
    // display Euler angles in degrees
    mpu.dmpGetQuaternion(&quat, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &quat);
    mpu.dmpGetYawPitchRoll(ypr, &quat, &gravity);
    Serial.print("ypr\t");
    Serial.print(ypr[0] * 180 / M_PI);
    Serial.print("\t");
    Serial.print(ypr[1] * 180 / M_PI);
    Serial.print("\t");
    Serial.print(ypr[2] * 180 / M_PI);
    Serial.println();
#endif
}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void gyro_handle_interrupt(void)
{    
    // block i2c bus access
    if (!I2C_MUTEX_LOCK())
        ESP_LOGE(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
    else
    {
        mpuIntStatus = mpu.getIntStatus();
        //mpu.setIntMotionEnabled(false);
        

        if (mpuIntStatus & _BV(MPU6050_INTERRUPT_MOT_BIT))
        {
            gyro_get_values();
            dataBuffer.data.MotionCounter = TIME_TO_NEXT_SLEEP_WITHOUT_MOTION;

#if (FASTLED_SHOW_DEGREE)
            LED_showDegree(dataBuffer.data.pitch);
#endif
        }

        gyro_dump_interrupt_source(mpuIntStatus);
        gyro_show_acc();
        //mpu.setIntMotionEnabled(true);
        I2C_MUTEX_UNLOCK(); // release i2c bus access
    }
}