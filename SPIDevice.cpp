
/**
 * \file SPIDevice.cpp
 *
 *  Created by Scott Erholm on 6/14/16.
 *  Copyright (c) 2016 Agilatech. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "SPIDevice.h"

namespace spibus {

    /**
     * The constructor for the SPIDevice that sets up and opens the SPI connection.
     * The destructor will close the SPI file connection.
     * @param devfile On BBB, filename is /dev/spidevB.D where B is bus and D device
     */
    SPIDevice::SPIDevice(std::string devfile) {
        this->file=-1;
        
        this->filename = devfile;
        
        this->mode = SPIDevice::MODE3;
        this->bits = 8;
        this->speed = 500000;
        this->delay = 0;
        
        this->open();
    }


    /**
     * This method opens the file connection to the SPI device.
     * @return 0 on a successful open of the file
     */
    int SPIDevice::open(){
        //cout << "Opening the file: " << filename.c_str() << endl;
        if ((this->file = ::open(filename.c_str(), O_RDWR))<0){
            std::cerr << "SPIDevice: Can't open device " << filename << std::endl;
            return -1;
        }
        if (this->setMode(this->mode)==-1) return -1;
        if (this->setSpeed(this->speed)==-1) return -1;
        if (this->setBitsPerWord(this->bits)==-1) return -1;
        return 0;
    }

    /**
     * Generic method to transfer data to and from the SPI device. It is used by the
     * following methods to read and write registers.
     * @param send The array of data to send to the SPI device
     * @param receive The array of data to receive from the SPI device
     * @param length The length of the array to send
     * @return -1 on failure
     */
    int SPIDevice::transfer(unsigned char send[], unsigned char receive[], int length){
        struct spi_ioc_transfer	transfer;
        transfer.tx_buf = (uint64_t) send;
        transfer.rx_buf = (uint64_t) receive;
        transfer.len = length;
        transfer.speed_hz = this->speed;
        transfer.bits_per_word = this->bits;
        transfer.delay_usecs = this->delay;
        int status = ioctl(this->file, SPI_IOC_MESSAGE(1), &transfer);
        if (status < 0) {
            std::cerr << "SPIDevice: SPI_IOC_MESSAGE Failed" << std::endl;
            return -1;
        }
        return status;
    }

    unsigned char SPIDevice::readRegister(uint32_t registerAddress){
        unsigned char send[2], receive[2];
        memset(send, 0, sizeof send);
        memset(receive, 0, sizeof receive);
        send[0] = (unsigned char) (0x00 | registerAddress);
        this->transfer(send, receive, 2);
        //cout << "The value that was received is: " << (int) receive[1] << endl;
        return receive[1];
    }

        
    unsigned char* SPIDevice::readRegisters(uint32_t number, uint32_t fromAddress){
        unsigned char* data = new unsigned char[number];
        unsigned char send[number+1], receive[number+1];
        memset(send, 0, sizeof send);
        send[0] =  (unsigned char) (0x80 | 0x40 | fromAddress); //set read bit and MB bit
        this->transfer(send, receive, number+1);
        memcpy(data, receive+1, number);  //ignore the first (address) byte in the array returned
        return data;
    }

    int SPIDevice::write(unsigned char value){
        unsigned char null_return = 0x00;
        //printf("[%02x]", value);
        this->transfer(&value, &null_return, 1);
        return 0;
    }

    int SPIDevice::write(unsigned char value[], int length){
        unsigned char null_return = 0x00;
        this->transfer(value, &null_return, length);
        return 0;
    }

    int SPIDevice::writeRegister(uint32_t registerAddress, unsigned char value){
        unsigned char send[2], receive[2];
        memset(receive, 0, sizeof receive);
        send[0] = (unsigned char) registerAddress;
        send[1] = value;
        //cout << "The value that was written is: " << (int) send[1] << endl;
        this->transfer(send, receive, 2);
        return 0;
    }

    void SPIDevice::debugDumpRegisters(uint32_t number){
        std::cerr << "SPIDevice: SPI Mode: " << this->mode << std::endl;
        std::cerr << "SPIDevice: Bits per word: " << (int)this->bits << std::endl;
        std::cerr << "SPIDevice: Max speed: " << this->speed << std::endl;
        std::cerr << "SPIDevice: Dumping Registers for Debug Purposes:" << std::endl;
        unsigned char *registers = this->readRegisters(number);
        
        for(int i=0; i<(int)number; i++){
            std::cerr << HEX(*(registers+i)) << " ";
            if (i%16==15) std::cout << std::endl;
        }
        std::cerr << std::dec;
    }

    int SPIDevice::setSpeed(uint32_t speed){
        this->speed = speed;
        if (ioctl(this->file, SPI_IOC_WR_MAX_SPEED_HZ, &this->speed)==-1){
            std::cerr << "SPIDevice: Can't set max speed HZ" << std::endl;
            return -1;
        }
        if (ioctl(this->file, SPI_IOC_RD_MAX_SPEED_HZ, &this->speed)==-1){
            std::cerr << "SPIDevice: Can't get max speed HZ." << std::endl;
            return -1;
        }
        return 0;
    }

    int SPIDevice::setMode(SPIDevice::SPIMODE mode){
        this->mode = mode;
        if (ioctl(this->file, SPI_IOC_WR_MODE, &this->mode)==-1){
            std::cerr << "SPIDevice: Can't set SPI mode." << std::endl;
            return -1;
        }
        if (ioctl(this->file, SPI_IOC_RD_MODE, &this->mode)==-1){
            std::cerr << "SPIDevice: Can't get SPI mode." << std::endl;
            return -1;
        }
        return 0;
    }

    int SPIDevice::setBitsPerWord(uint8_t bits){
        this->bits = bits;
        if (ioctl(this->file, SPI_IOC_WR_BITS_PER_WORD, &this->bits)==-1){
            std::cerr << "SPIDevice: Can't set bits per word." << std::endl;
            return -1;
        }
        if (ioctl(this->file, SPI_IOC_RD_BITS_PER_WORD, &this->bits)==-1){
            std::cerr << "SPIDevice: Can't get bits per word." << std::endl;
            return -1;
        }
        return 0;
    }

    void SPIDevice::close(){
        ::close(this->file);
        this->file = -1;
    }

    SPIDevice::~SPIDevice() {
        this->close();
    }

} /* namespace spibus */
