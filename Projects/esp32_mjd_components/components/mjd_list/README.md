# ESP32 MJD List component: Linux Kernel linked list implementation
This is component based on ESP-IDF for the ESP32 hardware from Espressif.



@source https://github.com/torvalds/linux/blob/master/include/linux/list.h 
@doc  https://www.kernel.org/doc/html/latest/core-api/kernel-api.html#list-management-functions
@ebook https://www.amazon.com/Linux-Kernel-Development-Robert-Love/dp/0672329468

The identifiers had to be prefixed with "mjd_" because the existing identifiers were already in use in ESP-IDF @ ~\esp-idf\components\esp32\include\rom\queue.h Examples: LIST_HEAD, list_for_each_entry(), ...



## Example ESP-IDF project
esp32_mjd_components



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

