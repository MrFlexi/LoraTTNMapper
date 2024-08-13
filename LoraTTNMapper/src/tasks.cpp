#include "globals.h"
#include "tasks.h"

static const char TAG[] = __FILE__;


u_int32_t get_free_spiffsKB()
{
  u_int32_t totalKBytes = u_int32_t(SPIFFS.totalBytes() / 1024);
  u_int32_t usedKBytes = u_int32_t(SPIFFS.usedBytes() / 1024);
  u_int32_t freeKBytes = 0;
  freeKBytes = totalKBytes - usedKBytes;

  Serial.println();
  Serial.println("SPIFF File sistem info.");

  Serial.print("Total space: ");
  Serial.print(totalKBytes);
  Serial.println(" KBytes");

  Serial.print("Free space: ");
  Serial.print(freeKBytes);
  Serial.println(" kBytes");
  Serial.println();
  return freeKBytes;
}


void t_cyclicRtos2m(void *parameter)
{
  // Task bound to core 0, Prio 0 =  very low
  for (;;)
  {
    ESP_LOGI(TAG, "t-Cyclic 2m");
    dataBuffer.data.spiffsfreeKBytes = get_free_spiffsKB();

    vTaskDelay( tcyclic2mRefreshIntervall / portTICK_PERIOD_MS );
  }
}

