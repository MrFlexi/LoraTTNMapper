#include "globals.h"

#if (USE_GPS)

HardwareSerial GPSSerial(1);
static const char TAG[] = "";

void Neo6m::init()
{
  GPSSerial.begin(9600, SERIAL_8N1, GPS_TX, GPS_RX);
  GPSSerial.setTimeout(2);
}

void Neo6m::encode()
{
  int data;
  int previousMillis = millis();

  while ((previousMillis + 1000) > millis())
  {
    while (GPSSerial.available())
    {
      char data = GPSSerial.read();
      //Serial.print(data);
      tGps.encode(data);
    }
  }
}

void Neo6m::buildPacket(uint8_t txBuffer[9])
{
  LatitudeBinary = ((tGps.location.lat() + 90) / 180.0) * 16777215;
  LongitudeBinary = ((tGps.location.lng() + 180) / 360.0) * 16777215;

  sprintf(t, "Lat: %f", tGps.location.lat());
  Serial.println(t);

  sprintf(t, "Lng: %f", tGps.location.lng());
  Serial.println(t);

  txBuffer[0] = (LatitudeBinary >> 16) & 0xFF;
  txBuffer[1] = (LatitudeBinary >> 8) & 0xFF;
  txBuffer[2] = LatitudeBinary & 0xFF;

  txBuffer[3] = (LongitudeBinary >> 16) & 0xFF;
  txBuffer[4] = (LongitudeBinary >> 8) & 0xFF;
  txBuffer[5] = LongitudeBinary & 0xFF;

  altitudeGps = tGps.altitude.meters();
  txBuffer[6] = (altitudeGps >> 8) & 0xFF;
  txBuffer[7] = altitudeGps & 0xFF;

  hdopGps = tGps.hdop.value() / 10;
  txBuffer[8] = hdopGps & 0xFF;
}

bool Neo6m::checkGpsFix()
{
  encode();
  if (tGps.location.isValid() &&
      tGps.location.age() < 2000 &&
      tGps.hdop.isValid() &&
      tGps.hdop.value() <= 300 &&
      tGps.hdop.age() < 2000 &&
      tGps.altitude.isValid() &&
      tGps.altitude.age() < 2000)
  {
    //Serial.println("Valid GPS fix.");
    return true;
  }
  else
  {

    //log_display("no GPS fix");
    // sprintf(t, "location valid: %i" , tGps.location.isValid());
    // Serial.println(t);
    // sprintf(t, "location age: %i" , tGps.location.age());
    // Serial.println(t);
    // sprintf(t, "hdop valid: %i" , tGps.hdop.isValid());
    // Serial.println(t);
    // sprintf(t, "hdop age: %i" , tGps.hdop.age());
    // Serial.println(t);
    // sprintf(t, "hdop: %i" , tGps.hdop.value());
    // Serial.println(t);
    // sprintf(t, "altitude valid: %i" , tGps.altitude.isValid());
    // Serial.println(t);
    // sprintf(t, "altitude age: %i" , tGps.altitude.age());
    // Serial.println(t);

    return false;
  }
}

  void Neo6m::wakeup() {
    Serial.println("GPS wake up");
    int data = -1;
    do
    {
      for (int i = 0; i < 20; i++)
      { //send random to trigger respose
        GPSSerial.write(0xFF);
      }
      data = GPSSerial.read();
    } while (data == -1);
    Serial.println("not sleeping");
  }


  void Neo6m::enable_sleep()
  { //TODO implement  UBX-ACK
    do
    {
      //We cannot read UBX ack therefore try to sleep gps until it does not send data anymore
      Serial.println("try to sleep gps!");
      Neo6m::softwareReset(); //sleep_mode can only be activated at start up
      delay(600);                //give some time to restart //TODO wait for ack
      GPSSerial.write(RXM_PMREQ, sizeof(RXM_PMREQ));
      unsigned long startTime = millis();
      unsigned long offTime = 1;
      Serial.println(offTime);

      while (millis() - startTime < 1000)
      { //wait for the last command to finish
        int c = GPSSerial.read();
        if (offTime == 1 && c == -1)
        { //check  if empty
          offTime = millis();
        }
        else if (c != -1)
        {
          offTime = 1;
        }
        if (offTime != 1 && millis() - offTime > 100)
        { //if gps chip does not send any commands for .. seconds it is sleeping
          Serial.println("sleeping gps!");
          return;
        }
      }
    } while (1);
  }


  void Neo6m::ecoMode()
  {
    GPSSerial.write(CFG_RXM, sizeof(CFG_RST));      // Eco Mode
  }

  void Neo6m::softwareReset()
  {
    GPSSerial.write(CFG_RST, sizeof(CFG_RST));
  }

void Neo6m::getDistance()
{
  // Calculate distance between 2 Points
  if (checkGpsFix()) // IS a GPS Position available
  {
    dataBuffer.data.gps = tGps.location; // Save actual position
    // Build Date/Time
    char sz[32];
    sprintf(dataBuffer.data.gps_datetime, "%02d-%02d-%02d %02d:%02d:%02d ", 
      tGps.date.month(),  tGps.date.day(),  tGps.date.year(), tGps.time.hour(), tGps.time.minute(), tGps.time.second());
    

    if (dataBuffer.data.gps_old.lat() == 0.0) // Store first available position in gps_old for later distance calculation
    {
      dataBuffer.data.gps_old = tGps.location;
    }
  }
  else
  ESP_LOGI(TAG, "GPS Distance NO FIX");


  if (( dataBuffer.data.gps.lat() != 0.0 ) && ( dataBuffer.data.gps_old.lat() != 0.0))
  {
    dataBuffer.data.gps_distance = gps.tGps.distanceBetween(dataBuffer.data.gps.lat(), dataBuffer.data.gps.lng(), dataBuffer.data.gps_old.lat(), dataBuffer.data.gps_old.lng());

  }
  else dataBuffer.data.gps_distance = 0;
  ESP_LOGI(TAG, "GPS Distance %f", dataBuffer.data.gps_distance);
  ESP_LOGI(TAG, "GPS Time %s", dataBuffer.data.gps_datetime);
}

void Neo6m::resetDistance()
{

  if (checkGpsFix()) // IS a GPS Position available
  {
    dataBuffer.data.gps = tGps.location; // Save actual position
    dataBuffer.data.gps_old = tGps.location;

  if (( dataBuffer.data.gps.lat() != 0.0 ) && ( dataBuffer.data.gps_old.lat() != 0.0))
  {
    dataBuffer.data.gps_distance = gps.tGps.distanceBetween(dataBuffer.data.gps.lat(), dataBuffer.data.gps.lng(), dataBuffer.data.gps_old.lat(), dataBuffer.data.gps_old.lng());

  }
  else dataBuffer.data.gps_distance = 0;
  ESP_LOGI(TAG, "GPS Distance %f", dataBuffer.data.gps_distance);
  }
  
}
Neo6m gps;

#endif