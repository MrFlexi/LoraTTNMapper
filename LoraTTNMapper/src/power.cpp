// Basic config
#include "globals.h"
#include "power.h"

// Local logging tag
static const char TAG[] = __FILE__;

#if (HAS_PMU)

AXP20X_Class pmu;

void AXP192_power(pmu_power_t powerlevel)
{

  switch (powerlevel)
  {

  default:
    pmu.setPowerOutPut(AXP192_LDO2, AXP202_ON);  // Lora on T-Beam V1.0
    pmu.setPowerOutPut(AXP192_LDO3, AXP202_ON);  // Gps on T-Beam V1.0
    pmu.setPowerOutPut(AXP192_DCDC1, AXP202_ON); // OLED on T-Beam v1.0
    //pmu.setChgLEDMode(AXP20X_LED_LOW_LEVEL);
    pmu.setChgLEDMode(AXP20X_LED_BLINK_4HZ);
    ESP_LOGI(TAG, "AXP power ON");
    break;

  case pmu_power_sleep:
    pmu.setChgLEDMode(AXP20X_LED_BLINK_1HZ);
    // we don't cut off DCDC1, because then display blocks i2c bus
    pmu.setPowerOutPut(AXP192_LDO3, AXP202_OFF); // gps off
    pmu.setPowerOutPut(AXP192_LDO2, AXP202_OFF); // lora off
    ESP_LOGI(TAG, "AXP power SLEEP");
    break;

  case pmu_power_off:
    pmu.setChgLEDMode(AXP20X_LED_OFF);
    pmu.setPowerOutPut(AXP192_DCDC1, AXP202_OFF);
    pmu.setPowerOutPut(AXP192_LDO3, AXP202_OFF);
    pmu.setPowerOutPut(AXP192_LDO2, AXP202_OFF);
    ESP_LOGI(TAG, "AXP power OFF");
    break;
  }
}

void AXP192_power_gps(bool on)
{
  if (on)
  {
    pmu.setPowerOutPut(AXP192_LDO3, AXP202_ON); // Gps on T-Beam V1.0
    ESP_LOGI(TAG, "GPS power ON");
  }
  else
  {
    pmu.setPowerOutPut(AXP192_LDO3, AXP202_OFF);
    ESP_LOGI(TAG, "GPS power OFF");
  }
}

void AXP192_showstatus(void)
{

  if (pmu.isBatteryConnect())
    if (pmu.isChargeing())
      ESP_LOGI(TAG, "Battery charging, %.2fV @ %.0fmAh",
               pmu.getBattVoltage() / 1000, pmu.getBattChargeCurrent());
    else
      ESP_LOGI(TAG, "Battery not charging");
  else
    ESP_LOGI(TAG, "No Battery");

  if (pmu.isVBUSPlug())
    ESP_LOGI(TAG, "USB powered %.2fV @ %.0fmAh", (pmu.getVbusVoltage() / 1000), pmu.getVbusCurrent());
  else
    ESP_LOGI(TAG, "USB not present");

  int cur = pmu.getChargeControlCur();
  ESP_LOGI(TAG, "Current charge control current = %d mA \n", cur);
}

