# ESP-IDF MJD LoraBee LoRa Board (Microchip RN2843)
This is a component for the ESP-IDF software framework of the ESP32 hardware from Espressif.

This component is developed for the SODAQ LoraBee Microchip RN2483A board in combination with an ESP32 development board.

The ESP-IDF component implements all configuration options of the LoraBee board.

The ESP-IDF component exposes functions to configure the LoraBee board settings (NVM, frequency, spreading factor, bandwidth, power, watchdog, ...).

The ESP-IDF component exposes functions to send and receive data using the LoRa protocol. This is the LoRa P2P protocol.

The LoraWAN protocol will be implemented in a later version. It will be relatively easy because the fundamental building blocks for communicating between the ESP32 development board and the LoraBee board are already implemented.



## Example ESP-IDF project(s)
- `my_lorabee_using_lib` This project demonstrates how to issue basic commands to the LoraBee module using the ESP32.
- `my_lorabee_using_pc_usbuart` This project demonstrates how to issue basic commands to the LoraBee module using a Windows PC and a USB-UART board (such as an FTDI). This is the recommended setup to get familiar with the features of the LoraBee / Microchip RN2843A board.
- `my_lorabee_tx_using_lib` (Lora Transmit) Use this project to configure a station to send data conform the LoRa protocol using the ESP32. The LoRa settings match those of the other project `my_lorabee_rx_using_lib`.
- `my_lorabee_rx_using_lib` (Lora Receive). Use this project to configure another station to receive data conform the LoRa protocol using the ESP32. The LoRa settings match those of the other project `my_lorabee_tx_using_lib`.



## Shop Product.
- SODAQ LoraBee Microchip RN2843. https://shop.sodaq.com/lorabee-rn2483-order-now.html
- Parallax XBee Adapter Board 32403. https://www.parallax.com/product/32403



