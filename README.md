##Node addon for hardware BMP183 sensor

#####This addon should work on any Linux platform, but has only been tested on BBB

###Install

```
npm install @agilatech/bmp183
```
OR
```
git clone https://github.com/Agilatech/bmp183.git
node-gyp configure build
```

###Usage
####Load the module
```
const addon = require('@agilatech/bmp183');
```

####Create an instance using one of three available constructors
Minimally specify the SPI device as an argument
```
// At a minimum, the constructor should contain one arg specifying the device.
// create an instance on the /dev/spidev1.0 serial device file
const bmp183 = new addon.Bmp183('/dev/spidev1.0');
// In practice, if omitted, a default device of /dev/spidev1.0 will be used
```
Unless you're at sea level, specify elevation in a second argument
```
// The second optional constructor argument is station elevation. 
const bmp183 = new addon.Bmp183('/dev/spidev1.0', 1000);
```
Operational mode can be specified in a third argument
```
// Default mode is 3, but can be changed using a 3-arg constructor
const bmp183 = new addon.Bmp183('/dev/spidev1.0', 1000, 2);
// 0 - Ultra Low Power Mode
// 1 - Standard Mode
// 2 - High Resolution Mode (default)
// 3 - Ultra High Resolution Mode 
```
####Get basic device info
```
const name = bmp183.bmp183();  // returns string with name of device
const version = bmp183.deviceVersion(); // returns this software version
const active = bmp183.deviceActive(); // true if active, false if inactive
const numVals =  bmp183.deviceNumValues(); // returns the number of paramters sensed
```
####Get parameter info and values
```
// pressure is at index 0
const paramName0 = bmp183.nameAtIndex(0);
const paramType0 = bmp183.typeAtIndex(0);
const paramVal0  = bmp183.valueAtIndexSync(0);
```
```
// temperature is at index 1
const paramName1 = bmp183.nameAtIndex(1);
const paramType1 = bmp183.typeAtIndex(1);
const paramVal1  = bmp183.valueAtIndexSync(1);
```
####Asynchronous value collection is also available
```
bmp183.valueAtIndex(0, function(err, val) {
  if (err) {
    console.log(err);
  }
  else {
    console.log(`Asynchronous call return: ${val}`);
  }
});
```

###Operation Notes
This driver is specific to the BMP183 pressure and temperature sensor manufactured by Bosch. It will output both 
pressure in hPa (hectopascal, equal to millibar), and temperature in °C.  The measured pressure range is from 
300hPa to 1100hPa, while the measured temperature range is from -50°C to +55°C.

Atmospheric pressure is directly related to elevation (altitude), so this software compensates for elevation and 
reports the result as if the sensor were at sea level. This removes the effect of station elevation on the reported 
pressure. For this reason it is very important to specify the station elevation in the addon constructor.  Failure
to specify elevation will default to 0, thus returning "none" or a wildly inaccurate pressure value for elevations
above sea level.

It is expected that this sensor will be used on the surface of the earth, subjected to the normal variations of
pressure caused by weather and air movements.  As such, any pressure results outside the extreme record variations
encounted on the planet will be discarded as anomalies (1090 mbar < p < 850 mbar).  An anomalous reading is returned
as "none".  Note that failure to supply a valid elevation may result in an anomalous reading, thereby returning
"none" even when the sensor and driver are working properly.

####Operational Mode
There are 4 different modes of operation, offering tradeoffs between power usage and accuracy. The driver defaults 
to the highest resolution, but this can be altered by specifying a different integer value as the 3rd constructor 
argument.  See the Bosch datasheet for full explanation.
* 0 - Ultra Low Power Mode
* 1 - Standard Mode
* 2 - High Resolution Mode
* 3 - Ultra High Resolution Mode

###Dependencies
* node-gyp

###Copyright
Copyright © 2016 Agilatech. All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