void AXP192_event_handler(void)
{
  ESP_LOGI(TAG, "PMU Event");
  if (!I2C_MUTEX_LOCK())
    ESP_LOGE(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
  else
  {

    pmu.readIRQ();

    if (pmu.isVbusOverVoltageIRQ())
      ESP_LOGI(TAG, "USB voltage %.2fV too high.", pmu.getVbusVoltage() / 1000);
    if (pmu.isVbusPlugInIRQ())
      ESP_LOGI(TAG, "USB plugged, %.2fV @ %.0mA", pmu.getVbusVoltage() / 1000,
               pmu.getVbusCurrent());
    if (pmu.isVbusRemoveIRQ())
      ESP_LOGI(TAG, "USB unplugged.");

    if (pmu.isBattPlugInIRQ())
      ESP_LOGI(TAG, "Battery is connected.");
    if (pmu.isBattRemoveIRQ())
      ESP_LOGI(TAG, "Battery was removed.");
    if (pmu.isChargingIRQ())
      ESP_LOGI(TAG, "Battery charging.");
    if (pmu.isChargingDoneIRQ())
      ESP_LOGI(TAG, "Battery charging done.");
    if (pmu.isBattTempLowIRQ())
      ESP_LOGI(TAG, "Battery high temperature.");
    if (pmu.isBattTempHighIRQ())
      ESP_LOGI(TAG, "Battery low temperature.");

    if (pmu.isPEKShortPressIRQ())
    {
      ESP_LOGI(TAG, "Power Button --> Short Pressed");
#if (USE_FASTLED)
      LED_sunset();
#endif
    }

    // long press -> shutdown power, can be exited by another longpress
    if (pmu.isPEKLongtPressIRQ())
    {
      ESP_LOGI(TAG, "Power Button --> LONG Press");
      AXP192_power(pmu_power_off); // switch off Lora, GPS, display
      pmu.shutdown();              // switch off device
    }

    pmu.clearIRQ();
    I2C_MUTEX_UNLOCK(); // release i2c bus access
  }
}

void AXP192_init(void)
{

  if (pmu.begin(Wire, AXP192_PRIMARY_ADDRESS) ==
      AXP_FAIL)
    ESP_LOGI(TAG, "AXP192 PMU initialization failed");
  else
  {

    // configure AXP192
    pmu.setDCDC1Voltage(3300);              // for external OLED display
    pmu.setTimeOutShutdown(false);          // no automatic shutdown
    pmu.setTSmode(AXP_TS_PIN_MODE_DISABLE); // TS pin mode off to save power

    // switch ADCs on
    pmu.adc1Enable(AXP202_BATT_VOL_ADC1, true);
    pmu.adc1Enable(AXP202_BATT_CUR_ADC1, true);
    pmu.adc1Enable(AXP202_VBUS_VOL_ADC1, true);
    pmu.adc1Enable(AXP202_VBUS_CUR_ADC1, true);

    ESP_LOGI(TAG, "CoulombReg: %d", pmu.getCoulombRegister());
    pmu.EnableCoulombcounter();
    ESP_LOGI(TAG, "CoulombReg: %d", pmu.getCoulombRegister());

    //pmu.setChargeControlCur(AXP1XX_CHARGE_CUR_450MA);
    pmu.setChargeControlCur(dataBuffer.settings.bat_max_charge_current);

    pmu.setVoffVoltage(AXP202_VOFF_VOLTAGE33);
    // switch power rails on
    AXP192_power(pmu_power_on);

#if (USE_PMU_INTERRUPT)
#ifdef PMU_INT_PIN
    pinMode(PMU_INT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PMU_INT_PIN), PMU_IRQ, FALLING);
    pmu.enableIRQ(AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ |
                      AXP202_BATT_REMOVED_IRQ | AXP202_BATT_CONNECT_IRQ |
                      AXP202_CHARGING_FINISHED_IRQ | AXP202_PEK_LONGPRESS_IRQ | AXP202_PEK_SHORTPRESS_IRQ,
                  1);
    pmu.clearIRQ();
#endif // PMU_INT
#endif
    ESP_LOGI(TAG, "AXP192 PMU initialized");
  }
}

// helper functions for mutexing i2c access
uint8_t i2c_readBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len)
{
  if (I2C_MUTEX_LOCK())
  {

    uint8_t ret = 0;
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission(false);
    uint8_t cnt = Wire.requestFrom(addr, (uint8_t)len, (uint8_t)1);
    if (!cnt)
      ret = 0xFF;
    uint16_t index = 0;
    while (Wire.available())
    {
      if (index > len)
      {
        ret = 0xFF;
        goto finish;
      }
      data[index++] = Wire.read();
    }

  finish:
    I2C_MUTEX_UNLOCK(); // release i2c bus access
    return ret;
  }
  else
  {
    ESP_LOGE(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
    return 0xFF;
  }
}

uint8_t i2c_writeBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len)
{
  if (I2C_MUTEX_LOCK())
  {

    uint8_t ret = 0;
    Wire.beginTransmission(addr);
    Wire.write(reg);
    for (uint16_t i = 0; i < len; i++)
    {
      Wire.write(data[i]);
    }
    ret = Wire.endTransmission();

    I2C_MUTEX_UNLOCK(); // release i2c bus access
    // return ret ? 0xFF : ret;
    return ret ? ret : 0xFF;
  }
  else
  {
    ESP_LOGE(TAG, "[%0.3f] i2c mutex lock failed", millis() / 1000.0);
    return 0xFF;
  }
}

