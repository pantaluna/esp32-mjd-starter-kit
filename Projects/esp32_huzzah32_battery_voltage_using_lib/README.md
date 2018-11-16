## Project Description
The main purpose of this project is to demonstrate how to obtain with a simple function call the current battery voltage on an Adafruit HUZZAH32 development board.

This project demonstrates the basics of using the MJD component "mjd_huzzah32" for the Adafruit HUZZAH32 development board.

Use it to get insights in how to use this component.

Goto the component "components/mjd_huzzah32" for the documentation.

## Running the example
- Run `make menuconfig` and verify the Voltage Reference for ADC: \
     Component config ---> Adafruit HUZZAH32 ---> (1090) Adafruit Huzzah32 Voltage Reference in mV (typically +-1100)
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

