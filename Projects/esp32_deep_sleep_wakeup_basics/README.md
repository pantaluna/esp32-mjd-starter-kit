## Project Description
Demonstrates how to use a switch or a magnetic door/window sensor to wake up an ESP32 from deep sleep. 



## What are the HW SW requirements of the ESP32 MJD Starter Kit?

### Hardware

- A decent ESP development board. I suggest to buy a popular development board with good technical documentation and a significant user base. Examples: [Adafruit HUZZAH32](https://www.adafruit.com/product/3405),  [Espressif ESP32-DevKitC](http://espressif.com/en/products/hardware/esp32-devkitc/overview), [Pycom WiPy](https://pycom.io/hardware/), [Wemos D32](https://wiki.wemos.cc/products:d32:d32).
- The peripherals that are used in the project.
  @tip The README of each component contains a section "Shop Products".
  @example A Bosch BME280 meteo sensor breakout board.

### Software: ESP-IDF v3.2

- A working installation of the **Espressif ESP-IDF *V3.2* development framework**** (detailed instructions @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).

```
mkdir ~/esp
cd    ~/esp
git clone -b v3.3 --recursive https://github.com/espressif/esp-idf.git esp-idf-v3.2
```

- A C language editor or the Eclipse IDE CDT (instructions also @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).



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

