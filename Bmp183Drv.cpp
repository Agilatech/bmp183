/**
 * \file Bmp183Drv.cpp
 *
 *  Created by Scott Erholm on 08/22/16.
 *  Copyright (c) 2016 Agilatech. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "Bmp183Drv.h"

Bmp183Drv::Bmp183Drv(std::string devfile):spibus::SPIDevice(devfile) {

    this->stationAltitude = 0;
    this->operatingMode = BMP183_MODE_ULTRAHIGHRES;
    
    this->activate();
}

Bmp183Drv::Bmp183Drv(std::string devfile, int altitude):spibus::SPIDevice(devfile) {

    this->stationAltitude = altitude;
    this->operatingMode = BMP183_MODE_ULTRAHIGHRES;
    
    this->activate();
}

Bmp183Drv::Bmp183Drv(std::string devfile, int altitude, int operationMode):spibus::SPIDevice(devfile) {
    
    this->stationAltitude = altitude;
    this->operatingMode = (bmp183_mode_t)operationMode;
    
    this->activate();
}

void Bmp183Drv::activate() {
    if (initialize()) {
        this->active = true;
    }
    else {
        std::cerr << name << " did not initialize. " << name << " is inactive" << std::endl;
    }
}

std::string Bmp183Drv::getVersion() {
    return name + " " + version;
}

std::string Bmp183Drv::getDeviceName() {
    return name;
}

std::string Bmp183Drv::getDeviceType() {
    return type;
}

int Bmp183Drv::getNumValues() {
    return numValues;
}

std::string Bmp183Drv::getTypeAtIndex(int index) {
    if ((index < 0) || (index > (numValues - 1))) {
        return "none";
    }
    
    return valueTypes[index];
}

std::string Bmp183Drv::getNameAtIndex(int index) {
    if ((index < 0) || (index > (numValues - 1))) {
        return "none";
    }
    
    return valueNames[index];
}

bool Bmp183Drv::isActive() {
    return this->active;
}

std::string Bmp183Drv::getValueByName(std::string name) {
    
    for (int i = 0; i < numValues; i++) {
        if (name == valueNames[i]) {
            return this->getValueAtIndex(i);
        }
    }
    
    return "none";
}

std::string Bmp183Drv::getValueAtIndex(int index) {
    
    if (!this->active) {
        return "none";
    }

    if (index == 0) {
        return this->readValue0();
    }
    else if (index == 1) {
        return this->readValue1();
    }
    else {
        return "none";
    }
    
}

bool Bmp183Drv::setOperatingMode(int operationMode) {
    if ((operationMode >= BMP183_MODE_ULTRALOWPOWER) && (operationMode <= BMP183_MODE_ULTRAHIGHRES)) {
        this->operatingMode = (bmp183_mode_t)operationMode;
        return true;
    }
    else {
        return false;
    }
}

bool Bmp183Drv::initialize() {
    
    setSpeed(5000000);
    setMode(spibus::SPIDevice::MODE3);
    
    // Mode boundary check -- default to ultra high if out of bounds
    if ((this->operatingMode > BMP183_MODE_ULTRAHIGHRES) || (this->operatingMode < 0)) {
        this->operatingMode = BMP183_MODE_ULTRAHIGHRES;
    }
    
    // Make sure we have the right device
    uint8_t id = (uint8_t)readRegister(BMP183_REGISTER_CHIPID);
    
    if (id != 0x55) {
        return false;
    }
    
    readCoefficients();
    
    getPressure();

    return true;
}

std::string Bmp183Drv::readValue0() {
    
    if (!this->active) {
        return "none";
    }
    
    // Get the pressure adjusted for altitude
    float value = this->seaLevelPressure(this->getPressure(), this->stationAltitude);
    
    // If the data is not valid, just return none
    if ((value < 850) || (value > 1090)) {
        return "none";
    }
    
    return DataManip::dataToString(value, 1);
}


std::string Bmp183Drv::readValue1() {
    
    if (!this->active) {
        return "none";
    }
    
    float value = this->getTemperature();
    
    // If the data is not valid, just return none
    if ((value < -50) || (value > 55)) {
        return "none";
    }
    
    return DataManip::dataToString(value, 1);
}


float Bmp183Drv::getPressure(void) {
    int32_t  ut = 0, up = 0, compp = 0;
    int32_t  x1, x2, b5, b6, x3, b3, p;
    uint32_t b4, b7;
    
    /* Get the raw pressure and temperature values */
    ut = readRawTemperature();
    up = readRawPressure();
    
    /* Temperature compensation */
    x1 = (ut - (int32_t)(this->bmp183_coeffs.ac6))*((int32_t)(this->bmp183_coeffs.ac5))/pow(2,15);
    x2 = ((int32_t)(this->bmp183_coeffs.mc*pow(2,11)))/(x1+(int32_t)(this->bmp183_coeffs.md));
    b5 = x1 + x2;
    
    /* Pressure compensation */
    b6 = b5 - 4000;
    x1 = (this->bmp183_coeffs.b2 * ((b6 * b6) >> 12)) >> 11;
    x2 = (this->bmp183_coeffs.ac2 * b6) >> 11;
    x3 = x1 + x2;
    b3 = (((((int32_t) this->bmp183_coeffs.ac1) * 4 + x3) << this->operatingMode) + 2) >> 2;
    x1 = (this->bmp183_coeffs.ac3 * b6) >> 13;
    x2 = (this->bmp183_coeffs.b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (this->bmp183_coeffs.ac4 * (uint32_t) (x3 + 32768)) >> 15;
    b7 = ((uint32_t) (up - b3) * (50000 >> this->operatingMode));
    
    if (b7 < 0x80000000) {
        p = (b7 << 1) / b4;
    }
    else {
        p = (b7 / b4) << 1;
    }
    
    x1 = (p >> 8) * (p >> 8);
    x1 = (x1 * 3038) >> 16;
    x2 = (-7357 * p) >> 16;
    compp = p + ((x1 + x2 + 3791) >> 4);
    
    /* Assign compensated pressure value */
    return compp / 100.0F;
}

