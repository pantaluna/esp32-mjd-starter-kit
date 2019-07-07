# ESP32 MJD Bosch BMP280 meteo sensor component
This is component based on ESP-IDF for the ESP32 hardware from Espressif.



## Example ESP-IDF project
my_bmp280_sensor_using_lib

## Shop Product.
GY-BMP280-3.3 High Precision Atmospheric Pressure Sensor Module For Arduino



## Wiring Instructions
### MCU
a. Adafruit HUZZAH32
- Pin #13 = Blue LED on the PCB

b. Wemos Lolin32 Lite
- Pin #22 = Blue LED on the PCB

### Sensor PIN layout
```
1 VCC
2 GND
3 SCL
4 SDA
5 CSB
6 SDO
```

### Sensor for the I2C protocol
- Connect device pin VCC to the MCU pin VCC 3V.
- Connect device pin GND to the MCU pin GND.
- Connect device pin SCLK to a MCU GPIO#21 (Huzzah32 #21 bottomleft)(Lolin32lite #13 bottomleft).
- Connect device pin SDA to a MCU GPIO#17  (Huzzah32 #17 bottomleft-1)(Lolin32lite #15 bottomleft-1).



## Data Sheet
[Go to the _doc directory for more documents and images.]

https://www.bosch-sensortec.com/bst/products/all_products/bmp280



## Sensor FAQ
- OK 3.3V
- Metrics: temperature, pressure (both in high precision).
- Pressure sensor: 300 .. 1100 hPa. Pressure resolution: 0.16 Pa (not hectoPascal!). Altitude precision: 0.16 Pa is equivalent to +- 1 meter difference in altitude.
- Temperature sensor range: -40°C .. +85°C. Temperature resolution 0.01°C.
- Sensor CHIP ID for the board GY-BMP280-3.3: 0x58 \
     @important for clones: the address must be 0x56 or 0x57 or 0x58 else the embedded Bosch driver does not work!
- Dependencies (included): Bosch BMP280_driver (https://github.com/BoschSensortec/BMP280_driver) \
     Revision : 2.0.5 (Pressure and Temperature compensation code revision is 1.1) \
     Date : 2016/07/01
- Supports the I2C protocol (opposed to the timing sensitive 1-Wire protocol of the meteo sensors DHT11 and AM2320 from Aosong). No external pull-up resistor is required.
- Supports the SPI protocol (up to 10 MHz) if the pins are broken out.



## Sensor Power Mode BMP280_FORCED_MODE
- In forced mode, a single measurement is performed according to the selected measurement and filter options.
- When the measurement is finished, the sensor returns to sleep mode and the measurement results can be obtained from the data registers.
- For the next measurement, the forced mode needs to be activated again.
- The Forced Mode is recommended for applications which require low sampling rate or host-based synchronization.
- Recommended minimum reading time interval: 1x / minute.



## Sensor Protocol Selection I2C/SPI
- Default: I2C.
- I2C/SPI interface selection is done based on the status of pin 5 CSB (Chip Select).
- The CSB pin must be connected to VCC to select the I2C interface. @important This is ALREADY DONE on this board GY-BMP280-3.3 by the on-board pull-up resistor, so pin 5 can be left disconnected when using the I2C interface.
- If CSB is connected to GND (pulled down) then the SPI interface is activated.



## Sensor I2C Address
- Default device addres = 0x76
- Use the project my_i2c_scanner to detect the actual I2C address :)
- By default the SDO pin (#6) is not connected. This implies the I2C address 0x76 (the on-board resistor pulls the SDO pin low -> GND).
- The 7-bit device address is 111011x. The 6 MSB bits (left ones) are fixed.
- The SDO pin (#6) determines the first bit (0th).
    + Connecting pin SDO to GND results in slave address 1110110 (0x76).
    + Connecting pin SDO to VCC results in slave address 1110111 (0x77).
    + PS The SDO pin cannot be left floating; if left floating, the I2C address will be UNDEFINED.
- PS Using 2 sensors simultaneously is NOT supported by this version of the Bosch driver (the software for the newer BME280 sensor does support wiring up multiple sensors but that version is UNSTABLE). 
  BUT you can wire up an extra sensor on the slave address 0x77 and run the programme against that address.



## Sensor I2C protocol
- Sensor acts as a slave.
- Sensor supports the I2C Standard, Fast and High Speed modes. (standard mode: 100 Kbit/s, full speed: 400 Kbit/s, fast mode: 1 Mbit/s, high speed: 3.2 Mbit/s).



## Issues



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

