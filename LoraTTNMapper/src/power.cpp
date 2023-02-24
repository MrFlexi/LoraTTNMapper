// Basic config
#include "globals.h"
#include "power.h"

// Local logging tag
static const char TAG[] = __FILE__;

#if (HAS_IP5306)

#define IP5306_REG_SYS_0 0x00
#define IP5306_REG_SYS_1 0x01
#define IP5306_REG_SYS_2 0x02
#define IP5306_REG_CHG_0 0x20
#define IP5306_REG_CHG_1 0x21
#define IP5306_REG_CHG_2 0x22
#define IP5306_REG_CHG_3 0x23
#define IP5306_REG_CHG_4 0x24
#define IP5306_REG_READ_0 0x70
#define IP5306_REG_READ_1 0x71
#define IP5306_REG_READ_2 0x72
#define IP5306_REG_READ_3 0x77
#define IP5306_REG_READ_4 0x78

#define IP5306_GetKeyOffEnabled() ip5306_get_bits(IP5306_REG_SYS_0, 0, 1)
#define IP5306_SetKeyOffEnabled(v) ip5306_set_bits(IP5306_REG_SYS_0, 0, 1, v) // 0:dis,*1:en

#define IP5306_GetBoostOutputEnabled() ip5306_get_bits(IP5306_REG_SYS_0, 1, 1)
#define IP5306_SetBoostOutputEnabled(v) ip5306_set_bits(IP5306_REG_SYS_0, 1, 1, v) //*0:dis,1:en

#define IP5306_GetPowerOnLoadEnabled() ip5306_get_bits(IP5306_REG_SYS_0, 2, 1)
#define IP5306_SetPowerOnLoadEnabled(v) ip5306_set_bits(IP5306_REG_SYS_0, 2, 1, v) // 0:dis,*1:en

#define IP5306_GetChargerEnabled() ip5306_get_bits(IP5306_REG_SYS_0, 4, 1)
#define IP5306_SetChargerEnabled(v) ip5306_set_bits(IP5306_REG_SYS_0, 4, 1, v) // 0:dis,*1:en

#define IP5306_GetBoostEnabled() ip5306_get_bits(IP5306_REG_SYS_0, 5, 1)
#define IP5306_SetBoostEnabled(v) ip5306_set_bits(IP5306_REG_SYS_0, 5, 1, v) // 0:dis,*1:en

#define IP5306_GetLowBatShutdownEnable() ip5306_get_bits(IP5306_REG_SYS_1, 0, 1)
#define IP5306_SetLowBatShutdownEnable(v) ip5306_set_bits(IP5306_REG_SYS_1, 0, 1, v) // 0:dis,*1:en

#define IP5306_GetBoostAfterVin() ip5306_get_bits(IP5306_REG_SYS_1, 2, 1)
#define IP5306_SetBoostAfterVin(v) ip5306_set_bits(IP5306_REG_SYS_1, 2, 1, v) // 0:Closed, *1:Open

#define IP5306_GetShortPressBoostSwitchEnable() ip5306_get_bits(IP5306_REG_SYS_1, 5, 1)
#define IP5306_SetShortPressBoostSwitchEnable(v) ip5306_set_bits(IP5306_REG_SYS_1, 5, 1, v) //*0:disabled, 1:enabled

#define IP5306_GetFlashlightClicks() ip5306_get_bits(IP5306_REG_SYS_1, 6, 1)
#define IP5306_SetFlashlightClicks(v) ip5306_set_bits(IP5306_REG_SYS_1, 6, 1, v) //*0:short press twice, 1:long press

#define IP5306_GetBoostOffClicks() ip5306_get_bits(IP5306_REG_SYS_1, 7, 1)
#define IP5306_SetBoostOffClicks(v) ip5306_set_bits(IP5306_REG_SYS_1, 7, 1, v) //*0:long press, 1:short press twice

#define IP5306_GetLightLoadShutdownTime() ip5306_get_bits(IP5306_REG_SYS_2, 2, 2)
#define IP5306_SetLightLoadShutdownTime(v) ip5306_set_bits(IP5306_REG_SYS_2, 2, 2, v) // 0:8s, *1:32s, 2:16s, 3:64s

