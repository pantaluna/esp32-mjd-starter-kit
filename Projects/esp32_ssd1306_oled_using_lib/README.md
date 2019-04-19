## Project Description
This project demonstrates the basics of using the MJD ESP-IDF component "**mjd_ssd1306**" for the ESP32 module and the popular **OLED Displays 128x32 and 128x64 that are based on the SSD1306 OLED Driver IC**.

Go to the component directory "**components/mjd_ssd1306**" for more documentation, installation and wiring instructions, data sheets, FAQ, photo's, etc.



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



## Lab Setup / Wiring Diagram

The project will use the I2C protocol to interface between the ESP32 Master and the SSD1306 Slave

.

![_doc/Wiring Diagram - Project esp32_ssd1306_oled_using_lib-01.png](C:\myiot\esp\esp32_ssd1306_oled_using_lib\_doc\Wiring Diagram - Project esp32_ssd1306_oled_using_lib-01.png)



```
OLED SSD1306 PIN LAYOUT:

PIN#  PIN NAME	 Description
----  ---------- -----------
 1    GND        Ground
 1    VCC        Power supply (3.3V for the ESP32)
 3    SCL        I2C Serial Clock line
 4    SDA        I2C Serial Data line
 
WIRING DIAGRAM: MCU - OLED SSD1306:

OLED SSD1306 PIN  MCU PIN
----------------  -------
GND               GND
VCC               VCC 3.3V
SCL               #21
SDA               #17
```



## Running the example

- Run `make menuconfig` and review/modify the section "MY PROJECT CONFIGURATION".
  @tip You can also change the log level in Components->Logging: use level INFO for normal operation, use level DEBUG for more detailed logging and to get insights in what the component is actually doing.
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.



## An extract of the UART Debugging Output

```
...
I (2441) myapp: OK Task has been created, and is running right now
I (2441) gpio: GPIO[13]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0
I (2441) myapp:   mjd_ssd1306_init()...
I (2831) myapp: Show message#10 on the OLED...
I (4001) myapp: Show message#9 on the OLED...
I (5161) myapp: Show message#8 on the OLED...
I (6321) myapp: Show message#7 on the OLED...
I (7481) myapp: Show message#6 on the OLED...
I (8641) myapp: Show message#5 on the OLED...
I (9801) myapp: Show message#4 on the OLED...
I (10961) myapp: Show message#3 on the OLED...
I (12121) myapp: Show message#2 on the OLED...
I (13281) myapp: Show message#1 on the OLED...
I (14441) myapp:   mjd_ssd1306_cmd_clear_screen()...
I (14511) myapp:   mjd_ssd1306_cmd_write_line(), this text stays on the display...
I (14671) myapp:   mjd_ssd1306_deinit()...
W (14681) mjd: mjd_rtos_wait_forever()
```



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit



