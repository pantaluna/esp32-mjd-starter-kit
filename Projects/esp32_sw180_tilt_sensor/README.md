## Project Description
This project demonstrates the basics of interfacing with the SW180 tilt sensor.

## Instructions

1. GPIO-MUX: the input pin GPIO_NUM_SENSOR is looped back to the output pin GPIO_NUM_LED so this is wired by software, not by cables :)

2. Sensor SW-180
@logic
    pinvalue=1 no tilt | pinvalue=0 yes tilt

@wiring
​    Connect pin 1 (left) of the sensor to DATA
​    Connect pin 2 (middle) of the sensor to GND
​    Connect pin 3 (right) of the sensor to VCC

@configure
​    Use GPIO_PULLUP_ENABLE in gpio_config() XOR Connect a 10K resistor from DATA pin to POWER pin.

## Running the example
- Run `make menuconfig` and modify for example the GPIO PIN# that you want to use.
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