float Bmp183Drv::getTemperature(void) {
    int32_t UT, X1, X2, B5;     // following ds convention
    float t;
    
    UT = readRawTemperature();
    
    // step 1
    X1 = (UT - (int32_t)this->bmp183_coeffs.ac6) * ((int32_t)this->bmp183_coeffs.ac5) / pow(2,15);
    X2 = ((int32_t)this->bmp183_coeffs.mc * pow(2,11)) / (X1+(int32_t)this->bmp183_coeffs.md);
    B5 = X1 + X2;
    t = (B5+8)/pow(2,4);
    t /= 10;
    
    return t;
}

float Bmp183Drv::pressureToAltitude(float seaLevel, float atmospheric, float temp) {
    /* Hyposometric formula:                      */
    /*                                            */
    /*     ((P0/P)^(1/5.257) - 1) * (T + 273.15)  */
    /* h = -------------------------------------  */
    /*                   0.0065                   */
    /*                                            */
    /* where: h   = height (in meters)            */
    /*        P0  = sea-level pressure (in hPa)   */
    /*        P   = atmospheric pressure (in hPa) */
    /*        T   = temperature (in �C)           */
    
    return (((float)pow((seaLevel/atmospheric), 0.190223F) - 1.0F)
            * (temp + 273.15F)) / 0.0065F;
}

float Bmp183Drv::seaLevelForAltitude(float altitude, float atmospheric, float temp) {
    /* Hyposometric formula:                      */
    /*                                            */
    /* P0=((((h*0.0065)/(temp + 273.15F))+1)^(^/0.190223F))*P */
    /*                                            */
    /* where: h   = height (in meters)            */
    /*        P0  = sea-level pressure (in hPa)   */
    /*        P   = atmospheric pressure (in hPa) */
    /*        T   = temperature (in �C)           */
    
    return (float)pow((((altitude*0.0065)/(temp + 273.15F))+1), (1.0/0.190223F))*atmospheric;
}

