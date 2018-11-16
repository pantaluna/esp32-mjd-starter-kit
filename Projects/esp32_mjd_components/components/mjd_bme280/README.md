# ESP-IDF MJD Bosch BME280 meteo sensor component
This is component based on ESP-IDF for the ESP32 hardware from Espressif.



## Example ESP-IDF project
my_bme280_sensor_using_lib

## Shop Product.
CJMCU-280E BME280 High Precision Atmospheric Pressure Sensor For Arduino.



## Wiring Instructions

### MCU Adafruit HUZZAH32
Pin #13 = Blue LED on the PCB

### Sensor PIN layout
```
1 VCC
2 GND
3 SCLK
4 SDA
5 CSB
6 SDO
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
- Good: temperature sensor which also provides humidity (opposed to the older BMP280).
- Metrics: humidity, pressure, temperature (all in high precision).
- Humidity sensor: 0 - 100%.
- Pressure sensor: 300 .. 1100 hPa. Pressure resolution: 0.16 Pa (not hectoPascal!). Altitude precision: 0.16 Pa is equivalent to +-1 meter difference in altitude.
- Temperature sensor range: -40°C .. +85°C. Temperature resolution 0.01°C.
- The breakouts boards contain 10K pull-up resistors. No external pull-up resistor is required.
- Supports the I2C protocol (opposed to the timing sensitive 1-Wire protocol of the meteo sensors DHT11 and AM2320 from Aosong).
- Sensor CHIP ID Unique Model ID in Register 0xD0 "id": 0x60 \
    The "id" register contains the chip identification number chip_id [bit7:0], which is 0x60. \
    This number can be read as soon as the device finishes the power-on-reset.
- The V1 supports the SPI protocol (up to 10 MHz).



## Sensor Power Mode BME280_FORCED_MODE
- In forced mode, a single measurement is performed according to the selected measurement and filter options.
- When the measurement is finished, the sensor returns to sleep mode and the measurement results can be obtained from the data registers.
- For the next measurement, the forced mode needs to be activated again.
- Forced mode is recommended for applications which require low sampling rate or host-based synchronization.
- Recommended minimum reading time interval: 1x / minute.



## Sensor Protocol Selection I2C/SPI
- Default: I2C.
- I2C/SPI interface selection is done based on the status of pin 5 CSB (Chip Select).
- The CSB pin must be connected to VCC to select the I2C interface. @important This is ALREADY DONE on this board GY-B11 by the on-board pull-up resistor, so pin 5 can be left disconnected when using the I2C interface.
- If CSB is connected to GND (pulled down) then the SPI interface is activated.



## Sensor I2C Address
- I2C Address: 0x76
- Use the project my_i2c_scanner to detect the actual I2C address.
- By default the SDO pin (#6) is not connected. This implies the I2C address 0x76 (the on-board resistor pulls the SDO pin low -> GND).
- The 7-bit device address is 111011x. The 6 MSB bits (left ones) are fixed.
- The SDO pin (#6) determines the first bit (0th). \
    + Connecting pin SDO to GND results in slave address 1110110 (0x76).
    + Connecting pin SDO to VCC results in slave address 1110111 (0x77).
    + PS The SDO pin cannot be left floating; if left floating, the I2C address will be UNDEFINED.
- Use case "2x Sensors BME280 device addres": configure the second sensor with I2C device address 0x77 (connect SDO to VCC).



## Sensor I2C protocol
- The sensor acts as a slave.
- The sensor supports the I2C Standard, Fast and High Speed modes (standard mode: 100 Kbit/s, full speed: 400 Kbit/s, fast mode: 1 Mbit/s, high speed: 3.2 Mbit/s).
- Dependencies (included): the official Bosch BMP280_driver https://github.com/BoschSensortec/BME280_driver \
  ​    Revision : V3.3.4 \
  ​    Date : Feb 26, 2018



## Issues



## Questions
- How to use the sensor as a GPS enhancement (e.g. time-to-first-fix improvement, dead reckoning, slope detection)?
- How to compute ALTITUDE based on the pressure? However, you can only really do a good accurate job of calculating altitude if you know the hPa pressure at sea level for your location and day.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

