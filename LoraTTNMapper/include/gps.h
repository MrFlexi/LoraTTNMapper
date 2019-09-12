#ifndef _GPS_H
#define _GPS_H

#include "globals.h"
#include <TinyGPS++.h>
#include <HardwareSerial.h>

class gps_jogi
{

    public:        
        void init();
        bool checkGpsFix();
        void buildPacket(uint8_t txBuffer[9]);
        void encode();
        TinyGPSPlus tGps;

    private:
        uint32_t LatitudeBinary, LongitudeBinary;
        uint16_t altitudeGps;
        uint8_t hdopGps;
        char t[32]; // used to sprintf for Serial output
       
};

#endif

extern gps_jogi gps;