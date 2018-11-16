## Project Description
This project demonstrates how to scan the I2C bus for I2C slave devices and get their I2C slave address.

The goal is to find the I2C slave address of new I2C devices that you connected to an I2C bus.

## Wiring Instructions
- Attach an I2C device
    + SCLK = GPIO#21 (Huzzah32 #21 bottomleft)  (Lolin32lite #13 bottomleft)
    + SDA  = GPIO#17 (Huzzah32 #17 bottomleft-1)(Lolin32lite #15 bottomleft-1)

## Running the example
- Run `make menuconfig` and modify for example the GPIO PIN# that you want to use.
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

