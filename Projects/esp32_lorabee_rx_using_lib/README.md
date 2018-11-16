## Project Description
This project demonstrates the Lora RX Receive functionality. Note: it uses Lora P2P and not LoRaWAN.

This project demonstrates the basics of using the MJD component "mjd_lorabee" for the popular SODAQ LoraBee board. That board is basically an XBee breakout board that contains a Microchip RN2843A Lora module (note the subversion "A"). The Microchip RN2483A module contains a Semtech SX1276 Lora transceiver chip. It must be used in combination with the Parallax XBee Adapter Board 32403 which breaks out the Xbee pins to breadboard-compatible pins.

Use it to get insights in how to use this component.

Goto the component "components/mjd_lorabee" for documentation, installation, soldering and wiring instructions, data sheets, FAQ, photo's, etc.

## Running the example
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.

## Notes
- Run this project on one ESP32 development board + LoraBee board to start receiving LoRa data. It is best used in combination with another ESP ESP32 development board + LoraBee board that runs the `my_lorabee_tx_using_lib` project which transmits data using the LoRa protocol.
- This project can also be used in combination with the `my_lorabee_using_pc_usbuart` project which demonstrates how to issue commands to the LoraBee module using a Windows PC and a USB-UART board (such as an FTDI). This is an easy way to get familiar with the features of the LoraBee / Microchip RN2843A board.
- Change the logging level of each ESP32 project using `make menuconfig` from "INFO" to "DEBUG" if you want to get more details about the requests and responses that are exchanged using the UART data channel.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

