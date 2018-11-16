# ESP32 MJD List component: Linux Kernel linked list implementation
This is component based on ESP-IDF for the ESP32 hardware from Espressif.

@source https://github.com/torvalds/linux/blob/master/include/linux/list.h 
@doc  https://www.kernel.org/doc/html/latest/core-api/kernel-api.html#list-management-functions
@ebook https://www.amazon.com/Linux-Kernel-Development-Robert-Love/dp/0672329468

The identifiers had to be prefixed with "mjd_" because the existing identifiers were already in use in ESP-IDF @ ~\esp-idf\components\esp32\include\rom\queue.h Examples: LIST_HEAD, list_for_each_entry(), ...

## Example ESP-IDF project
mjd_components
