## Project Description
Demonstrates how to use a switch or a magnetic door/window sensor to wake up an ESP32 from deep sleep. 

## Shop Product.
A button switch or a MC-38 Wired Door Window Sensor Magnetic Switch for Home Alarm System, NO (Normally Open).

Check the image "_doc/MC-38 Door Sensor - product.jpg"

## Wiring Instructions
Use the door sensor which has 2 wires, or use a button switch, and attach 2 Dupont cables to it.

- Connect one wire to VCC 3.3V
- Connect one wire to digital pin GPIO#27 (RTC_GPIO17) of the ESP32 development board. You can use another GPIO pin but it must be an RTC PIN which is exposed on your dev board.
- The internal pulldown resistor of digital pin GPIO#27 is enabled by the app.

## Sensor FAQ
- OK 3.3V
- The door sensor is based on a magnetic reed switch. Read more at https://en.wikipedia.org/wiki/Reed_switch

## Running the app
```
cd ~/esp32_deep_sleep_wakeup_basics
make menuconfig
make erase_flash
make flash monitor
```
## Related Projects
my_door_sensor_reed_switch



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