float Bmp183Drv::seaLevelPressure(float pressure_mb, int stationAltitude) {
    return (pressure_mb - 0.3) * pow( (1.0 + ((8.42288 / 100000.0) * (stationAltitude / pow((pressure_mb - 0.3), 0.190284) ) )) , (1.0/0.190284));
}


void Bmp183Drv::readCoefficients(void) {
    this->bmp183_coeffs.ac1 = this->readSigned16(BMP183_REGISTER_CAL_AC1);
    this->bmp183_coeffs.ac2 = this->readSigned16(BMP183_REGISTER_CAL_AC2);
    this->bmp183_coeffs.ac3 = this->readSigned16(BMP183_REGISTER_CAL_AC3);
    this->bmp183_coeffs.ac4 = this->readSigned16(BMP183_REGISTER_CAL_AC4);
    this->bmp183_coeffs.ac5 = this->readSigned16(BMP183_REGISTER_CAL_AC5);
    this->bmp183_coeffs.ac6 = this->readSigned16(BMP183_REGISTER_CAL_AC6);
    this->bmp183_coeffs.b1 = this->readSigned16(BMP183_REGISTER_CAL_B1);
    this->bmp183_coeffs.b2 = this->readSigned16(BMP183_REGISTER_CAL_B2);
    this->bmp183_coeffs.mb = this->readSigned16(BMP183_REGISTER_CAL_MB);
    this->bmp183_coeffs.mc = this->readSigned16(BMP183_REGISTER_CAL_MC);
    this->bmp183_coeffs.md = this->readSigned16(BMP183_REGISTER_CAL_MD);
}

int16_t Bmp183Drv::readRawTemperature() {
    writeRegister(BMP183_REGISTER_CONTROL, BMP183_REGISTER_READTEMPCMD);
    usleep(5000);
    return  this->readUnsigned16(BMP183_REGISTER_TEMPDATA);
}

int32_t Bmp183Drv::readRawPressure() {
    uint8_t  p8;
    uint16_t p16;
    int32_t  p32;
    
    writeRegister(BMP183_REGISTER_CONTROL, BMP183_REGISTER_READPRESSURECMD + (this->operatingMode << 6));
    
    switch(this->operatingMode) {
        case BMP183_MODE_ULTRALOWPOWER:
            usleep(5000);
            break;
        case BMP183_MODE_STANDARD:
            usleep(8000);
            break;
        case BMP183_MODE_HIGHRES:
            usleep(14000);
            break;
        case BMP183_MODE_ULTRAHIGHRES:
        default:
            usleep(26000);
            break;
    }
    
    p16 = this->readUnsigned16(BMP183_REGISTER_PRESSUREDATA);
    p32 = (uint32_t)p16 << 8;
    p8 = readRegister(BMP183_REGISTER_PRESSUREDATA+2);
    p32 += p8;
    p32 >>= (8 - this->operatingMode);
    
    return p32;
}

uint16_t Bmp183Drv::readUnsigned16(uint32_t registerAddress) {
    uint16_t result;
    result = combineRegisters(readRegister(registerAddress), readRegister(registerAddress+1));
    return result;
}

int16_t Bmp183Drv::readSigned16(uint32_t registerAddress) {
    int16_t result;
    result = (int16_t)combineRegisters(readRegister(registerAddress), readRegister(registerAddress+1));
    return result;
}

/**
 * Method to combine two 8-bit registers into a single short, which is 16-bits on the BBB. It shifts
 * the MSB 8-bits to the left and then ORs the result with the LSB.
 * @param msb an unsigned character that contains the most significant byte
 * @param lsb an unsigned character that contains the least significant byte
 */
uint16_t Bmp183Drv::combineRegisters(unsigned char msb, unsigned char lsb) {
    //shift the MSB right by 8 bits and OR with LSB
    return ((uint16_t)msb<<8)|(uint16_t)lsb;
}




