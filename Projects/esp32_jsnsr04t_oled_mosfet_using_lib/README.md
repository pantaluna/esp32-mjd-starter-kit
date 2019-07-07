## Project Description esp32_jsnsr04t_oled_mosfet_using_lib
This project uses the ESP-IDF components "mjd_jsnsr04t" and "mjd_ssd1306".

It demonstrates all the features of the JSN-SR04T V2.0 Waterproof Ultrasonic Sensor Module in combination with:

- An ESP32 development board
- An OLED display module to show the measurements
- A Power MOSFET to turn the sensor on and off (to save power consumption during deep sleep)
- A deep sleep cycle.
- It also dumps the measurements in the debug log.

Goto the component directory "components/mjd_jsnsr04t" for documentation, installation, soldering instructions (if any), wiring instructions, data sheets, FAQ, photo's, etc.



## What are the HW SW requirements of the ESP32 MJD Starter Kit?

### Hardware

@tip The README of each ESP_IDF component contains a section "Shop Product".

- ESP development board. I suggest to buy a popular development board with good technical documentation and a significant user base. Examples: [Adafruit HUZZAH32](https://www.adafruit.com/product/3405), [LOLIN D32](https://wiki.wemos.cc/products:d32:d32), [Espressif ESP32-DevKitC](http://espressif.com/en/products/hardware/esp32-devkitc/overview), [Pycom WiPy](https://pycom.io/hardware/).
- JSN-SR04T-2.0 Waterproof Ultrasonic Sensor. Make sure to get the V2.0. 
- OLED Display Module 0.91 Inch 128x32 Blue I2C SSD1306 DC 3.3V 5V.
- N Channel Power MOSFET that is 3.3V compatible. Good ones:
  - IRF3708 N-Channel Power MOSFET in package THT TO-220.
  
  - IRLML6244PBF N Channel MOSFET in package SMD SOT23-3 (can be soldered easily on a SOT23-3 breakout board).



### Software: ESP-IDF v3.2

- A working installation of the **Espressif ESP-IDF *V3.2* development framework**** (detailed instructions @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).

```
mkdir ~/esp
cd    ~/esp
git clone -b v3.2 --recursive https://github.com/espressif/esp-idf.git esp-idf-v3.2
```

- A C language editor or the Eclipse IDE CDT (instructions also @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).



## Lab Setup / Wiring Diagram

![Wiring Diagram - Project esp32_jsnsr04t_oled_mosfet_using_lib-01.png](.\_doc\Wiring Diagram - Project esp32_jsnsr04t_oled_mosfet_using_lib-01.png)



```
PIN LAYOUT & WIRING

Sensor board    =>	ESP32 Development Board
--------------      -----------------------
1   5V              3.3V pin (not the 5V or VUSB pin!)
2   Trig            GPIO#16 (Adafruit Huzzah32 pin 16 @ bottomleft-2)
3	Echo            GPIO#14 (Adafruit Huzzah32 pin 14 @ bottomright-2)
4   GND             No connection here

OLED board  =>	ESP32 Development Board
----------      -----------------------
1   SDA            3.3V pin (not the 5V or VUSB pin!)
2   SCL            GPIO#17 (Adafruit Huzzah32 GPIO#17 = bottomleft-1)
3	VCC            GPIO#21 (Adafruit Huzzah32 GPIO#21 = bottomleft)
4   GND            No connection here

MOSFET             Destination
----------------   -----------------------
1   GATE           ESP32 dev board: GPIO#4 (Adafruit Huzzah32 GPIO#4 = bottomleft-6)
2   DRAIN          Sensor board: GND
2   DRAIN          OLED board: GND
3	SOURCE (GND)   ESP32 dev board: GND pin
```



## Running the example
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.



## Notes
- Change the logging level of each ESP32 project using `make menuconfig` from "INFO" to "DEBUG" if you want to get more details about the requests and responses that are exchanged using the UART data channel.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

