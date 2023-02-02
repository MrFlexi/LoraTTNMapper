#pragma once

#include <Arduino.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>

#define DEFAULT_VREF 1100 // tbd: use adc2_vref_to_gpio() for better estimate
#define NO_OF_SAMPLES 64  // we do some multisampling to get better values

#define AXP192_PRIMARY_ADDRESS (0x34)
#define ON true
#define OFF false

uint16_t read_voltage(void);
float read_current(void);
void calibrate_voltage(void);
bool batt_sufficient(void);
void ESP32_sleep();

#if(HAS_IP5306)
bool setupPowerIP5306();
#endif

#if (HAS_PMU)
#include <axp20x.h>
void AXP192_event_handler(void);
void AXP192_power(pmu_power_t powerlevel);
void AXP192_power_gps(bool on);
void AXP192_power_lora(bool on);
void AXP192_init(void);
void AXP192_showstatus(void);
uint8_t i2c_writeBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len);
uint8_t i2c_readBytes(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len);
extern AXP20X_Class pmu;
#endif // HAS_PMU




