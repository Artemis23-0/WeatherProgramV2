#include "../include/SHT40.h"

///////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////
int const PIN_SDA = 32;
int const PIN_SCL = 33;

///////////////////////////////////////////////////////////////
// Constructors and initializers
///////////////////////////////////////////////////////////////
SHT40::SHT40() {}

bool SHT40::init(TwoWire *theWire) {
    _wire = theWire;
    _wire->begin();
    
    bool connectionFound = false;
    int tries = 0;

    // See if the device is connected
    while (!connectionFound && tries < 100) {
        connectionFound = SHT40::scanForSHTConnection(true);
        tries = tries + 1;
    }

    if (connectionFound) {
        return true;
    } else {
        return false;
    }
}

bool SHT40::scanForSHTConnection(bool verbose) {
    byte error;

    _wire->beginTransmission(SHT_REG_TEMP_HUM_CONFIG);
    error = _wire->endTransmission();
    if (error == 0) {
        char message[20];
        SHT40::printI2cReturnStatus(error, 0, message);
        return true;
    } else if (verbose) {
        char message[20];
        SHT40::printI2cReturnStatus(error, 2, message);
        return false;
    }
    return false;
}

// void SHT40::enableTempHum() {
//     SHT40::writeReg8Addr16DataWithProof(SHT_REG_TEMP_HUM_CONFIG, 2, 0x00, "Enableing Temperature & Humidity", true);
// }

// void SHT40::writeReg8Addr16Data(byte regAddr, uint16_t data, String action, bool verbose) {
//     // Enable I2C connection
//     Serial.println("Transmitting");
//     Wire.beginTransmission(SHT_I2C_ADDRESS);

//     // Prepare and write address, data and end transmission
//     Serial.println("Writing Bytes");
//     int bytesWritten = Wire.write(regAddr);
//     bytesWritten += Wire.write(data & 0xFF);  // Write LSB
//     bytesWritten += Wire.write(data >> 8);  // Write MSB
//     byte returnStatus = Wire.endTransmission(); // (to send all data)
//     if (verbose)
//         SHT40::printI2cReturnStatus(returnStatus, bytesWritten, action.c_str());
// }

void SHT40::writeWithBuff(uint8_t *data, int numBytesToWrite, String action, bool verbose) {
    // Enable I2C connection
    _wire->beginTransmission(SHT_I2C_ADDRESS);

    // Prepare and write address, data and end transmission
    int bytesWritten = _wire->write(data, numBytesToWrite);
    byte returnStatus = Wire.endTransmission(); // (to send all data)
    if (verbose)
        SHT40::printI2cReturnStatus(returnStatus, bytesWritten, action.c_str());
}

// bool SHT40::writeReg8Addr16DataWithProof(byte regAddr, int numBytesToWrite, uint16_t data, String action, bool verbose) {
    
//     // Read existing value
//     Serial.println("Reading");
//     uint16_t existingData = SHT40::readReg8Addr16Data(regAddr, numBytesToWrite, action, false);
//     delay(10);
    
//     // Retry limit
//     int maxRetries = 200;
//     Serial.println("Writing after reading");
//     // Attempt data write and then read to ensure written properly
//     for (int i = 0; i < maxRetries; i++) {
//         SHT40::writeReg8Addr16Data(regAddr, data, action, false);
//         uint16_t dataRead = SHT40::readReg8Addr16Data(regAddr, numBytesToWrite, action, false);
//         if (dataRead == data) {
//             if (verbose)
//                 Serial.printf("\tSTATUS: I2C updated REG[0x%02X]: 0x%04X ==> 0x%04X (%s)\n", regAddr, existingData, dataRead, action.c_str());
//             return true;
//         }
//         delay(10);
//     }

// 	// Print error message and return status
//     Serial.printf("\tERROR: I2C FAILED to write REG[0x%02X]: 0x%04X =X=> 0x%04X (%s)\n", regAddr, existingData, data, action.c_str());
//     return false;
// }

uint16_t SHT40::getTemperature() {

    uint8_t readbuffer[6];
    uint8_t cmd = SHT_NOHEAT_MED;
    uint16_t duration = 5;

    SHT40::writeWithBuff(&cmd, 1, "Getting Temperature Data", false);
    delay(duration);
    SHT40::readHelper(readbuffer, 6, true);

  float t_ticks = (uint16_t)readbuffer[0] * 256 + (uint16_t)readbuffer[1];

  return -45 + 175 * t_ticks / 65535;
}

uint16_t SHT40::getHumidity() {
    uint8_t readbuffer[6];
    uint8_t cmd = SHT_NOHEAT_MED;
    uint16_t duration = 5;

    SHT40::writeWithBuff(&cmd, 1, "Getting Humidity Data", false);
    delay(duration);
    SHT40::readHelper(readbuffer, 6, true);

  float rh_ticks = (uint16_t)readbuffer[3] * 256 + (uint16_t)readbuffer[4];

  return min(max((-6 + 125 * rh_ticks / 65535), (float)0.0), (float)100.0);
}

bool SHT40::readWithBuffer(uint8_t *buffer, size_t len, bool stop) {
    _wire->requestFrom((uint8_t)SHT_I2C_ADDRESS, (uint8_t)len, (uint8_t)stop);

  for (uint16_t i = 0; i < len; i++) {
    buffer[i] = _wire->read();
  }
  return true;
}

bool SHT40::readHelper(uint8_t *buffer, size_t len, bool stop) {
  size_t pos = 0;
  while (pos < len) {
    size_t read_len =
        ((len - pos) > 30) ? 30 : (len - pos);
    bool read_stop = (pos < (len - read_len)) ? false : stop;
    if (!SHT40::readWithBuffer(buffer + pos, read_len, read_stop))
      return false;
    pos += read_len;
  }
  return true;
}

///////////////////////////////////////////////////////////////
// Helpers
///////////////////////////////////////////////////////////////
void SHT40::printI2cReturnStatus(byte returnStatus, int bytesWritten, const char action[]) {
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

/*OLD OLD OLD*/
/*

uint16_t SHT40::readReg8Addr16Data(uint8_t regAddr, int numBytesToRead, String action, bool verbose) {
    
    // Attempts a number of retries in case the data is not initially ready
    int maxRetries = 50;
    for (int i = 0; i < maxRetries; i++) {
        // Enable I2C connection
        Wire.beginTransmission(SHT_I2C_ADDRESS);

        // Prepare and write address - MSB first and then LSB
        Serial.print("Reading: Writing Bytes at regAddr");
        int bytesWritten = 0;
        bytesWritten += Wire.write(regAddr);
        Serial.println(String(bytesWritten));

        // End transmission (not writing...reading)
        byte returnStatus = Wire.endTransmission(false);
        
        if (verbose)
            SHT40::printI2cReturnStatus(returnStatus, bytesWritten, action.c_str());
        delay(1100);
        // Read data from above address
        Serial.println("you fucked up");
        Wire.requestFrom(SHT_I2C_ADDRESS, numBytesToRead);
        
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
*/