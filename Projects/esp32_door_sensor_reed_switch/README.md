## Project Description
Demonstrates how to use a magnetic door/window sensor which is based on a reed switch. 

This app is developed for a door sensor which contains a MC-38 N.O. Normally Opened magnetic reed switch. This means that the switch is open (and no current flowing) when the 2 plastic blocks are away from each other. The GPIO interrupt handler contains de-bounce logic.

When the sensor is closed (the 2 blocks are close to each other) then the LED of the dev board will light up and the interrupt event (Positive Edge) will be logged in the UART Logger.

When the sensor is open (the 2 blocks are away from each other) then nothing happens.

The app uses either the GPIO ISR handler service or a simple pooling loop. The mode can be set with the variable `mjd_app_mode`.

The board's LED will light up when the GPIO input pin is HIGH. This gives you an indication of what is going on.

## Shop Product.
MC-38 Wired Door Window Sensor Magnetic Switch for Home Alarm System, NO (Normally Open).

Check the image "_doc/MC-38 Door Sensor - product.jpg"

## WIRING INSTRUCTIONS
The reed switch has 2 wires.

- Connect one wire to VCC 3.3V
- Connect one wire to digital input pin GPIO#27 of the ESP32 development board (or any other GPIO input pin which is free on your dev board).
- The internal pulldown resistor of GPIO#27 is enabled by the app.

## Sensor FAQ
- OK 3.3V
- The door sensor is based on a magnetic reed switch. Read more at https://en.wikipedia.org/wiki/Reed_switch

## Running the app
```
cd ~/esp32_door_sensor_reed_switch
make menuconfig
make flash monitor
```
## Related Projects
my_deep_sleep_wakeup



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

