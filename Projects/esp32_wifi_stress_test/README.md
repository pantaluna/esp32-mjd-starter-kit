# esp32_wifi_stress_test

## Project description
This app run a stress test for the ESP32 dev board in the role as Wifi Station.

The purpose is to verify the stability of the ESP32 Wifi software driver of a specific version of the ESP-IDF framework; to verify its correct operation with Wifi Access Point products of various vendors. Some older Wifi Access Points have known issues, for example when using Wireless Security "WPA Personal". The app will also use the Wifi driver after a wakeup of a deep sleep period (a special case for the ESP32). 

The app performs these actions 100 times in a loop:
- Start Wifi driver.
- Connect from the ESP2 Wifi Station to a Wifi Access Point.
- Check that Internet is accessible.
- Sync the current datetime using the SNT protocol.
- Disconnect from the Access Point.
- Stop the ESP32 Wifi driver.

It is good to have an app that you can use again and again to make sure that the Wifi software is working correctly.



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



# Running the example
- Run `make menuconfig` and declare in the section "MY PROJECT configuration" the name and password of your Wifi Access Point.
- Run `make erase_flash`.
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.
- The app will stop when an error occurs.
- The app will run again after a deep sleep period of 60 seconds.

# Notes
- You can lower the Logging Level from Verbose to Info using `make menuconfig`.
- The app uses the ESP-IDF component "mjd_wifi" to make working with Wifi as easy as possible.
- The memory usage is logged regularly so you can identify memory leaks very easily.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

