#include "../include/VCNL4040.h"

///////////////////////////////////////////////////////////////
// Constructors and initializers
///////////////////////////////////////////////////////////////
VCNL4040::VCNL4040(void) {}

bool VCNL4040::init() {
    // Initialize I2C interface
    Wire.begin(PIN_SDA, PIN_SCL, 400000);

    bool connectionFound = false;
    int tries = 0;

    // See if the device is connected
    while (!connectionFound && tries < 10) {
        connectionFound = VCNL4040::scanForVCNLConnection(true);
        tries = tries + 1;
    }

    if (connectionFound) {
        VCNL4040::enableAmbientLight();
        VCNL4040::enableWhiteLight();
        VCNL4040::enableProximity();
        return true;
    } else {
        Serial.println("Could not connect to VCNL4040");
        return false;
    }
    return false;
}

bool VCNL4040::scanForVCNLConnection(bool verbose) {
    byte error;

    Wire.beginTransmission(VCNL_I2C_ADDRESS);
    error = Wire.endTransmission();
    if (error == 0) {
        char message[20];
        VCNL4040::printI2cReturnStatus(error, 0, message);
        return true;
    } else if (verbose) {
        char message[20];
        VCNL4040::printI2cReturnStatus(error, 2, message);
        return false;
    }
    return false;
}

bool VCNL4040::enableProximity() {
    return VCNL4040::writeReg8Addr16DataWithProof(VCNL_REG_PS_CONFIG, 2, 0x0800, "Enableing Proximity", true);
}

bool VCNL4040::enableWhiteLight() {
    return VCNL4040::writeReg8Addr16DataWithProof(VCNL_REG_WHITE_CONFIG, 2, 0x0000, "Enableing White Light", true);
}

bool VCNL4040::enableAmbientLight() {
    return VCNL4040::writeReg8Addr16DataWithProof(VCNL_REG_ALS_CONFIG, 2, 0x0000, "Enableing Ambient Light", true);
}

void VCNL4040::writeReg8Addr16Data(byte regAddr, uint16_t data, String action, bool verbose) {
    // Enable I2C connection
    Wire.beginTransmission(VCNL_I2C_ADDRESS);

    // Prepare and write address, data and end transmission
    int bytesWritten = Wire.write(regAddr);
    bytesWritten += Wire.write(data & 0xFF);  // Write LSB
    bytesWritten += Wire.write(data >> 8);  // Write MSB
    byte returnStatus = Wire.endTransmission(); // (to send all data)
    if (verbose)
        VCNL4040::printI2cReturnStatus(returnStatus, bytesWritten, action.c_str());
}

bool VCNL4040::writeReg8Addr16DataWithProof(byte regAddr, int numBytesToWrite, uint16_t data, String action, bool verbose) {
    
    // Read existing value
    uint16_t existingData = VCNL4040::readReg8Addr16Data(regAddr, numBytesToWrite, action, false);
    delay(10);
    
    // Retry limit
    int maxRetries = 200;
    
    // Attempt data write and then read to ensure written properly
    for (int i = 0; i < maxRetries; i++) {
        VCNL4040::writeReg8Addr16Data(regAddr, data, action, false);
        uint16_t dataRead = VCNL4040::readReg8Addr16Data(regAddr, numBytesToWrite, action, false);
        if (dataRead == data) {
            if (verbose)
                Serial.printf("\tSTATUS: I2C updated REG[0x%02X]: 0x%04X ==> 0x%04X (%s)\n", regAddr, existingData, dataRead, action.c_str());
            return true;
        }
        delay(10);
    }

	// Print error message and return status
    Serial.printf("\tERROR: I2C FAILED to write REG[0x%02X]: 0x%04X =X=> 0x%04X (%s)\n", regAddr, existingData, data, action.c_str());
    return false;
}

uint16_t VCNL4040::getProximity() {
    return VCNL4040::readReg8Addr16Data(VCNL_REG_PROX_DATA, 2, "Getting Proximity Data", false);
}

uint16_t VCNL4040::getWhiteLight() {
    return 0.1 * VCNL4040::readReg8Addr16Data(VCNL_REG_WHITE_DATA, 2, "Getting White Light Data", false);
}

uint16_t VCNL4040::getAmbientLight() {
    return 0.1 * VCNL4040::readReg8Addr16Data(VCNL_REG_ALS_DATA, 2, "Getting Ambient Light Data", false);
}

uint16_t VCNL4040::readReg8Addr16Data(byte regAddr, int numBytesToRead, String action, bool verbose) {
    
    // Attempts a number of retries in case the data is not initially ready
    int maxRetries = 50;
    for (int i = 0; i < maxRetries; i++) {
        // Enable I2C connection
        Wire.beginTransmission(VCNL_I2C_ADDRESS);

        // Prepare and write address - MSB first and then LSB
        int bytesWritten = 0;
        bytesWritten += Wire.write(regAddr);

        // End transmission (not writing...reading)
        byte returnStatus = Wire.endTransmission(false);
        if (verbose)
            VCNL4040::printI2cReturnStatus(returnStatus, bytesWritten, action.c_str());

        // Read data from above address
        Wire.requestFrom(VCNL_I2C_ADDRESS, numBytesToRead);

        // Grab the data from the data line
        if (Wire.available() == numBytesToRead) {
            uint16_t data = 0;
            uint8_t lsb = Wire.read();
            uint8_t msb = Wire.read();
            data = ((uint16_t)msb << 8 | lsb);
          
            if (verbose)
                Serial.printf("\tSTATUS: I2C read at address 0x%02X = 0x%04X (%s)\n", regAddr, data, action.c_str());
            return data;
        }
        delay(10);
    }
    
    // Did not succeed after a number of retries
    Serial.printf("\tERROR: I2C FAILED to read at address 0x%02X (%s)\n", regAddr, action.c_str());
    return -1;
}

///////////////////////////////////////////////////////////////
// Helpers
///////////////////////////////////////////////////////////////
void VCNL4040::printI2cReturnStatus(byte returnStatus, int bytesWritten, const char action[]) {
    switch (returnStatus) {
        case 0:
            Serial.printf("\tSTATUS (I2C): %d bytes successfully read/written %s\n", bytesWritten, action);
            break;
        case 1:
            Serial.printf("\t***ERROR (I2C): %d bytes failed %s - data too long to fit in transmit buffer***\n", bytesWritten, action);
            break;
        case 2:
            Serial.printf("\t***ERROR (I2C): %d bytes failed %s - received NACK on transmit of address***\n", bytesWritten, action);
            break;
        case 3:
            Serial.printf("\t***ERROR (I2C): %d bytes failed %s - received NACK on transmit of data***\n", bytesWritten, action);
            break;
        default:
            Serial.printf("\t***ERROR (I2C): %d bytes failed %s - unknown error***\n", bytesWritten, action);
            break;
    }
}