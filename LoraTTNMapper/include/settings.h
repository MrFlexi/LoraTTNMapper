#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <Effortless_SPIFFS.h>

void load_settings();
void save_settings();

void saveConfiguration();
void loadConfiguration();