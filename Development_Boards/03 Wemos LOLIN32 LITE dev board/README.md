# WEMOS LOLIN32 LITE dev board

## FACTS

- @important Breadboard friendly (***1*** free row at one side of the bb, ***1*** free row at the other side of the bb).

- Product Dimensions: Length 50mm | Width 25.4mm

- Battery connector and battery charger module :)

- USB MICRO connector (micro is the most popular, usb-mini is the older one).

- UART Chip: CH340C (company WCH).

- Install MS Windows Driver: UART WCH CH34x_Install_Windows_v3_4 Wemos-Lolin32-Lite.zip

- MS Windows COM PORT = currently COM5 (the COM# number varies...).

- MS Windows Port COM5: set baud speed = 115200

- Disable the Microsoft Serial Mouse device (it conflicts with some UART-to-Serial drivers such as Silicon Labs CPx!). 

- The ESP32 Chip is ESP32D0WDQ6 REVISION#1 = OK.

- Battery Connector: JST PH2 (PH=2mm distance, 2=2pin) https://en.wikipedia.org/wiki/JST_connector

- Blue LED GPIO#22 can be used by your app
  @location Below the PCB Antenna.
  @important It is connected to ***VCC*** so set pin level 1 for OFF, and pin level 0 for ON [The opposite of the MCU Adafruit HUZZAH32!]

