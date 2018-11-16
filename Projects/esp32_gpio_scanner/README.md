## Project Description
This project demonstrates how to scan the GPIO pins.

The goal is to find the GPIO# of the on-board LED, and find out if the LED lights up when the pin is High or Low; the Adafruit Huzzah32's LED lights up when pin is high, the Wemos Lolin32Lite's LED lights up when pin is low.

The method is to set each GPIO pin to high and low and check if the LED is ON or not. Some pins cannot be set like that (e.g. the RESET pin) or are input-only pins. The list of excluded GPIO numbers are defined in the array lolin32lite_fatal_gpios[]

## Running the example
- Run `make menuconfig` and modify for example the GPIO PIN# that you want to use.
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

