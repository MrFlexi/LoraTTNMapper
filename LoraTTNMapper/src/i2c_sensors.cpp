#include "globals.h"
#include "i2c_sensors.h"

static const char TAG[] = "";

#if (HAS_INA219)

//Adafruit_INA219 ina219;
Adafruit_INA219 ina219(0x41);

void print_ina219()
{
  Serial.println("---- INA 219 Current Sensor ----");
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  Serial.print("Bus Voltage:   ");
  Serial.print(busvoltage);
  Serial.println(" V");
  Serial.print("Shunt Voltage: ");
  Serial.print(shuntvoltage);
  Serial.println(" mV");
  Serial.print("Load Voltage:  ");
  Serial.print(loadvoltage);
  Serial.println(" V");
  Serial.print("Current:       ");
  Serial.print(current_mA);
  Serial.println(" mA");
  Serial.print("Power:         ");
  Serial.print(power_mW);
  Serial.println(" mW");
  Serial.println("");
}

void setup_ina219()
{
  ESP_LOGI(TAG, "Setup INA 219");
  if (!ina219.begin())
  {
     ESP_LOGI(TAG, "Could not find a valid INA219 sensor");
  }
  else
  {
    ESP_LOGI(TAG, "INA219 sensor found");
    print_ina219();
  }
}

#endif

#if (HAS_INA3221)
SDL_Arduino_INA3221 ina3221;

void print_ina3221()
{
  Serial.println("");
  float shuntvoltage1 = 0;
  float busvoltage1 = 0;
  float current_mA1 = 0;
  float loadvoltage1 = 0;

  busvoltage1 = ina3221.getBusVoltage_V(1);
  shuntvoltage1 = ina3221.getShuntVoltage_mV(1);
  current_mA1 = -ina3221.getCurrent_mA(1); // minus is to get the "sense" right.   - means the battery is charging, + that it is discharging
  loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000);

  Serial.print("Bus Voltage:");
  Serial.print(busvoltage1);
  Serial.println(" V");
  Serial.print("Shunt Voltage:");
  Serial.print(shuntvoltage1);
  Serial.println(" mV");
  Serial.print("Battery Load Voltage:");
  Serial.print(loadvoltage1);
  Serial.println(" V");
  Serial.print("Battery Current 1:");
  Serial.print(current_mA1);
  Serial.println(" mA");
  Serial.println("");
}

void setup_ina3221()
{
  ESP_LOGI(TAG, "Setup INA 3221, Manufacturer ID: 0x%02X", ina3221.getManufID());
  ina3221.begin();
  print_ina3221();
}

#endif

#if (USE_BME280)

Adafruit_BME280 bme;

void setup_bme280()
{
  ESP_LOGI(TAG, "BME280 Setup...");
  unsigned status;

  status = bme.begin(0x76);
  if (!status)
  {
    ESP_LOGI(TAG, "Could not find a valid BME280 sensor");
  }
  else
  {
    Serial.println();
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");
    Serial.print("Pressure = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");
    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");
    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");
    Serial.println();
  }
}
#endif

void setup_i2c_sensors()
{

#if (HAS_INA3221)
  setup_ina3221();
#endif

#if (HAS_INA219)
  setup_ina219();
#endif

#if (USE_BME280)
  setup_bme280();
#endif
}
