## Project Description
This project demonstrates the basics of using the standard UART driver of the ESP-IDF framework.

Use it to get insights in how to use this component.

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

