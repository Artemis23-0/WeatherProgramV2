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
        enableTempHum();
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

void SHT40::enableTempHum() {
    uint8_t command = SHT_NOHEAT_MED;
    SHT40::writeReg8Addr96DataWithProof(&command, "Enableing Temperature & Humidity", true);
}

///////////////////////////////////////////////////////////////
// Reading
///////////////////////////////////////////////////////////////
// bool SHT40::readWithBuffer(uint8_t *buffer, size_t len, bool stop) {
//     _wire->requestFrom((uint8_t)SHT_I2C_ADDRESS, (uint8_t)len, (uint8_t)stop);

//   for (uint16_t i = 0; i < len; i++) {
//     buffer[i] = _wire->read();
//   }
//   return true;
// }

// // len = 6, for 6 bytes of data
// bool SHT40::readHelper(uint8_t *buffer, size_t len, bool stop) {
//   size_t pos = 0;
//     //For each byte of data, read
//     // read_len = current byte
//   while (pos < len) {
//     size_t read_len = len - pos;
//     bool read_stop = (pos < (len - read_len)) ? false : stop;
//     if (!SHT40::readWithBuffer(buffer + pos, read_len, read_stop))
//       return false;
//     pos += read_len;
//   }
//   return true;
// }

// addr -> command
// 
// uint16_t SHT40::readReg8Addr96Data(uint8_t regAddr, int numBytesToRead, String action, bool verbose) {
    
//     // Attempts a number of retries in case the data is not initially ready
//     int maxRetries = 50;
//     for (int i = 0; i < maxRetries; i++) {
//         //#region Activate with command
//         // Enable I2C connection
//         Wire.beginTransmission(SHT_I2C_ADDRESS);

//         // Prepare and write address - MSB first and then LSB
//         Serial.print("Reading: Writing Bytes at regAddr");
//         int bytesWritten = 0;
//         bytesWritten += Wire.write(regAddr);
//         Serial.println(String(bytesWritten));

//         // End transmission (not writing...reading)
//         byte returnStatus = Wire.endTransmission(false);
//         //#endregion
        
//         if (verbose)
//             SHT40::printI2cReturnStatus(returnStatus, bytesWritten, action.c_str());
//         delay(1100);

//         // THIS IS WHERE THINGS GET FREAKY
//         // Read data from above address
//         Wire.requestFrom(SHT_I2C_ADDRESS, numBytesToRead, true);
        
//         // Grab the data from the data line
//         if (Wire.available() == numBytesToRead) {
//             uint16_t data = 0;
//             uint8_t lsb = Wire.read();
//             uint8_t msb = Wire.read();
//             data = ((uint16_t)msb << 8 | lsb);
          
//             if (verbose)
//                 Serial.printf("\tSTATUS: I2C read at address 0x%02X = 0x%04X (%s)\n", regAddr, data, action.c_str());
//             return data;
//         }
//         delay(10);
//     }
    
//     // Did not succeed after a number of retries
//     Serial.printf("\tERROR: I2C FAILED to read at address 0x%02X (%s)\n", regAddr, action.c_str());
//     return -1;
// }

///////////////////////////////////////////////////////////////
// Writing
///////////////////////////////////////////////////////////////

void SHT40::writeReg8Addr96Data(uint8_t *command, String action, bool verbose) {
    // Enable I2C connection
    Serial.println("Transmitting to SHT");
    _wire->beginTransmission(SHT_I2C_ADDRESS);

    // Prepare and write address, data and end transmission
    Serial.println("Writing Bytes");
    int bytesWritten = _wire->write(command, 1);
    byte returnStatus = _wire->endTransmission();
    if (verbose)
        SHT40::printI2cReturnStatus(returnStatus, bytesWritten, action.c_str());
}

