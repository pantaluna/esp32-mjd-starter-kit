# ESP-IDF MJD SSD1306 Component

This is a component for the ESP-IDF software framework of the ESP32 hardware from Espressif.

This component is developed for **the popular 128x32 and 128x64 OLED displays which are based on the SSD1306 OLED Driver IC**.

**The main purpose is to make it easy to display short debug text messages and status text information on the OLED screen.**

The font is Courier Regular 12px and each line is 16px high. You can write up to 2 lines on a 128x32 display and up to 4 lines on a 128x64 display.

The API supports:

- 128x32 and 128x64 OLED monochrome displays based on the SSD1306 IC.
- Writing a string to a specific line on the display (the screen is cleared when writing the line#1). You can write up to 2 lines to a 128x32 OLED display. You can write up to 4 lines to a 128x64 OLED display.
- Clearing the screen.

If you need more functionality then feel free to use the U8G2 component directly.



## Example ESP-IDF project(s)

Go to the examples and learn how the component is used.

```esp32_ssd1306_oled_using_lib``` This project demonstrates the component mjd_ssd1306 to show text on a an OLED display.




## Shop Product

[ Goto the _doc folder for photo's.]

- Adafruit Monochrome 128x32 I2C OLED graphic display.
- 0.91 Inch 128x32 I2C Blue OLED Display Module SSD1306 DC 3.3V 5V.
- White color 128X64 OLED Display Module For Arduino 0.96 inch I2C.

@important Choose a variant of these products that supports the I2C protocol. Those typically have at least 2 pins labeled SCL and SDA.



## Wiring Instructions

### General

The specific wiring instructions are documented in the  ESP-IDF example project(s).



### PIN layout of the SSD306 OLED

[ Goto the _doc folder for photo's.]

```
PIN# PIN NAME Description
---- -------- -----------
1    GND      Ground
2    VCC      Power supply (3.3V for the ESP32)
3    SCL      I2C Serial Clock line
4    SDA      I2C Serial Data line
```



### Wiring for the I2C protocol

- Connect device pin "VCC" to the MCU pin VCC (3.3V).
- Connect device pin "GND" to the MCU pin GND.
- Connect device pin "SCL" to the MCU pin SCL. I use GPIO#21 on the HUZZAH32 dev board.
- Connect device pin "SDA" to the MCU pin SDA. I use GPIO#17 on the HUZZAH32 dev board.



## Device I2C protocol

- The device acts as a slave.
- The IC supports I2C clock speeds up to 100 Khz.



### Device I2C Slave Address

- **Default I2C Address: 0x3C.**
- Use the project ```esp32_i2c_scanner``` to detect the actual I2C Address and to verify that the device is working.

  

### I2C and wire length

- It is important to have stable I2C connections.
- Use **good quality breadboards**. Many have bad contacts, especially after using one for a month in the lab.
- Use **short, good quality Dupont cables**. In my experience a stock Dupont cable of 30cm is too long when using these modules on a breadboard (for the SCL and SDA connections). A 10cm Dupont cable always works.
- It is better to solder everything together on **a PCB** as soon as possible and use quality AWG22 wires and connectors.
- Guidelines "Topic: I2C flakiness:  best strategy to identify and fix?" https://forum.arduino.cc/index.php?topic=509323.0



## Data Sheet

[Go to the _doc directory for more documents and images.]

<http://www.solomon-systech.com/en/product/advanced-display/oled-display-driver-ic/ssd1306/>




## IC/Module/ESP-IDF Component FAQ
- Operating voltages: 3.3V, 5V.
- Resolution: 128x32 and 128x64 dot matrix panel.
- Check **the data sheet ** for detailed information.
- Check **the documented example projects and the sources of this component** for practical information.



## Issues



## Development - Dependencies

@note This procedure has already been executed.

- ```EDP-IDF component u8g2: v2.25.10 of 2019-02-09``` <https://github.com/olikraus/u8g2>
  - Download that release from GitHub into your project's components directory.
  - Remove all subdirectories except ```./csrc/``` to save precious space. The total space used goes down from 210MB to 20MB. The ```./csrc/``` subdirectory contains the C Language sources of the project.
  - The repo already contains an ```component.mk``` file so that the ESP-IDF Framework recognizes it as a valid ESP-IDF component :) 
  - This library is hardware agnostic so it is still required to write a HAL for the ESP32 platform. This is where the source code from ```nkolban``` comes in.



## Development - Integrated Sources

@note This procedure has already been executed.

- Nkolban's HAL for the U8G2 library for monochrome OLED screens
  - Download the ```v3.0 branch``` of the repo https://github.com/nkolban/esp32-snippets
  - Extract the files ```u8g2_esp32_hal.h``` and ```u8g2_esp32_hal.c``` from the directory ```./master/hardware/displays/U8G2``` into this ```mjd_ssd1306``` component.
  - I have modified the HAL so that it is possible to specify in the configuration which ESP32's I2C Master bus to use (```I2C_NUM_0``` or ```I2C_NUM_1```). The default is ```I2C_NUM_0```. This feature is handy when using multiple I2C slave devices in a project.
  - I have modified the HAL so that it is possible to specify in the configuration if the component should initialize the I2C Master Bus, or not. The default is ```Yes```. This feature becomes handy when using multiple I2C slave devices in a project.



## Credits

<https://github.com/olikraus/u8g2>

<https://github.com/nkolban/esp32-snippets/tree/master/hardware/displays/U8G2>



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