#define IP5306_GetLongPressTime() ip5306_get_bits(IP5306_REG_SYS_2, 4, 1)
#define IP5306_SetLongPressTime(v) ip5306_set_bits(IP5306_REG_SYS_2, 4, 1, v) //*0:2s, 1:3s

#define IP5306_GetChargingFullStopVoltage() ip5306_get_bits(IP5306_REG_CHG_0, 0, 2)
#define IP5306_SetChargingFullStopVoltage(v) ip5306_set_bits(IP5306_REG_CHG_0, 0, 2, v) // 0:4.14V, *1:4.17V, 2:4.185V, 3:4.2V (values are for charge cutoff voltage 4.2V, 0 or 1 is recommended)

#define IP5306_GetChargeUnderVoltageLoop() ip5306_get_bits(IP5306_REG_CHG_1, 2, 3)     // Automatically adjust the charging current when the voltage of VOUT is greater than the set value
#define IP5306_SetChargeUnderVoltageLoop(v) ip5306_set_bits(IP5306_REG_CHG_1, 2, 3, v) // Vout=4.45V + (v * 0.05V) (default 4.55V) //When charging at the maximum current, the charge is less than the set value. Slowly reducing the charging current to maintain this voltage

#define IP5306_GetEndChargeCurrentDetection() ip5306_get_bits(IP5306_REG_CHG_1, 6, 2)
#define IP5306_SetEndChargeCurrentDetection(v) ip5306_set_bits(IP5306_REG_CHG_1, 6, 2, v) // 0:200mA, 1:400mA, *2:500mA, 3:600mA

#define IP5306_GetVoltagePressure() ip5306_get_bits(IP5306_REG_CHG_2, 0, 2)
#define IP5306_SetVoltagePressure(v) ip5306_set_bits(IP5306_REG_CHG_2, 0, 2, v) // 0:none, 1:14mV, *2:28mV, 3:42mV (28mV recommended for 4.2V)

#define IP5306_GetChargeCutoffVoltage() ip5306_get_bits(IP5306_REG_CHG_2, 2, 2)
#define IP5306_SetChargeCutoffVoltage(v) ip5306_set_bits(IP5306_REG_CHG_2, 2, 2, v) //*0:4.2V, 1:4.3V, 2:4.35V, 3:4.4V

#define IP5306_GetChargeCCLoop() ip5306_get_bits(IP5306_REG_CHG_3, 5, 1)
#define IP5306_SetChargeCCLoop(v) ip5306_set_bits(IP5306_REG_CHG_3, 5, 1, v) // 0:BAT, *1:VIN

#define IP5306_GetVinCurrent() ip5306_get_bits(IP5306_REG_CHG_4, 0, 5)
#define IP5306_SetVinCurrent(v) ip5306_set_bits(IP5306_REG_CHG_4, 0, 5, v) // ImA=(v*100)+50 (default 2250mA)

#define IP5306_GetShortPressDetected() ip5306_get_bits(IP5306_REG_READ_3, 0, 1)
#define IP5306_ClearShortPressDetected() ip5306_set_bits(IP5306_REG_READ_3, 0, 1, 1)

#define IP5306_GetLongPressDetected() ip5306_get_bits(IP5306_REG_READ_3, 1, 1)
#define IP5306_ClearLongPressDetected() ip5306_set_bits(IP5306_REG_READ_3, 1, 1, 1)

#define IP5306_GetDoubleClickDetected() ip5306_get_bits(IP5306_REG_READ_3, 2, 1)
#define IP5306_ClearDoubleClickDetected() ip5306_set_bits(IP5306_REG_READ_3, 2, 1, 1)

#define IP5306_GetPowerSource() ip5306_get_bits(IP5306_REG_READ_0, 3, 1)           // 0:BAT, 1:VIN
#define IP5306_GetBatteryFull() ip5306_get_bits(IP5306_REG_READ_1, 3, 1)           // 0:CHG/DIS, 1:FULL
#define IP5306_GetOutputLoad() ip5306_get_bits(IP5306_REG_READ_2, 2, 1)            // 0:heavy, 1:light
#define IP5306_GetLevelLeds() ((~ip5306_get_bits(IP5306_REG_READ_4, 4, 4)) & 0x0F) // LED[0-4] State (inverted)