bool SHT40::writeReg8Addr96DataWithProof(uint8_t *command, String action, bool verbose) {
    
    uint8_t readbufferIntial[6];
    uint8_t readbufferAfter[6];
    // Read existing value
    Serial.println("Reading");
    SHT40::readHelper(readbufferIntial, 6, true);
    delay(10);
    
    // Retry limit
    int maxRetries = 200;
    Serial.println("Writing after reading");
    // Attempt data write and then read to ensure written properly
    for (int i = 0; i < maxRetries; i++) {
        SHT40::writeReg8Addr96Data(command, action, false);
        SHT40::readHelper(readbufferAfter, 6, true);
        if (readbufferIntial != readbufferAfter) {
            if (verbose)
                Serial.printf("\tSTATUS: I2C updated REG[0x%02X]: 0x%04X ==> 0x%04X (%s)\n", SHT_I2C_ADDRESS, readbufferIntial, readbufferAfter, action.c_str());
            return true;
        }
        delay(10);
    }

	// Print error message and return status
    Serial.printf("\tERROR: I2C FAILED to write REG[0x%02X]: 0x%04X =X=> 0x%04X (%s)\n", SHT_I2C_ADDRESS, readbufferIntial, command, action.c_str());
    return false;
}

// uint16_t SHT40::getTemperature() {

//     uint8_t readbuffer[6];
//     uint8_t cmd = SHT_NOHEAT_MED;

//     SHT40::writeReg8Addr96Data(&cmd, "Getting Temperature Data", true);
//     delay(5);
//     SHT40::readHelper(readbuffer, 6, true);

//     float t_ticks = (uint16_t)readbuffer[0] * 256 + (uint16_t)readbuffer[1];

//   return -45 + 175 * t_ticks / 65535;
// }
bool SHT40::readWithBuffer(uint8_t *buffer, size_t len, bool stop) {
    _wire->requestFrom((uint8_t)SHT_I2C_ADDRESS, (uint8_t)len, (uint8_t)stop);

  for (uint16_t i = 0; i < len; i++) {
    buffer[i] = _wire->read();
  }
  return true;
}

// len = 6, for 6 bytes of data
bool SHT40::readHelper(uint8_t *buffer, size_t len, bool stop) {
  size_t pos = 0;
    //For each byte of data, read
    // read_len = current byte
  while (pos < len) {
    size_t read_len = len - pos;
    bool read_stop = (pos < (len - read_len)) ? false : stop;
    if (!SHT40::readWithBuffer(buffer + pos, read_len, read_stop))
      return false;
    pos += read_len;
  }
  return true;
}
uint16_t SHT40::getTemperature() {
    uint8_t cmd = SHT_NOHEAT_MED;

// Attempts a number of retries in case the data is not initially ready
    int maxRetries = 50;
    for (int i = 0; i < maxRetries; i++) {

        //#region Activate with command
        SHT40::writeReg8Addr96Data(&cmd, "Gettng Temp Data", false);
        //#endregion
        delay(1100);

        // THIS IS WHERE THINGS GET FREAKY
        // Read data from above address
        _wire->requestFrom((uint8_t)SHT_I2C_ADDRESS, (uint8_t)6);
        //Wire.requestFrom(SHT_I2C_ADDRESS, 6, true);
        
        // Grab the data from the data line
        if (Wire.available() == 6) {
            Serial.print("Got inside");
            float data = 0;
            uint8_t msb = Wire.read();
            uint8_t lsb = Wire.read();
            
            uint8_t crc1 = Wire.read();
            uint8_t msb2 = Wire.read();
            uint8_t lsb2 = Wire.read();
            uint8_t crc2 = Wire.read();

            data = (float) ((uint16_t)msb2 << 8 | lsb2);
            return min(max((-6 + 125 * data / 65535), (float)0.0), (float)100.0);
        }
        delay(10);
    }
    return -1;
}

uint16_t SHT40::getHumidity() {
    uint8_t readbuffer[6];
    uint8_t cmd = SHT_NOHEAT_MED;
    
    SHT40::writeReg8Addr96Data(&cmd, "Getting Humidity Data", true);
    delay(5);
    SHT40::readHelper(readbuffer, 6, true);

  float rh_ticks = (uint16_t)readbuffer[3] * 256 + (uint16_t)readbuffer[4];

  return min(max((-6 + 125 * rh_ticks / 65535), (float)0.0), (float)100.0);
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


*/