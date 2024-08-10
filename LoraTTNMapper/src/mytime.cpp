#include <Arduino.h>
#include "globals.h"
#include "time.h"
#include <sys/time.h>

static const char TAG[] = "";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

void printLocalTime()
{
  time_t now;
  struct tm timeinfo;

  time(&now);
  localtime_r(&now, &timeinfo);

  //dataBuffer.data.timeinfo = timeinfo;
  //dataBuffer.data.timeinfo.tm_year = dataBuffer.data.timeinfo.tm_year + 1900;
  //dataBuffer.data.timeinfo.tm_mon = dataBuffer.data.timeinfo.tm_mon + 1;
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  //Serial.print("Day of week: ");
  //Serial.println(&timeinfo, "%A");
  //Serial.print("Month: ");
  //Serial.println(&timeinfo, "%B");
  //Serial.print("Day of Month: ");
  //Serial.println(&timeinfo, "%d");
  //Serial.print("Year: ");
  //Serial.println(&timeinfo, "%Y");
  //Serial.print("Hour: ");
  //Serial.println(&timeinfo, "%H");
  //Serial.print("Hour (12 hour format): ");
  //Serial.println(&timeinfo, "%I");
  //Serial.print("Minute: ");
  //Serial.println(&timeinfo, "%M");
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

time_t epochConverter(TinyGPSDate &d, TinyGPSTime &t)
{
  // make the object we'll use
  struct tm timeinfo;
  // if the TinyGPS time is invalid, we'll return past time to make it obvious.
  // Shouldn't really reach this point though because of the isValid check in pps_interrupt
  if (!t.isValid())
  {
    return 0;
  }
  else
  {
    // GPS time is probably valid, let's construct seconds from it
    timeinfo.tm_year = d.year() - 1900; // TM offset is years from 1970, i.e. 2014 is 44.
    timeinfo.tm_mon = d.month() - 1 ;  // Month, where 0 = jan
    timeinfo.tm_mday = d.day();
    timeinfo.tm_hour = t.hour();
    timeinfo.tm_min = t.minute();
    timeinfo.tm_sec = t.second();
    time_t epochtime = mktime(&timeinfo);

    return epochtime;
  }
  // nothing matched? Shouldn't arrive here.
  return 1320001666;
}

void set_time_from_gps()
{
  struct tm t;
  time_t t_of_day;
  struct tm timeinfo;

  unsigned long age, date, time, chars;
  int year;
  byte month, day, hour, minute, second, hundredths;

  gps.checkGpsFix(); // Get data from GPS Module

  {
    ESP_LOGI(TAG, "GPS hour %d", gps.tGps.time.value());
    ESP_LOGI(TAG, "GPS Year %d", gps.tGps.date.year());
    ESP_LOGI(TAG, "GPS Month %d", gps.tGps.date.month());
    ESP_LOGI(TAG, "GPS Satellites %d", gps.tGps.satellites.value());

    // do some checks.....
    if (!gps.tGps.time.isValid() || !gps.tGps.date.isValid())
      ESP_LOGI(TAG, "NO GPS Time/Date");
    if (!gps.tGps.time.isUpdated() || !gps.tGps.date.isUpdated())
      ESP_LOGI(TAG, "GPS Time/Date not up to date");
    if (gps.tGps.date.year() < 2019)
      ESP_LOGI(TAG, "GPS Year < 2019 ");

        
    t.tm_hour = gps.tGps.time.hour() + 2;
    t.tm_isdst = 1; // Is DST on? 1 = yes, 0 = no, -1 = unknown
    t_of_day = mktime(&t);

    struct timeval tv;
    
    tv.tv_sec = epochConverter(gps.tGps.date, gps.tGps.time);  // epoch time (seconds)
    tv.tv_usec = 0;       // microseconds

    ESP_LOGI(TAG, "GPS epoch %d", tv.tv_sec);

    settimeofday(&tv, 0);
    ESP_LOGI(TAG, "Received GPS Time");
    printLocalTime();
  }
}

void setup_time()
{

   ESP_LOGI(TAG, "-----------  Setup Time from WLAN or GPS   -----------");
  struct tm timeinfo;
  // Init and get the time if availabe via Internet
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

#if (USE_GPS) // or from GPS Module
    if (!getLocalTime(&timeinfo))
  {
    set_time_from_gps();
  }
#endif
}

void handle_time()
{
  struct tm timeinfo;
#if (USE_GPS)
  if (!getLocalTime(&timeinfo))
  {
    set_time_from_gps();
  }
#endif

  if (getLocalTime(&timeinfo))
  {
    dataBuffer.data.timeinfo = timeinfo;
    dataBuffer.data.time.year = timeinfo.tm_year + 1900;
    dataBuffer.data.time.month = timeinfo.tm_mon + 1;
    dataBuffer.data.time.day = timeinfo.tm_mday;
    dataBuffer.data.timeinfo.tm_mon = dataBuffer.data.timeinfo.tm_mon + 1;
    Serial.println(&dataBuffer.data.timeinfo, "%A, %B %d %Y %H:%M:%S");
  }
}
