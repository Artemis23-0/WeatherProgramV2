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

class VCNL4040
{
    public:
        //Constructor
        VCNL4040();

        // Initialization and debugging methods
        static void init(void);
        static bool scanForVCNLConnection(bool verbose);
        static void printI2cReturnStatus(byte returnStatus, int bytesWritten, const char action[]);

        // Main Write
        static bool enableProximity();
        static bool enableWhiteLight();
        static bool enableAmbientLight();

        // Main Read
        static uint16_t getProximity();
        static uint16_t getWhiteLight();
        static uint16_t getAmbientLight();
        
        // 8-bit register methods
        static uint16_t readReg8Addr16Data(byte regAddr, int numBytesToRead, String action, bool verbose);
        static void writeReg8Addr16Data(byte regAddr, uint16_t data, String action, bool verbose);
        static bool writeReg8Addr16DataWithProof(byte regAddr, int numBytesToWrite, uint16_t data, String action, bool verbose);
};

#endif