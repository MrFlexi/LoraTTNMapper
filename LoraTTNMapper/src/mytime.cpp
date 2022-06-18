#include <Arduino.h>
#include "globals.h"
#include "time.h"
#include <sys/time.h>

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  dataBuffer.data.timeinfo = timeinfo;
  dataBuffer.data.timeinfo.tm_year = dataBuffer.data.timeinfo.tm_year + 1900;
  dataBuffer.data.timeinfo.tm_mon = dataBuffer.data.timeinfo.tm_mon + 1;
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  //Serial.print("Day of week: ");
  //Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  //Serial.print("Hour (12 hour format): ");
  //Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  //Serial.print("Second: ");
  //Serial.println(&timeinfo, "%S");

  //Serial.println("Time variables");
  //char timeHour[3];
  //strftime(timeHour, 3, "%H", &timeinfo);
  //Serial.println(timeHour);
  //char timeWeekDay[10];
  //strftime(timeWeekDay, 10, "%A", &timeinfo);
  //Serial.println(timeWeekDay);
  //Serial.println();
}

void set_time_from_gps()
{
  struct tm t;
  time_t t_of_day;
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo))
  {
    if (gps.checkGpsFix())
    {
      t.tm_year = gps.tGps.date.year() - 1900; // Year - 1900
      t.tm_mon = gps.tGps.date.month() - 1;    // Month, where 0 = jan
      t.tm_mday = gps.tGps.date.day();         // Day of the month
      t.tm_hour = gps.tGps.time.hour() + 2;
      t.tm_min = gps.tGps.time.minute();
      t.tm_sec = gps.tGps.time.second();
      t.tm_isdst = 1; // Is DST on? 1 = yes, 0 = no, -1 = unknown
      t_of_day = mktime(&t);

      struct timeval tv;
      tv.tv_sec = t_of_day; // epoch time (seconds)
      tv.tv_usec = 0;       // microseconds
      settimeofday(&tv, 0);
      ESP_LOGI(TAG, "Received GPS Time");
      printLocalTime();
    }
    else
    {
      ESP_LOGI(TAG, "NO GPS Time");
    }
  }
}

void setup_time()
{
  // Init and get the time if availabe via Internet
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

#if (USE_GPS) // or from GPS Module
  set_time_from_gps();
#endif

  printLocalTime();
}
