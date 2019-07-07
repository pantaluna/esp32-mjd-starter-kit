# Adafruit HUZZAH32 ESP32 development board

BOUGHT Jan2018 20EUR @ Antratek.be



## General

- USB Micro Connector (more popular than the older USB Mini connector).
- Breadboard friendly (***2*** free rows at one side of the bb, ***1*** free row at the other side of the bb).
- Product Dimensions: length 50.0mm, width 23.5mm.
- JST-PH2 battery connector (check polarity).
- Lion/LiPo battery charger.
- LED #GPIO13 (low side).
- The ESP32 runs on 3.3V power and logic. The GPIO pins are not 5V safe!

- Chip is Espressif ESP32D0WDQ6 (revision 1). Flash size: 4MB. The crystal oscillator frequency is 32 kHz.

- GPIO#35 / A13:  this pin is not exposed, it is used for measuring the voltage on the battery. The voltage is divided by 2 so be sure to double it once you've done the analog reading.



## Product

Adafruit HUZZAH32 https://www.adafruit.com/product/3405



## USB UART
- Chip: Silicon Labs CP2104
https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers
- Install the driver "Silicon Labs CP210x_Universal_Windows_Driver v10.x" 
@important The older version "Silicon Labs CP210x_Universal_Windows_Driver v10.1.1 [Nov2017]" does not work.
- The MS Windows PORT COM# varies.
- Configure the MS Windows Port. Device Manager -> COMx: -> tab Port Settings: set
  - - Bits Per Second = 115200 (baud speed).
    - Data Bits = 8
    - Parity = None
    - Stop Bits = 1
    - Flow control = None.
- Disable the Microsoft Serial Ballpoint device if it shows up in the Windows device manager.
- Disable the Microsoft Serial Mouse device if it shows up in the Windows device manager using the Registry Editor (it conflicts with some UART-to-Serial drivers such as Silicon Labs CPx). 
http://www.taltech.com/support/entry/windows_2000_nt_serial_mice_and_missing_com_port



## The PCB contains the official Espressif WROOM32 rev1 module.
- Chip ESP32D0WDQ6 (revision 1!. [@INFO CHIP_VERSION 0x8 = REVISION#1:)]
````
## Display chip_id
$ python /c/myiot/esp/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32 --port COM3 --baud 2000000 chip_id
    Connecting....
    Chip is ESP32D0WDQ6 (revision 1)
    Features: WiFi, BT, Dual Core
    Chip ID: 0x3530aea43095

$ python /c/myiot/esp/esp-idf/components/esptool_py/esptool/espefuse.py --port COM3 summary
    Identity fuses:
        MAC                    MAC Address		  = 30:ae:a4:30:95:ac (CRC 35 OK) R/W
        CHIP_VERSION           Chip version               = 8 R/W (0x8)
        CHIP_PACKAGE           Chip package identifier    = 0 R/W (0x0)
    Flash voltage (VDD_SDIO) determined by GPIO12 on reset (High for 1.8V, Low/NC for 3.3V).

## Display flash_id
$ python /c/myiot/esp/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32 --port COM3 --baud 2000000 flash_id
    Connecting....
    Manufacturer: c8
    Device: 4016
    Detected flash size: 4MB
````



## Using rechargeable batteries with the Adafruit HUZZAH32 ESP32 Dev Board

- The Lion battery will be trickle-charged (+-4.0V +-50-100mA) when the USB cable is connected. The LED next to the USB jack will also light up.
- Ensure the JST-PH2 (PH=2mm distance, 2=2pin) battery connector is wired correctly (polarity) else the dev board gets destroyed. https://en.wikipedia.org/wiki/JST_connector Solution: cross-connect the red/black wires from the battery holder, with the black/red wires that have the JST-PH2 connector. Solder them and mark clearly that the wires are cross connected.
- Never use this dev board in combination with a a LiFePO4 battery. The dev board contains a **LiPo battery** charger circuit (emits 4.2V) which is not suited for LiFePO4 batteries (nominal voltage of 3.6V)!



## Minimizing power usage of the board

- The CP2104 USB-UART chip is wired up so that it never goes in suspend mode and therefore always consumes at least 6.5mA (also when the ESP32 is in deep sleep). You can de-solder the chip if you have to save as much power consumption as possible.
- The green LED next to the Micro-USB connector is always on when the board is powered by 5V. Desolder it if you have to save as much power usage as possible.



## Using an external 5V power source with the Adafruit HUZZAH32 ESP32 Dev Board

Sometimes you want to use an external 5V DC power source.

@example A 12V DC battery => a power-efficient 5V DC step-down voltage regulator => output 5V DC for the MCU.

In this configuration, please make sure the USB cable in the Micro-USB connector and the Lion battery in the battery connector are both disconnected from the HUZZAH32 dev board.

Wiring:

- Connect the GND pin of the power source to the GND pin, or the Micro-USB connector, of the HUZZAH32 board.
- Connect the VOUT 5V DC pin of the power source to the 5V pin, or the Micro-USB connector, of the HUZZAH32 board.

Soldering:

* Desolder the green LED if you require low power consumption. The LED is always on when you feed 5V to the board (via the USB-5V pin or the MicroUSB connector).



## Using an external 3V3 power source with the Adafruit HUZZAH32 ESP32 Dev Board

Sometimes you want to use an external 3V3 power source. 

@example A 12V DC battery => a power-efficient 3.3V step-down voltage regulator => output 3.3V DC for the MCU.

Wiring:

- Connect the GND pin of the power source to the GND pin of the HUZZAH32 board. 
- Connect the VOUT 3.3V output pin of the power source to the 3.3V input pin of the HUZZAH32 board. This bypasses the LiPo Charger and the Voltage Regulator, which is what we want.



The **VBAT SENSE pin (GPIO#35)** returns an invalid reading in this configuration. It normally exposes the battery voltage of the connected Lion battery. The VBAT SENSE pin is setup as a 50% voltage divider between VBAT and GND.

Remember that the 5V rail is not available in this configuration. This is a disadvantage if you want to power sensors or actuators that require 5V DC.



## Using -LiFePO4- batteries with the dev board
- LiFePO4 batteries: ensures the JST-PH2 (PH=2mm distance, 2=2pin) battery connector is wired correctly else the dev board gets destroyed. https://en.wikipedia.org/wiki/JST_connector
- LiFePO4 batteries: low power configuration guidelines (important for Production):
    1. Connect the LiFePO4 battery directly to the GND pin & the 3.3V pin (check polarity!). This bypasses the Lion charger and the voltage regulator.
    2. Make sure the USB cable is disconnected when the battery is hooked up to the 3.3V pin, and vice versa!
- The dev boards cannot be used for charging a LiFePO4 battery (max 3.7V). The dev board contains a **Lion/LiPo battery** charger circuit that emits +-4.2V which is not suited for LiFePO4 batteries.

