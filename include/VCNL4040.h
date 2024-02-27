#ifndef _VCNL4040_H
#define _VCNL4040_H

// Includes
#include "Arduino.h"
#include <Wire.h>    // I2C library

#define VCNL_I2C_ADDRESS 0x60 // Address of the sensor

// Data Registers
#define VCNL_REG_PROX_DATA 0x08 // Proximity
#define VCNL_REG_ALS_DATA 0x09 // Ambient Light
#define VCNL_REG_WHITE_DATA 0x0A // White Light

// Config Registers
#define VCNL_REG_PS_CONFIG 0x03 // Proximity 
#define VCNL_REG_ALS_CONFIG 0x00 // Ambient Light
#define VCNL_REG_WHITE_CONFIG 0x04 // White Light

#define PIN_SDA 32
#define PIN_SCL 33

class VCNL4040
{
    public:
        //Constructor
        VCNL4040();

        // Initialization and debugging methods
        bool init();
        bool scanForVCNLConnection(bool verbose);
        void printI2cReturnStatus(byte returnStatus, int bytesWritten, const char action[]);

        // Main Write
        bool enableProximity();
        bool enableWhiteLight();
        bool enableAmbientLight();

        // Main Read
        uint16_t getProximity();
        uint16_t getWhiteLight();
        uint16_t getAmbientLight();
        
        // 8-bit register methods
        uint16_t readReg8Addr16Data(byte regAddr, int numBytesToRead, String action, bool verbose);
        void writeReg8Addr16Data(byte regAddr, uint16_t data, String action, bool verbose);
        bool writeReg8Addr16DataWithProof(byte regAddr, int numBytesToWrite, uint16_t data, String action, bool verbose);
};

#endif