## LoraBee board + Parallax XBee Adapter Board
[Goto the _doc folder for photo's and documents.]

The SODAQ LoraBee board is basically an XBee breakout board that contains a Microchip RN2843A Lora module (note the subversion "A"). The Microchip RN2483A module contains a Semtech SX1276 Lora transceiver chip. As far as I am aware the LoRaBee only uses the pins VCC, GND, RX & TX of the Microchip RN2483A module.

The antenna connector type of the LoraBee is SMA male (inner pin). The supplied 868Mhz antenna connector type is SMA female (inner pin).

Unfortunately the 2mm pin spacing of the LoraBee XBee board is not compatible with the 2.54mm (0.1") pin spacing of a standard breadboard.

We have to use the Parallax XBee Adapter Board 32403 which breaks out the Xbee pins to breadboard-compatible pins.



## Soldering Instructions
[Goto the _doc folder for photo's and documents.]

### SODAQ LoraBee board
Solder join the 2 pads of the SJ1 solder bridge at the backplate of the LoraBee board. This way you can hard reset the board (via the microcontroller) when driving for example the GPIO#17 high.

By default the board does no reset at all. The RN2843A's Reset Pin is wired via the solder bridge SJ1 to the Parallax XBee breakout pin "IO3" (it is the 4th pin on the RIGHT side of the Parallax board). The solder bridge SJ1 is open by default to safeguard your keys when you reset your microcontroller, so you can continue transmitting. But you can close the solder bridge SJ1 if you want the LoraBee to reset whenever you want; connect the LoRaBee RESET pin#17 to the Parallax XBee pin "IO3" to the MCU pin GPIO#14.

### Parallax XBee Pro Adapter Board
- Pitch 2.0mm (not 2.54mm or 1.27mm)!
- Solder the 2 small XBee pin header strips to the board. @tip First plug the Lorabee Xbee board into the XBee pin headers (else the solder often falls into the female headers and the Lorabee board will not fit anymore...).
- Solder the 2 breadboard pin header strips to the board.

## Special: soldering XBee headers 2.0mm pitch PCB sockets onto the Parallax XBee Pro Adapter Board

Source: https://www.mattairtech.com/index.php/development-boards/xeno-standard.html

Most 2.0mm female headers (especially low profile headers) require extra care when soldering because **the inside of the sockets can very easily fill with solder, rendering the headers useless! The 2.0mm headers do not have a mechanical barrier preventing solder from flowing into the socket!**

Follow these guidelines for soldering:

+ First install the Xbee radio into the PCB socket so it acts as a heat sink. You may want to start soldering on the ends, then periodically wiggle the Xbee radio to make sure the pins are not being soldered.
+ Use a relatively low soldering iron temperature.
+ Pay attention to how much solder is flowing into the hole. Too much solder may keep flowing into the hole rather than build up around the pin (the heat sinking should help).



## Wiring Instructions

### General
[Goto the _doc folder for photo's and documents.]

- Plug the Lorabee Xbee board into the Parallax board.
- Plug the Parallax board into the breadboard.

### Wiring instructions Parallax board - ESP development board
Use jumper wires to make the following connections.

The 0.1" Header row on **the LEFT side** of the Parallax board:

```
Parallax board  =>  ESP32 Development Board
--------------      -----------------------
1   not used
2   VCC             3.3V
3   DOUT            GPIO#23 (Adafruit Huzzah32: #23=bottomright)(UART RX)
4   DIN             GPIO#22 (Adafruit Huzzah32: #22=bottomright-1)(UART TX)
5   not used
6   not used
7   not used
8   not used
9   not used
10  not used
11  VSS/GND         GND
```

The 0.1" Header row on **the RIGHT side** of the Parallax board:

```
Parallax board  =>  ESP32 Development Board
--------------      -----------------------
1   IO0
2   IO1
3   IO2
4   IO3             GPIO#14 (IO3 is wired to the LoraBee reset pin)
5   not used
6   not used
7   not used
8   not used
9   not used
10  not used
11  (not connected on Parallax board)
```

If you want to use other pins on the ESP32 dev board then make sure to change the #define's in the C program.




## Firmware
- Microchip RN2483A Firmware version: RN2483 1.0.3 Mar 22 2017 06:00:42



## Data Sheet
[Goto the _doc folder for photo's.]

https://support.sodaq.com/sodaq-one/schema-lorabee-rn2483/

https://www.microchip.com/wwwproducts/en/RN2483

https://www.semtech.com/products/wireless-rf/lora-transceivers/SX1276



## FAQ LORABEE Microchip RN2483A
- 3.3V
- Serial 57600 8N1.
- Use the Microchip Windows tool "LoraDevUtility" to get started with the LoraBee board.
- Supported ISM Frequency Bands "EU863-870" 863.000-870.000 MHz (also "EU433" 433.050-434.790 MHz but then you need another antenna!).
- The RN2483A module requires '\r\n' (CR+LF) at the end of an input line. The response also ends with '\r\n' (CR+LF).
- The RN2483A module does not echo back any characters it receives via UART.
- The RN2483A module does not use RTS (nor CTS) for serial comms at the moment.
- LoraBee PIN#17 = RESET SIJ (active @ Low).
- LoRaBee proper reset: close the SJ1 jumper and apply a reset pulse (0 LOW) via Xbee pin 17.
- Lora Data Rate should be as fast as possible to minimize your airtime. SF7BW125 is usually a good setting to start with, it consumes the least power and airtime.
- The baud rate can be changed by triggering the auto-baud detection sequence of the module. To do this, the host system needs to transmit to the module a break condition followed by a 0x55 character at the new baud rate.



## SOP: Microchip RN2483A Firmware upgrade @ LoraBee module
- Current Firmware version: v1.0.3 of May 2017.
- Use the Microchip LoraDevUtility in Boot Load Recover mode to upload new firmware using the "<"LoRa Development Utility v 1.0.1">" 
  @important Set baudrate=9600 in the utility (other speeds such as 57600, 115200 do not work).
- Documentation: Microchip_LoRa_FirmwareUpdate_viaGUI.pdf



## Issues

None.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit





