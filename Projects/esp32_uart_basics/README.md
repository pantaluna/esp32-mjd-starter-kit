## Project Description
This project demonstrates the basics of using the standard UART driver of the ESP-IDF framework.

Use it to get insights in how to use this component.



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



## Shop Products: USB UART Boards
- RobotDyn CP2104 USB To TTL UART 3.3V 5V Serial Adapter
- Geekcreit FT232RL FTDI USB To TTL Serial Converter Adapter Module

## Hardware instructions
- MCU Adafruit HUZZAH32:
    + Pin #13 = Blue LED on the PCB
    
- MCU Wemos Lolin32 Lite:
    + Pin #22 = Blue LED on the PCB

- For USB UART board: Robotdyn CP2104 R USB UART board (preferred board):
    + Computer USB Port COM18 (might vary)
    + Ensure you soldered the pads for 3.3V signal level configuration (not 5V) ELSE your MCU wil blow up!

- For USB UART board: Geekcreit FTDI FT232R USB UART board:
    + Computer USB Port COM17 (might vary)
    + Set the jumper to 3.3V so that its VCC RX TX pins use 3.3V ELSE your MCU wil blow up!
    
- Attach the USB UART board to the MCU:
     + @important Do not wire the device pin VCC to the MCU pin VCC because the MCU in this project is powered by its own USB micro connector. Connecting both will lead to a short circuit.
     + Connect device pin GND to the MCU pin GND.
     + Connect device pin TX to the MCU pin RX, I choose GPIO#23 (Huzzah32 #23 bottomright)(Lolin32lite #13 bottomright)
     + Connect device pin RX to the MCU pin TX, I choose GPIO#22 (Huzzah32 #22 bottomright-1)(Lolin32lite #15 bottomright-1)

## TEST PROCEDURE:
- Run `make menuconfig`.
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.
- Start Terminal emulator MobaXTerm, RealTerm or PuTTY. Connect to the COMx Port (COMx 115200 8N1N).
- Type something in the terminal window.
- The typed text will also appear in the MCU monitor screen (after a delay), and echoed back to the terminal window.
- PS The RX read_bytes() buffer size is kept small by design, so you can experience the effect of small buffers.

## FAQ
 - The UART buffer sizes must be at least `#define UART_FIFO_LEN (128) //!< Length of the hardware FIFO buffers >`



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

