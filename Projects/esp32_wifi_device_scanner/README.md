## Project Description
This project demonstrates how to scan for devices on a Wifi network.

This project also demonstrates the basics of using the MJD component "mjd_wifi".

## NOTES:
- Video #163 Wi-Fi Sniffer as Sensor for Humans https://www.youtube.com/watch?time_continue=125&v=fmhjtzmLrg8
- RTOS RingBuffer API supports variable length buffers (=perfect for Wifi variable length packets).
- Wifi promiscious packet variable length: CB: len_packet=334 | CB: len_packet=148 | CB: len_packet=431| CB: len_packet=262] ...

## Running the example
- Run `make menuconfig` and modify for example the GPIO PIN# that you want to use.
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