#define IP5306_LEDS2PCT(byte) \
  ((byte & 0x01 ? 25 : 0) +   \
   (byte & 0x02 ? 25 : 0) +   \
   (byte & 0x04 ? 25 : 0) +   \
   (byte & 0x08 ? 25 : 0))

int ip5306_get_reg(uint8_t reg)
{
  Wire.beginTransmission(0x75);
  Wire.write(reg);
  if (Wire.endTransmission(false) == 0 && Wire.requestFrom(0x75, 1))
  {
    return Wire.read();
  }
  return -1;
}

int ip5306_set_reg(uint8_t reg, uint8_t value)
{
  Wire.beginTransmission(0x75);
  Wire.write(reg);
  Wire.write(value);
  if (Wire.endTransmission(true) == 0)
  {
    return 0;
  }
  return -1;
}

uint8_t ip5306_get_bits(uint8_t reg, uint8_t index, uint8_t bits)
{
  int value = ip5306_get_reg(reg);
  if (value < 0)
  {
    Serial.printf("ip5306_get_bits fail: 0x%02x\n", reg);
    return 0;
  }
  return (value >> index) & ((1 << bits) - 1);
}

void ip5306_set_bits(uint8_t reg, uint8_t index, uint8_t bits, uint8_t value)
{
  uint8_t mask = (1 << bits) - 1;
  int v = ip5306_get_reg(reg);
  if (v < 0)
  {
    Serial.printf("ip5306_get_reg fail: 0x%02x\n", reg);
    return;
  }
  v &= ~(mask << index);
  v |= ((value & mask) << index);
  if (ip5306_set_reg(reg, v))
  {
    Serial.printf("ip5306_set_bits fail: 0x%02x\n", reg);
  }
}

void printIP5306Stats()
{
  bool usb = IP5306_GetPowerSource();
  bool full = IP5306_GetBatteryFull();
  uint8_t leds = IP5306_GetLevelLeds();
  Serial.printf("IP5306: Power Source: %s, Battery State: %s, Battery Available: %u%%\n", usb ? "USB" : "BATTERY", full ? "CHARGED" : (usb ? "CHARGING" : "DISCHARGING"), IP5306_LEDS2PCT(leds));
}

void printIP5306Settings()
{
  Serial.println("IP5306 Settings:");
  Serial.printf("  KeyOff: %s\n", IP5306_GetKeyOffEnabled() ? "Enabled" : "Disabled");
  Serial.printf("  BoostOutput: %s\n", IP5306_GetBoostOutputEnabled() ? "Enabled" : "Disabled");
  Serial.printf("  PowerOnLoad: %s\n", IP5306_GetPowerOnLoadEnabled() ? "Enabled" : "Disabled");
  Serial.printf("  Charger: %s\n", IP5306_GetChargerEnabled() ? "Enabled" : "Disabled");
  Serial.printf("  Boost: %s\n", IP5306_GetBoostEnabled() ? "Enabled" : "Disabled");
  Serial.printf("  LowBatShutdown: %s\n", IP5306_GetLowBatShutdownEnable() ? "Enabled" : "Disabled");
  Serial.printf("  ShortPressBoostSwitch: %s\n", IP5306_GetShortPressBoostSwitchEnable() ? "Enabled" : "Disabled");
  Serial.printf("  FlashlightClicks: %s\n", IP5306_GetFlashlightClicks() ? "Long Press" : "Double Press");
  Serial.printf("  BoostOffClicks: %s\n", IP5306_GetBoostOffClicks() ? "Double Press" : "Long Press");
  Serial.printf("  BoostAfterVin: %s\n", IP5306_GetBoostAfterVin() ? "Open" : "Not Open");
  Serial.printf("  LongPressTime: %s\n", IP5306_GetLongPressTime() ? "3s" : "2s");
  Serial.printf("  ChargeUnderVoltageLoop: %.2fV\n", 4.45 + (IP5306_GetChargeUnderVoltageLoop() * 0.05));
  Serial.printf("  ChargeCCLoop: %s\n", IP5306_GetChargeCCLoop() ? "Vin" : "Bat");
  Serial.printf("  VinCurrent: %dmA\n", (IP5306_GetVinCurrent() * 100) + 50);
  Serial.printf("  VoltagePressure: %dmV\n", IP5306_GetVoltagePressure() * 14);
  Serial.printf("  ChargingFullStopVoltage: %u\n", IP5306_GetChargingFullStopVoltage());
  Serial.printf("  LightLoadShutdownTime: %u\n", IP5306_GetLightLoadShutdownTime());
  Serial.printf("  EndChargeCurrentDetection: %u\n", IP5306_GetEndChargeCurrentDetection());
  Serial.printf("  ChargeCutoffVoltage: %u\n", IP5306_GetChargeCutoffVoltage());
  Serial.println();
}

