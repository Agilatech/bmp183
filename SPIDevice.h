/**
 * \file SPIDevice.h
 *
 *  Created by Scott Erholm on 6/14/16.
 *  Copyright (c) 2016 Agilatech. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __SPIDevice__
#define __SPIDevice__

#include <string>
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define HEX(x) std::setw(2) << std::setfill('0') << std::hex << (int)(x)

namespace spibus {

/**
 * @class SPIDevice
 * @brief Generic SPI Device class that can be used to connect to any type of SPI device and read or write to its registers
 */
class SPIDevice {
public:
    /// The SPI Mode
    enum SPIMODE{
        MODE0 = 0,   //!< Low at idle, capture on rising clock edge
        MODE1 = 1,   //!< Low at idle, capture on falling clock edge
        MODE2 = 2,   //!< High at idle, capture on falling clock edge
        MODE3 = 3    //!< High at idle, capture on rising clock edge
    };
    
private:
    std::string filename;

public:
	SPIDevice(std::string devfile);
    virtual int open();
	virtual unsigned char readRegister(uint32_t registerAddress);
	virtual unsigned char* readRegisters(uint32_t number, uint32_t fromAddress=0);
	virtual int writeRegister(uint32_t registerAddress, unsigned char value);
	virtual void debugDumpRegisters(uint32_t number = 0xff);
	virtual int write(unsigned char value);
	virtual int write(unsigned char value[], int length);
	virtual int setSpeed(uint32_t speed);
	virtual int setMode(SPIDevice::SPIMODE mode);
	virtual int setBitsPerWord(uint8_t bits);
	virtual void close();
	virtual ~SPIDevice();

protected:
    int file;
    
private:
	virtual int transfer(unsigned char read[], unsigned char write[], int length);
	SPIMODE mode;
	uint8_t bits;
	uint32_t speed;
	uint16_t delay;
};

} /* namespace spibus */

#endif /* __SPIDevice__ */
