/**
 * \file Bmp183Drv.h
 *
 *  Created by Scott Erholm on 08/22/16.
 *  Copyright (c) 2016 Agilatech. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __Bmp183Drv__
#define __Bmp183Drv__

#include <iostream>
#include <fstream>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "SPIDevice.h"
#include "DataManip.h"

#ifdef DEBUG
#  define DPRINT(x) do { std::cerr << x; std::cerr << std::endl; } while (0)
#else
#  define DPRINT(x) do {} while (0)
#endif


static const std::string name = "BMP183";
static const std::string type = "sensor";

static const std::string version = "0.8.0";

static const int numValues = 2;

static const std::string valueNames[numValues] = {"pressure", "temperature"};
static const std::string valueTypes[numValues] = {"float", "float"};

/*=========================================================================
 REGISTERS
 -----------------------------------------------------------------------*/

enum
{
    BMP183_REGISTER_CAL_AC1            = 0xAA,  // R   Calibration data (16 bits)
    BMP183_REGISTER_CAL_AC2            = 0xAC,  // R   Calibration data (16 bits)
    BMP183_REGISTER_CAL_AC3            = 0xAE,  // R   Calibration data (16 bits)
    BMP183_REGISTER_CAL_AC4            = 0xB0,  // R   Calibration data (16 bits)
    BMP183_REGISTER_CAL_AC5            = 0xB2,  // R   Calibration data (16 bits)
    BMP183_REGISTER_CAL_AC6            = 0xB4,  // R   Calibration data (16 bits)
    BMP183_REGISTER_CAL_B1             = 0xB6,  // R   Calibration data (16 bits)
    BMP183_REGISTER_CAL_B2             = 0xB8,  // R   Calibration data (16 bits)
    BMP183_REGISTER_CAL_MB             = 0xBA,  // R   Calibration data (16 bits)
    BMP183_REGISTER_CAL_MC             = 0xBC,  // R   Calibration data (16 bits)
    BMP183_REGISTER_CAL_MD             = 0xBE,  // R   Calibration data (16 bits)
    BMP183_REGISTER_CHIPID             = 0xD0,
    BMP183_REGISTER_VERSION            = 0xD1,
    BMP183_REGISTER_SOFTRESET          = 0xE0,
    BMP183_REGISTER_CONTROL            = 0x74,
    BMP183_REGISTER_TEMPDATA           = 0xF6,
    BMP183_REGISTER_PRESSUREDATA       = 0xF6,
    BMP183_REGISTER_READTEMPCMD        = 0x2E,
    BMP183_REGISTER_READPRESSURECMD    = 0x34
};

/*=========================================================================*/

/*=========================================================================
 MODE SETTINGS
 -----------------------------------------------------------------------*/
typedef enum
{
    BMP183_MODE_ULTRALOWPOWER          = 0,
    BMP183_MODE_STANDARD               = 1,
    BMP183_MODE_HIGHRES                = 2,
    BMP183_MODE_ULTRAHIGHRES           = 3
} bmp183_mode_t;
/*=========================================================================*/

/*=========================================================================
 CALIBRATION DATA
 -----------------------------------------------------------------------*/
typedef struct
{
    int16_t  ac1;
    int16_t  ac2;
    int16_t  ac3;
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t  b1;
    int16_t  b2;
    int16_t  mb;
    int16_t  mc;
    int16_t  md;
} bmp183_calib_data;
/*=========================================================================*/


class Bmp183Drv : public spibus::SPIDevice  {
    
public:
    // Sorry about all the constructors--they're needed because the last two parameters are
    // the same type, so default values are ambiguous here. 
    Bmp183Drv(std::string devfile);
    Bmp183Drv(std::string devfile, int altitude);
    Bmp183Drv(std::string devfile, int altitude, int operationMode);
    
    static std::string getVersion();
    static std::string getDeviceName();
    static std::string getDeviceType();
    static int getNumValues();
    static std::string getTypeAtIndex(int index);
    static std::string getNameAtIndex(int index);
    
    bool isActive();
    std::string getValueByName(std::string name);
    std::string getValueAtIndex(int index);
    bool setOperatingMode(int operationMode);
    
protected:
    
    bool initialize();
    std::string readValue0();
    std::string readValue1();
    
private:
    void activate();
    float  getTemperature();
    float  getPressure();
    float pressureToAltitude(float seaLevel, float atmospheric, float temp);
    float seaLevelForAltitude(float altitude, float atmospheric, float temp);
    float seaLevelPressure(float pressure_mb, int stationAltitude);
    void readCoefficients(void);
    int16_t readRawTemperature();
    int32_t readRawPressure();
    uint16_t readUnsigned16(uint32_t registerAddress);
    int16_t readSigned16(uint32_t registerAddress);
    uint16_t combineRegisters(unsigned char msb, unsigned char lsb);

    bool active = false;
    int stationAltitude;
    bmp183_calib_data bmp183_coeffs;
    bmp183_mode_t operatingMode = BMP183_MODE_ULTRAHIGHRES;
        
};

#endif /* defined(__Bmp183Drv__) */
