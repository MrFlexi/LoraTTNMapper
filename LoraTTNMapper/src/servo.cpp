#include <Arduino.h>
#include "globals.h"
#include "ServoEasing.hpp"

#if (USE_PWM_SERVO)

ServoEasing Servo1(PCA9685_DEFAULT_ADDRESS, &Wire);
ServoEasing Servo2(PCA9685_DEFAULT_ADDRESS, &Wire);

void setup_servo_pwm()
{
  
  Servo1.attach(0);
  Servo1.setEasingType(EASE_CUBIC_IN_OUT);

  Servo2.attach(1);
  Servo2.setEasingType(EASE_CUBIC_IN_OUT);
}

void servo_move_to()
{
Servo1.startEaseTo(dataBuffer.data.servo1, 30);
Servo2.startEaseTo(dataBuffer.data.servo1, 30);

}

void servo_pwm_test() {

  int speed = 20;

  ESP_LOGI(TAG, "Turning Servos");
  Servo1.startEaseTo(40, speed);
  delay(1000);
  Servo1.startEaseTo(20, speed);
  delay(1000);
  Servo1.startEaseTo(180,speed);
  

  Servo2.startEaseTo(20, speed);
  delay(2000);
  Servo2.startEaseTo(90, speed);
  delay(2000);
  Servo2.startEaseTo(120, speed);
  delay(2000);
  ESP_LOGI(TAG, "End");
}




#endif