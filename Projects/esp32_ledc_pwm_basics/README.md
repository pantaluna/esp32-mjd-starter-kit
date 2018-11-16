## Project Description
This example shows how to control the intensity of LEDs using the standard ESP-IDF LEDC driver (a LED Controller using PWM):
- Frequency
- Duty resolution
- Duty cycle

Espressif Reference Documentation: https://esp-idf.readthedocs.io/en/latest/api-reference/peripherals/ledc.html

## FAQ
- LEDC: the available duty resolution steps = (2 ^ bit_num bit_num of the duty resolution) - 1
- LEDC: the maximal frequency is 80000000 / (2 ^ bit_num of the duty resolution)
- https://physics.stackexchange.com/questions/19040/limit-of-human-eye-flicker-perception

## Hardware Setup
- This setup uses a yellow LED with a Forward voltage of 2.0-2.2V. The resistor value of 100 Ohm is specific for the LED's forward voltage. If you use a LED with a higher Forward Voltage then you might have to lower the resistor value else the LED will not turn on.  
- Wire ESP32's GPIO_NUM_14 -> (+) 5mm yellow LED (-) -> Resistor 100 Ohm -> GND

## Running the example
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

