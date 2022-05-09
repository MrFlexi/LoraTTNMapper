#include <Arduino.h>
#include "globals.h"
#include "ServoEasing.hpp"
#include <Preferences.h>

Preferences prefs;

#define servo1_pin 0
#define servo2_pin 1

Helios helios;
double dAzimuth;
double dElevation;

#if (USE_PWM_SERVO)

ServoEasing Servo1(PCA9685_DEFAULT_ADDRESS, &Wire);
ServoEasing Servo2(PCA9685_DEFAULT_ADDRESS, &Wire);

void save_servo_position_to_flash(uint8_t servo1_pos, uint8_t servo2_pos)
{
  if (prefs.begin("SERVO", false)) // Read/Write
  {
    prefs.clear();
    ESP_LOGI(TAG, "FLASH free entries: %d\n", prefs.freeEntries());
    if (prefs.putUChar("servo1_pos", servo1_pos) == 0)
      ESP_LOGE(TAG, "Error writing Servo 1 pos");

    if (prefs.putUChar("servo2_pos", servo2_pos) == 0)
      ESP_LOGE(TAG, "Error writing Servo 2 pos");

    ESP_LOGI(TAG, "FLASH free entries: %d\n", prefs.freeEntries());
    prefs.end();
  }

  else
  {
    ESP_LOGE(TAG, "Fehler beim Öffnen des NVS-Namespace");
  }
}

void get_servo_position_from_flash(uint8_t *servo1_pos, uint8_t *servo2_pos)
{
  if (prefs.begin("SERVO", true)) // open read only
  {
    ESP_LOGI(TAG, "FLASH free entries: %d\n", prefs.freeEntries());
    *servo1_pos = prefs.getUChar("servo1_pos", false);
    *servo2_pos = prefs.getUChar("servo2_pos", false);

    ESP_LOGI(TAG, "FlashServoPosition:   %d %d", *servo1_pos, *servo2_pos);
    prefs.end();
  }
  else
  {
    ESP_LOGE(TAG, "Fehler beim Öffnen des NVS-Namespace");
  }
}

//--------------------------------------------------------------------------
// Sun Elevation Calculation
//--------------------------------------------------------------------------
#if (USE_SUN_POSITION)


void calc_sun()
{
  //----------------------------------------
  // Calc Sun Position München
  //----------------------------------------
  //Serial.println();
  //Serial.println();
  Serial.println("Sun Azimuth and Elevation Munich");

  helios.calcSunPos(2022, dataBuffer.data.timeinfo.tm_mon, dataBuffer.data.timeinfo.tm_mday, dataBuffer.data.timeinfo.tm_hour - 2, dataBuffer.data.timeinfo.tm_min, 00.00, 11.57754, 48.13641);
  //helios.calcSunPos(2022, dataBuffer.data.timeinfo.tm_mon, dataBuffer.data.timeinfo.tm_mday, 12, dataBuffer.data.timeinfo.tm_min, 00.00, 11.57754, 48.13641);
  ESP_LOGI(TAG, "Azimuth: %g.2\n", helios.dAzimuth);
  ESP_LOGI(TAG, "Elevation: %g.2\n", helios.dElevation);

  dataBuffer.data.sun_azimuth = helios.dAzimuth;
  dataBuffer.data.sun_elevation = helios.dElevation;
}
#endif


void wireReadRegister(uint8_t reg)
{
  uint16_t value16;
  uint8_t value;
  Wire.beginTransmission(PCA9685_DEFAULT_ADDRESS);
  Wire.write(reg);                       // Register
  Wire.endTransmission();
  delay(10); 
  Wire.requestFrom(PCA9685_DEFAULT_ADDRESS, (uint8_t)1);  
  //value = ((Wire.read() << 8) | Wire.read());
  value = Wire.read();
  Serial.print("Reg:");Serial.print(reg, HEX); Serial.print("  Val:"); Serial.println(value,BIN);
}

