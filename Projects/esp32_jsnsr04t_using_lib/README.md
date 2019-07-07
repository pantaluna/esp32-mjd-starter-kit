## Project Description esp32_jsnsr04t_using_lib
This project uses the ESP-IDF component "mjd_jsnsr04t" for the JSN-SR04T V2.0 Waterproof Ultrasonic Sensor Module and demonstrates all the features of the module in combination with an ESP32 development board. It dumps the measurements in the debug log.

Goto the component "components/mjd_jsnsr04t" for documentation, installation, soldering instructions (if any), wiring instructions, data sheets, FAQ, photo's, etc.



## What are the HW SW requirements of the ESP32 MJD Starter Kit?

### Hardware

- A decent ESP development board. I suggest to buy a popular development board with good technical documentation and a significant user base. Examples: [Adafruit HUZZAH32](https://www.adafruit.com/product/3405), [LOLIN D32](https://wiki.wemos.cc/products:d32:d32), [Espressif ESP32-DevKitC](http://espressif.com/en/products/hardware/esp32-devkitc/overview), [Pycom WiPy](https://pycom.io/hardware/).
- A JSN-SR04T-2.0 Waterproof Ultrasonic Sensor. Make sure to get the V2.0. @tip The README of each component contains a section "Shop Product".



### Software: ESP-IDF v3.2

- A working installation of the **Espressif ESP-IDF *V3.2* development framework**** (detailed instructions @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).

```
mkdir ~/esp
cd    ~/esp
git clone -b v3.2 --recursive https://github.com/espressif/esp-idf.git esp-idf-v3.2
```

- A C language editor or the Eclipse IDE CDT (instructions also @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).



## Lab Setup / Wiring Diagram

![Wiring Diagram - Project esp32_jsnsr04t_using_lib-01.png](.\_doc\Wiring Diagram - Project esp32_jsnsr04t_using_lib-01.png)



```
PIN LAYOUT & WIRING

Sensor board    =>	ESP32 Development Board
--------------      -----------------------
1   5V              3.3V pin (not the 5V or VUSB pin!)
2   Trig            GPIO#16 (Adafruit Huzzah32 pin 16 @ bottomleft-2)
3	Echo            GPIO#14 (Adafruit Huzzah32 pin 14 @ bottomright-2)
4   GND             GND pin
```



## Running the example
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.



## Notes
- Change the logging level of each ESP32 project using `make menuconfig` from "INFO" to "DEBUG" if you want to get more details about the requests and responses that are exchanged using the UART data channel.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

