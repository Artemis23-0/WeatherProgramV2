#ifndef _SHT40_H
#define _SHT40_H

// Includes
#include "Arduino.h"
#include <Wire.h>    // I2C library


#define SHT_I2C_ADDRESS 0x44 // Address of the sensor

// Commands
#define SHT_NOHEAT_MED  0xF6 // Medium precision measurement, no heater


class SHT40
{
    public:
        //Constructor
        SHT40();

        // Initialization and debugging methods
        bool init(TwoWire *theWire = &Wire);
        bool scanForSHTConnection(bool verbose);
        void printI2cReturnStatus(byte returnStatus, int bytesWritten, const char action[]);

        // Main Read
        bool update();
        uint16_t getTemperature();
        uint16_t getHumidity();

    private:
        TwoWire *_wire;
        uint16_t _temperature;
        uint16_t _humidity;
};

#endif