#endif // HAS_PMU

#ifdef BAT_MEASURE_ADC
esp_adc_cal_characteristics_t *adc_characs =
    (esp_adc_cal_characteristics_t *)calloc(
        1, sizeof(esp_adc_cal_characteristics_t));

#ifndef BAT_MEASURE_ADC_UNIT // ADC1
static const adc1_channel_t adc_channel = BAT_MEASURE_ADC;
#else // ADC2
static const adc2_channel_t adc_channel = BAT_MEASURE_ADC;
#endif
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

#endif // BAT_MEASURE_ADC

void calibrate_voltage(void)
{
#ifdef BAT_MEASURE_ADC
// configure ADC
#ifndef BAT_MEASURE_ADC_UNIT // ADC1
  ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_12));
  ESP_ERROR_CHECK(adc1_config_channel_atten(adc_channel, atten));
#else // ADC2 \
      // ESP_ERROR_CHECK(adc2_config_width(ADC_WIDTH_BIT_12));
  ESP_ERROR_CHECK(adc2_config_channel_atten(adc_channel, atten));
#endif
  // calibrate ADC
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
      unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_characs);
  // show ADC characterization base
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
  {
    ESP_LOGI(TAG,
             "ADC characterization based on Two Point values stored in eFuse");
  }
  else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
  {
    ESP_LOGI(TAG,
             "ADC characterization based on reference voltage stored in eFuse");
  }
  else
  {
    ESP_LOGI(TAG, "ADC characterization based on default reference voltage");
  }
#endif
}

bool batt_sufficient()
{
#if (defined HAS_PMU || defined BAT_MEASURE_ADC)
  uint16_t volts = read_voltage();
  return ((volts < 1000) ||
          (volts > 100)); // no battery or battery sufficient
#else
  return true;
#endif
}

uint16_t read_voltage()
{
  uint16_t voltage = 0;

#if (HAS_PMU)
  voltage = pmu.isVBUSPlug() ? pmu.getVbusVoltage() : pmu.getBattVoltage();
#else

#ifdef BAT_MEASURE_ADC
  // multisample ADC
  uint32_t adc_reading = 0;
#ifndef BAT_MEASURE_ADC_UNIT // ADC1
  for (int i = 0; i < NO_OF_SAMPLES; i++)
  {
    adc_reading += adc1_get_raw(adc_channel);
  }
#else                        // ADC2
  int adc_buf = 0;
  for (int i = 0; i < NO_OF_SAMPLES; i++)
  {
    ESP_ERROR_CHECK(adc2_get_raw(adc_channel, ADC_WIDTH_BIT_12, &adc_buf));
    adc_reading += adc_buf;
  }
#endif                       // BAT_MEASURE_ADC_UNIT
  adc_reading /= NO_OF_SAMPLES;
  // Convert ADC reading to voltage in mV
  voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_characs);
#endif                       // BAT_MEASURE_ADC

#ifdef BAT_VOLTAGE_DIVIDER
  voltage *= BAT_VOLTAGE_DIVIDER;
#endif // BAT_VOLTAGE_DIVIDER

#endif // HAS_PMU

  return voltage;
}

void esp_set_deep_sleep_minutes(uint32_t value)
{
  uint64_t s_time_us = value * uS_TO_S_FACTOR * 60;
  esp_sleep_enable_timer_wakeup(s_time_us);
  ESP_LOGI(TAG, "Set deep sleep to %i min", dataBuffer.settings.sleep_time);

}

void ESP32_sleep()
{

  esp_set_deep_sleep_minutes(dataBuffer.settings.sleep_time);

#if (USE_BUTTON)
  esp_sleep_enable_ext0_wakeup(BUTTON_PIN, 0); //1 = High, 0 = Low
#endif

#if (HAS_PMU)
  AXP192_power(pmu_power_sleep);
#endif

#if (USE_FASTLED)
  LED_sunset();
  LED_deepSleep();
#endif

  gps.enable_sleep();
  Serial.flush();
  showPage(PAGE_SLEEP);
  ESP_LOGI(TAG, "Deep Sleep started");
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}
