# Geekcreit FT232RL FTDI - USB To TTL Serial Converter Adapter Module
Bought Mar2018.

https://www.banggood.com/3Pcs-FT232RL-FTDI-USB-To-TTL-Serial-Converter-Adapter-Module-p-959210.html

## FAQ
- This USB Bus Powered device gets its power from the USB bus (not from the exposed VCC pin - which is outgoing only!).
- Working voltage: 3.3V / 5V. The board contains a voltage regulator.
- Baud speeds 921600, 115200 and 9600 work. The FT232R chip supports all standard baud rates and non-standard baud rates from 300 Baud up to 3 Megabaud.

## JUMPER Voltage 3.3V/5V:
Set jumper:=3.3V (opposed to 5V) else the ESP32 MCU will blow-up!

- OK: when the jumper is set to 3.3V then the RX TX pins are also set to 3.3V (not 5V) so it is safe for an ESP32.

@important Some other UART boards still emit 5V on the TX RX pins when the VCC output pin is configured for 3.3V!

## PIN LAYOUT
````
(front side of the board)
1 GND
2 CTS
3 VCC
4 TX
5 RX
6 DTR
````

## Wiring Instructions
```
- @important Do not wire the VCC pin!
- Connect GND pin to the GND pin on the MCU.
- Connect TX pin to the RX pin on the MCU.
- Connect RX pin to the TX pin on the MCU.
```

## DRIVER Windows 10 USB device driver:
- Download from http://www.ftdichip.com/Drivers/VCP.htm the "FTDI VP Drivers CDM V2.12.28_Aug2017_Setup.exe"
- RESTART the computer after installing the driver.

## FIX MS Windows 10 Conflict FTDI - Serial Mouse
```
Windows Device Manager: Port COMx (UART): Port Settings => Advanced: Set SerialEnumeration=*OFF!
```

## Config Instructions:
- The baud speed used on the UART board must be the same as the MCU config value for baudspeed.
- The baud speed used on the UART board must be the same as the MobaXterm configured baudspeed.

## ISSUES
- It is equipped with a vintage USB-MINI connector (not USB-MICRO...) so you need an USB adapter to use it with an USB-Micro/USB-TypeC of your machine.
- The FTDI cannot be used to supply power (VCC+GND) to the ESP32 MCU because the amperage (current) is too low at 250mA. You have to use another board such as the RobotDyn Silicon Labs CP2104 UART board.
