# RobotDyn CP2104 UART USB To TTL 3.3V 5V Serial Adapter
Bought Apr2018.

https://robotdyn.com/usb-serial-adapter-microcontroller-cp2104-5v-3-3v-digital-i-o-micro-usb.html

## Usages
1. As an UART Logger for ESP32 dev boards (typically when it is battery-only powered so no USB line is connected to the ESP32 dev board).
2. To hook up UART devices directly to the PC. Example: u-blox NEOM8N GPS Board, LoraBee Board, Plantower PMSA003 particle sensor.

## USB Type of the connector: Micro-USB.

@important This board uses 5V SIGNAL LEVELS by default! This board needs a soldering job before it can be used with 3.3V MCU's - to switch the default 5V mode to 3.3V else your MCU will blow up.

Three pads for setting Voltage 3.3V/5V for the RX TX pins. For use 3.3V level do:

0. Look at the pictures of the board.
1. Cut the trace between the two pads closest to the corner of the board (back of the board, label "5V").
2. Solder connect the two pads furthest away from the corner of the board (back of the board, label "3.3V").

## PIN LAYOUT
```
(front side of the board)
pin 1 DTR *Not used*
pin 2 3V3 *Not used, when the board is used in combination an ESP32 dev board* *Only used when connecting a board that requires 3.3V power to a PC.*
pin 3 5V  *Not used, when the board is used in combination an ESP32 dev board* *Only used when connecting a board that requires 5V power to a PC.*
pin 4 TXD
pin 5 RXD
pin 6 GND
```

## Wiring Instructions: using the board as an UART Logger in combination with an ESP32 dev board:
```
- [Do not connect voltage pins!]
- pin 4 TXD -> devboard pin UART RX
- pin 5 RXD -> devboard pin UART TX
- pin 6 GND -> devboard pin GND
```

## UART Chip: Silicon Labs CP2104
OK. This is a high-quality USB UART chip.
https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers


## Install the driver "Silicon Labs CP210x_Universal_Windows_Driver v10.1.3 [May2018]"

### General
@important The previous version "Silicon Labs CP210x_Universal_Windows_Driver v10.1.1 [Nov2017]" DOES NOT WORK.

@important RESTART the computer after installing the driver.



- The MS Windows PORT COMx port number can vary (it depends on which USB port/hub you plug the cable in and on what other USB devices are enabled on your machine).
- Configure the MS Windows COM Port: set baud speed = 115200.
- Disable the Microsoft Serial Ballpoint device if it shows up in the Windows device manager (it conflicts with some UART-to-Serial drivers such as Silicon Labs CPx).
- Disable the Microsoft Serial Mouse device via the Registry Editor (it conflicts with some UART-to-Serial drivers such as Silicon Labs CPx). http://www.taltech.com/support/entry/windows_2000_nt_serial_mice_and_missing_com_port

## Config Instructions:
- The baud speed used on the UART board must be the same as the baudspeed configuration of the ESP32 MCU (typically 115200).
- The baud speed used on the UART board must be the same as the baudspeed configured in the terminal programme (MobaXterm) (typically 115200).

## FAQ
- This USB Bus Powered device gets its power from the USB bus (not from the exposed VCC pin - which is outgoing only!).
- Working voltage: 3.3V - 5V. The board contains a voltage regulator.
- Speeds 921600, 115200 and 9600 certainly work.
- When the jumper is set to 3.3V then the RX TX pins are also using 3.3V (not 5V) so it is safe for an ESP32.

## ISSUES
- The board is by default configured for 5V TX RX signals. You have to resolder the pads for 3.3V (see earlier).
