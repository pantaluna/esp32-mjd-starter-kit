## Project Description
This project demonstrates the basics of using the MJD component "mjd_neom8n" for the GPS board "u-blox NEO-M8N".

Use it to get insights in how to use this component.

Goto the component "components/mjd_neom8n" for installation and usage/wiring instructions, data sheets, FAQ, photo's, etc. for the hardware and software.

The app initializes the GPS device, and reads the actual GPS data (fix_quality, coordinates, number of satellites tracked) from the GPS device using the ESP-IDF component.

The app demonstrates (simultaneously with reading the GPS data) the optional functions to control the power mode, to enable/disable the GNSS Receiver, to set the Measurement Rate, etc.

The app can read the actual / latest available GPS data with a simple function call, even when the device is powered-down at that moment.

## Running the example
- Run `make menuconfig` and modify for example the GPIO PIN# that you want to use.
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

