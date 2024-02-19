#ifndef _SHT40_H
#define _SHT40_H

// Includes
#include "Arduino.h"
#include <Wire.h>    // I2C library


#define SHT_I2C_ADDRESS 0x44 // Address of the sensor

// Data Registers
//#define SHT_REG_TEMP_HUM_DATA //Temperature & Humidity
#define SHT_REG_HUM_DATA 0x0401 // Humidity
#define SHT_REG_TEMP_DATA 0x0400 // Temperature

// Config Registers
#define SHT_REG_TEMP_HUM_CONFIG 0x00  // Temperatrue & Humidity

// Commands
#define SHT_NOHEAT_HIGH 0xFD /**< High precision measurement, no heater */
#define SHT_NOHEAT_MED  0xF6 /**< Medium precision measurement, no heater */

#define SHT_HIGHHEAT_1S  0x39 /**< High precision measurement, high heat for 1 sec */
#define SHT_HIGHHEAT_100MS  0x32 /**< High precision measurement, high heat for 0.1 sec */
#define SHT_MEDHEAT_1S 0x2F /**< High precision measurement, med heat for 1 sec */
#define SHT_MEDHEAT_100MS 0x24 /**< High precision measurement, med heat for 0.1 sec */
#define SHT_LOWHEAT_1S 0x1E /**< High precision measurement, low heat for 1 sec */
#define SHT_LOWHEAT_100MS 0x15 /**< High precision measurement, low heat for 0.1 sec */


class SHT40
{
    public:
        //Constructor
        SHT40();

        // Initialization and debugging methods
        bool init(TwoWire *theWire = &Wire);
        bool scanForSHTConnection(bool verbose);
        void printI2cReturnStatus(byte returnStatus, int bytesWritten, const char action[]);

        // Main Write
        //void enableTempHum();

        // Main Read
        uint16_t getTemperature();
        uint16_t getHumidity();
        
        // 8-bit register methods
        //uint16_t readReg8Addr16Data(byte regAddr, int numBytesToRead, String action, bool verbose);
       // void writeReg8Addr16Data(byte regAddr, uint16_t data, String action, bool verbose);
        //bool writeReg8Addr16DataWithProof(byte regAddr, int numBytesToWrite, uint16_t data, String action, bool verbose);
        
        void writeWithBuff(uint8_t *data, int numBytesToWrite, String action, bool verbose);
        
        bool readWithBuffer(uint8_t *buffer, size_t len, bool stop);
        bool readHelper(uint8_t *buffer, size_t len, bool stop);

    private:
        TwoWire *_wire;
};

#endif