void readRegisters()
{
    Serial.println("Chip:");
    wireReadRegister(PCA9685_MODE1_REGISTER);
    Serial.println();
    Serial.println("Servo1:");
    wireReadRegister((PCA9685_FIRST_PWM_REGISTER + 2) + 4 * servo1_pin );
    wireReadRegister((PCA9685_FIRST_PWM_REGISTER + 3) + 4 * servo1_pin );
    Serial.println();Serial.println();

    Serial.println("Servo2:");
    wireReadRegister((PCA9685_FIRST_PWM_REGISTER + 2) + 4 * servo2_pin );
    wireReadRegister((PCA9685_FIRST_PWM_REGISTER + 3) + 4 * servo2_pin );
    Serial.println();
}

void setup_servo_pwm()
{

  uint8_t servo1_pos;
  uint8_t servo2_pos;

  //Servo1.I2CWriteByte(PCA9685_MODE1_REGISTER, _BV(PCA9685_MODE_1_SLEEP)); // go to sleep 

  struct tm timeinfo;
  int speed = 30;

  ESP_LOGI(TAG, "Attaching Servos");
  Servo1.attach(servo1_pin);
  Servo1.setEasingType(EASE_CUBIC_IN_OUT);

  Servo2.attach(servo2_pin);
  Servo2.setEasingType(EASE_CUBIC_IN_OUT);
}

void servo_move_to( uint8_t servo_number , uint8_t servo_position )
{
  
  setup_servo_pwm();
  ESP_LOGI(TAG, "Turning Servo %d to %d", dataBuffer.data.servo1, dataBuffer.data.servo2);
  ServoEasing::ServoEasingArray[servo_number]->easeTo(servo_position, 30); 
  delay(2000);  
    
}


#if (USE_SUN_POSITION)
void servo_move_to_sun()
{

  uint8_t servo1_pos;
  uint8_t servo2_pos;
  struct tm timeinfo;
  int speed = 30;

  if (getLocalTime(&timeinfo))
  {    
    calc_sun();

    uint8_t servo1_pos = (uint8_t) dataBuffer.data.sun_azimuth - 79; // 90-11 Grad Servoanpassung
    uint8_t servo2_pos = (uint8_t) dataBuffer.data.sun_elevation;

    ESP_LOGI(TAG, "Sun azimut/elevation: %05.2g %05.2g", dataBuffer.data.sun_azimuth, dataBuffer.data.sun_elevation);    
    save_servo_position_to_flash(servo1_pos, servo2_pos);
    
    ESP_LOGI(TAG, "Attaching Servos");
    ESP_LOGI(TAG, "Turning servos to   : %d  %d", servo1_pos, servo2_pos);

    Servo1.setEasingType(EASE_CUBIC_IN_OUT);
    if ((servo1_pos >= 0) && (servo1_pos <= 180))
      Servo1.attach(servo1_pin, servo1_pos);

    Servo2.setEasingType(EASE_CUBIC_IN_OUT);
    if ((servo2_pos >= 0) && (servo2_pos <= 180))
      Servo2.attach(servo2_pin, servo2_pos);

    //if (servo1_pos >= 0)
    //  Servo1.startEaseTo(servo1_pos, speed);
    //if (servo2_pos >= 0)
    //  Servo2.startEaseTo(servo2_pos, speed);

    delay(5000);
    // readRegisters();    
    ESP_LOGI(TAG, "Servos detached");

    Servo2.detach();
    Servo1.detach();
    // readRegisters();
 
  }
  else
  {
    ESP_LOGE(TAG, "No time information (WLAN/GPS) available, getting old position from flash");
  }
}
#endif

void servo_move_to_last_position()
{

  uint8_t servo1_pos;
  uint8_t servo2_pos;
  struct tm timeinfo;
  int speed = 30;

  ESP_LOGI(TAG, "Moving Servos to last position from flash");
  get_servo_position_from_flash(&servo1_pos, &servo2_pos);  

  Servo1.setEasingType(EASE_CUBIC_IN_OUT);
   if ((servo1_pos >= 0) && (servo1_pos <= 180))
    Servo1.attach(servo1_pin, servo1_pos);

  Servo2.setEasingType(EASE_CUBIC_IN_OUT);
   if ((servo2_pos >= 0) && (servo2_pos <= 180))
    Servo2.attach(servo2_pin, servo2_pos);

  delay(5000);

  Servo1.detach();
  Servo2.detach();
  ESP_LOGI(TAG, "Servos detached");
}

#endif