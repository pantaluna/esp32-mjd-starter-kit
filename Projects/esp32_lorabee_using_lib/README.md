## Project Description
This project demonstrates the basic commands to configure the device and to read/write the NVM.

This project demonstrates the basics of using the MJD component "mjd_lorabee" for the popular SODAQ LoraBee board. That board is basically an XBee breakout board that contains a Microchip RN2843A Lora module (note the subversion "A"). The Microchip RN2483A module contains a Semtech SX1276 Lora transceiver chip. It must be used in combination with the Parallax XBee Adapter Board 32403 which breaks out the Xbee pins to breadboard-compatible pins.

Use it to get insights in how to use this component.

Goto the component "components/mjd_lorabee" for documentation, installation, soldering and wiring instructions, data sheets, FAQ, photo's, etc.

## Running the example
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.

## Notes
- Change the logging level of each ESP32 project using `make menuconfig` from "INFO" to "DEBUG" if you want to get more details about the requests and responses that are exchanged using the UART data channel.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

