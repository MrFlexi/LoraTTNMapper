//--------------------------------------------------------------------------
// U8G2 Display Setup  Definition
//--------------------------------------------------------------------------

#pragma once

#include <Arduino.h>
#include "globals.h"

#include "esp_camera.h"
#include "SPIFFS.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <TFT_eSPI.h>

String sendPhoto(void);
void setupCam(void);