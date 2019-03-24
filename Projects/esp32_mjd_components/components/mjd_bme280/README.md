# ESP-IDF MJD Bosch BME280 meteo sensor component
This is component based on ESP-IDF for the ESP32 hardware from Espressif.



## Example ESP-IDF project
my_bme280_sensor_using_lib

## Shop Product
- CJMCU-280E BME280 High Precision Atmospheric Pressure Sensor Module (3.3V, 6 pins, I2C and SPI). This is the bigger board. The I2C address **can** be changed. It does not contain a voltage regulator 5V to 3.3V which is a good thing (less power consumption).
- GY-BME280 Temperature Humidity Pressure Sensor Module (3.3 - 5V, 4 pins, only I2C). This is the smaller board. The I2C address **cannot** be changed. It contains a voltage regulator 5V -> 3.3V (not required for an ESP32 dev board) and therefore consumes more power than the bigger board. The smaller form factor might be useful if you have to place the sensor in tiny places.

## Wiring Instructions

### Sensor PIN layout
```
1 VCC
2 GND
3 SCLK
4 SDA
5 CSB (absent on the smaller board which only supports the I2C protocol)
6 SDO (absent on the smaller board which only supports the I2C protocol)
```

### Sensor for the I2C protocol
- Connect device pin VCC to the MCU pin VCC 3V.
- Connect device pin GND to the MCU pin GND.
- Connect device pin SCLK to a MCU GPIO#21 (Huzzah32 #21 bottomleft)(Lolin32lite #13 bottomleft).
- Connect device pin SDA to a MCU GPIO#17 (Huzzah32 #17 bottomleft-1)(Lolin32lite #15 bottomleft-1).



## Data Sheet
[Go to the _doc directory for more documents and images.]

https://www.bosch-sensortec.com/bst/products/all_products/bme280



## Sensor FAQ
- OK 3.3V
- Good: an air temperature sensor, atmospheric pressure sensor and relative humidity sensor (opposed to the older BMP280 which does not feature relative humidity).
- Metrics: humidity, pressure, temperature (all in high precision).
- Humidity sensor: 0 - 100%.
- Pressure sensor: 300 .. 1100 hPa. Pressure resolution: 0.16 Pa (Pascal, not hectoPascal!). Altitude precision: 0.16 Pa is equivalent to +-1 meter difference in altitude.
- Temperature sensor range: -40°C .. +85°C. Temperature resolution 0.01°C.
- These breakouts boards contain 10K pull-up resistors for the SCL and SDA signals. No external pull-up resistor is required. Important: if you have multiple I2C slaves on one I2C Bus and each board has pullup resistors then you might have to unsolder some pullup resistors because the total resistance becomes too high (typically a total of > 100K is too high).
- Supports the I2C protocol (opposed to the timing sensitive 1-Wire protocol of the meteo sensors DHT11 and AM2320 from Aosong).
- Sensor CHIP ID Unique Model ID in Register 0xD0 "id": 0x60 \
    The "id" register contains the chip identification number chip_id [bit7:0], which is 0x60. \
    This number can be read as soon as the device finishes the power-on-reset.
- The V1 supports the SPI protocol (up to 10 MHz).



## Sensor Power Mode BME280_FORCED_MODE
- In forced mode, a single measurement is performed according to the selected measurement and filter options.
- When the measurement is finished, the sensor returns to sleep mode and the measurement results can be obtained from the data registers. The sensor has a minimum response time; it varies with the configuration (oversampling rates etc).
- For the next measurement, the forced mode needs to be activated again.
- Forced mode is recommended for applications which require low sampling rate or host-based synchronization.
- Recommended minimum reading time interval: 1x / minute.



## Sensor Protocol Selection I2C/SPI
This section is not relevant for the smaller board because it only supports the I2C protocol.

- Default: I2C.
- I2C/SPI interface selection is done based on the status of pin 5 CSB (Chip Select).
- The CSB pin must be connected to VCC to select the I2C interface. @important This is already wired up on the bigger board GY-B11 by the on-board pull-up resistor, so pin 5 can be left disconnected when using the I2C interface.
- If CSB is connected to GND (pulled down) then the SPI interface is activated.



## Sensor I2C Address
Use the project esp32_i2c_scanner to detect the actual I2C address.

This section is not relevant for the smaller board because its I2C address cannot be changed.

- I2C Address: 0x76.
- By default the SDO pin (#6) is not connected. This implies the I2C address is 0x76 (the on-board resistor pulls the SDO pin low -> GND).
- The 7-bit device address is 111011x. The 6 MSB bits (left ones) are fixed.
- The SDO pin (#6) determines the lowest bit (0th).
    + Connecting pin SDO to GND results in slave address 1110110 (0x76).
    + Connecting pin SDO to VCC results in slave address 1110111 (0x77).
    + @important The SDO pin cannot be left floating; if left floating, the I2C address will be UNDEFINED.
- Use case "2x Sensors BME280 device address": configure the second sensor with I2C device address 0x77 (connect SDO to VCC).



## Sensor I2C protocol
- The sensor acts as a slave.
- The sensor supports the I2C Standard, Fast and High Speed modes (standard mode: 100 Kbit/s, full speed: 400 Kbit/s, fast mode: 1 Mbit/s, high speed: 3.2 Mbit/s).
- Dependencies (included): the official Bosch BMP280_driver https://github.com/BoschSensortec/BME280_driver 
  ​    Revision : V3.3.4
  ​    Date : Feb 26, 2018



## Issues



## Questions
- How to use the sensor as a GPS enhancement (e.g. time-to-first-fix improvement, dead reckoning, slope detection)?
- How to compute **the altitude** based on the atmospheric pressure? You can only calculate the altitude accurately if you know the atmospheric pressure (hPa) at sea level for your location and day.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

