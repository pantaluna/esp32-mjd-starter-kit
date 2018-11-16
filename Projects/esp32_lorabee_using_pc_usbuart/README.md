## Project Description
This project demonstrates how to issue basic commands to the LoraBee module via a USB-UART board (such as an FTDI board) using the command prompt on a Windows PC. It is an easy way to get familiar with the features of the LoraBee / Microchip RN2843A board.

This project is specific for the popular SODAQ LoraBee board. That board is an XBee breakout board that contains a Microchip RN2843A Lora module (note the subversion "A"). The Microchip RN2483A module contains a Semtech SX1276 Lora transceiver chip. It must be used in combination with the Parallax XBee Adapter Board 32403 which breaks out the Xbee pins to breadboard-compatible pins.

## Products
- SODAQ LoraBee RN2843. https://shop.sodaq.com/lorabee-rn2483-order-now.html
- Parallax XBee Adapter Board 32403. https://www.parallax.com/product/32403
- RobotDyn CP2104 UART USB To TTL 3.3V 5V Serial Adapter https://robotdyn.com/usb-serial-adapter-microcontroller-cp2104-5v-3-3v-digital-i-o-micro-usb.html

## Hardware installation
- Goto the component directory "components/mjd_lorabee" for documentation, installation, soldering, data sheets, FAQ, photo's, etc for the LoraBee board. @important Skip the section "wiring instructions" because we will not wire up the LoraBee board to an ESP32 development board but to the USB UART board.
- Configure the RobotDyn CP2104 UART USB board for 3.3V (consult the MJD ESP32 Starter Kit for detailed instructions).

## Wiring instructions
### General
[Goto the _doc folder component "components/mjd_lorabee" for photo's and documents.]

- Plug the Lorabee Xbee board into the Parallax board.
- Plug the Parallax board into the breadboard.

### Wiring instructions Parallax board - ESP development board
Use jumper wires to make the following connections.

The 0.1" Header row on the LEFT side of the Parallax board:

```
Parallax board  =>  USB UART Board
--------------      -------------
1   not used
2   VCC             3.3V
3   DOUT            RX (the UART Receive pin)
4   DIN             TX (the UART transmit pin)
5   not used
6   not used
7   not used
8   not used
9   not used
10  VSS/GND         GND
```

The 0.1" Header row on the RIGHT side of the Parallax board:

```
Parallax board  =>  ESP32 Dev Board
--------------      -------------
1   not used
2   not used
3   not used
4   not used
5   not used
6   not used
7   not used
8   not used
9   not used
10  not used
```

## Running the Bash scripts.
- Install Cygwin with the default settings.
- Start a Cygwin terminal
- Run `ls -l /dev` to identify the device id for the USB UART board. A typical name is /dev/ttyS15 Use this device id consequently in all Bash scripts.
- Copy-paste the commands from the various Bash scripts in the `bash_scripts` directory.

## Using a terminal emulator program
I recommend RealTerm (https://realterm.i2cchip.com/). Don't forget to tick the boxes "CR" and "LF" in the tab "Send" so each command is suffixed with \r\n.

## Notes
- It can be useful to also run the project `my_lorabee_tx_using_lib` or `my_lorabee_tx_using_lib` simultaneously to see how they all interact. These projects respectively transmit/receive Lora data using an ESP32 development board + LoraBee board.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

