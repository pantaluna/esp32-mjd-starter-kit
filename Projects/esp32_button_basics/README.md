## Project Description
This project demonstrates the basics of using the EDP-IDF framework for handling buttons and switches.

## Wiring instructions
- HUZZAH32 built-in LED PIN#13.
- Connect a wire from GPIO #16 to one leg of the pushbutton.
- The other leg of the button connects to the power rail.


## Remarks
- No external pull-down resistor (e.g. 4.7K or 10K ohm) to ground is required because the ESP32 internal pull-down for pin 32 is enabled.
- The internal pulldown for GPIO#16 ensures that the signal is LOW when the button is released (=not pressed).

## Running the example
- Run `make menuconfig` and modify for example the GPIO PIN# that you want to use.
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

