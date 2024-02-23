#include "../include/SHT40.h"

// https://sensirion.com/media/documents/33FD6951/6555C40E/Sensirion_Datasheet_SHT4x.pdf

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

    _wire->beginTransmission(SHT_I2C_ADDRESS);
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

///////////////////////////////////////////////////////////////
// Reading
///////////////////////////////////////////////////////////////
bool SHT40::update() {
    uint8_t command = SHT_NOHEAT_MED;

// Attempts a number of retries in case the data is not initially ready
    int maxRetries = 50;
    for (int i = 0; i < maxRetries; i++) {
        // Activate Line
        _wire->beginTransmission(SHT_I2C_ADDRESS);

        // Prepare and write address, data and end transmission
        Serial.println("Writing Bytes");
        int bytesWritten = _wire->write(command);
        byte returnStatus = _wire->endTransmission();

        delay(10);

        // Read data from above address
        _wire->requestFrom((uint8_t)SHT_I2C_ADDRESS, (uint8_t)6);
        
        // Grab the data from the data line
        if (_wire->available() == 6) {
            float tempData = 0;
            float humData = 0;

            // Temp Data
            uint8_t msb = _wire->read();
            uint8_t lsb = _wire->read();
            uint8_t crc1 = _wire->read(); // Don't care about this

            // Humidity Data
            uint8_t msb2 = _wire->read();
            uint8_t lsb2 = _wire->read();
            uint8_t crc2 = _wire->read(); // Don't care about this

            tempData = (float) ((uint16_t)msb << 8 | lsb);
            humData = (float) ((uint16_t)msb2 << 8 | lsb2);

            _temperature = -45 + 175 * tempData / 65535;
            _humidity = min(max((-6 + 125 * humData / 65535), (float)0.0), (float)100.0);
            return true;;
        }
    }
    return false;
}

uint16_t SHT40::getTemperature() {
    return _temperature;
}

uint16_t SHT40::getHumidity() {
    return _humidity;
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