bool setupPowerIP5306()
{
  ESP_LOGI(TAG, "Power Up IP5306");
#define IP5306_ADDR 0X75
#define IP5306_REG_SYS_CTL0 0x00
  bool en = true;
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(IP5306_REG_SYS_CTL0);
  if (en)
    Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  else
    Wire.write(0x35); // 0x37 is default reg value
  Wire.endTransmission();

  printIP5306Stats();
  printIP5306Settings();
  return true;
}
#endif

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
    pmu.setChgLEDMode(AXP20X_LED_BLINK_1HZ);
    ESP_LOGI(TAG, "AXP power ON");
    break;

  case pmu_power_sleep_all:
    pmu.setPowerOutPut(AXP192_DCDC1, AXP202_OFF); // OLED on T-Beam v1.0
    pmu.setPowerOutPut(AXP192_LDO3, AXP202_OFF);  // gps off
    pmu.setPowerOutPut(AXP192_LDO2, AXP202_OFF);  // lora off
                                                  // pmu.setChgLEDMode(AXP20X_LED_BLINK_1HZ);
    pmu.setChgLEDMode(AXP20X_LED_OFF);
    ESP_LOGI(TAG, "AXP power SLEEP");
    break;

  case pmu_power_sleep:
    // we don't cut off DCDC1, because then display blocks i2c bus
    pmu.setPowerOutPut(AXP192_LDO3, AXP202_OFF); // gps off
    pmu.setPowerOutPut(AXP192_LDO2, AXP202_OFF); // lora off
                                                 // pmu.setChgLEDMode(AXP20X_LED_BLINK_1HZ);
    pmu.setChgLEDMode(AXP20X_LED_OFF);
    ESP_LOGI(TAG, "AXP power SLEEP");
    break;

  case pmu_power_off:
    pmu.setPowerOutPut(AXP192_DCDC1, AXP202_OFF);
    pmu.setPowerOutPut(AXP192_LDO3, AXP202_OFF);
    pmu.setPowerOutPut(AXP192_LDO2, AXP202_OFF);
    pmu.setChgLEDMode(AXP20X_LED_OFF);
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

    pmu.setChargeControlCur(AXP1XX_CHARGE_CUR_450MA);
    // pmu.setChargeControlCur(dataBuffer.settings.bat_max_charge_current);

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
  esp_sleep_enable_ext0_wakeup(BUTTON_PIN, 0); // 1 = High, 0 = Low
#endif

#if (HAS_PMU)
#if (PMU_SLEEP_ALL_OFF)
  AXP192_power(pmu_power_sleep_all);
#else
  AXP192_power(pmu_power_sleep); // Keep Display connected to DCDC1 on
#endif
#endif

#if (USE_FASTLED)
  LED_sunset();
  LED_deepSleep();
#endif

  gps.enable_sleep();
  Serial.flush();
#if (USE_WIFI)
  WiFi.disconnect();
#endif
  showPage(PAGE_SLEEP);
  ESP_LOGI(TAG, "Deep Sleep started");

  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}
