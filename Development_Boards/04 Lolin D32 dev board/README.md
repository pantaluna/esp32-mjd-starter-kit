# LOLIN D32 development board (ESP32)

Bought y2018.



## General

- Very low deep sleep power consumption of 0.080 microAmp = 80 nanoAmp (which is very low; the Adafruit Huzzah32 consumes 6mA which is considered unacceptable).
- USB Micro Connector.
- Breadboard friendly: ***1*** free row at one long side of the bb, ***1*** free row at the other long side of the bb.
- Product Dimensions: length 57mm, width 25.4mm.
- JST-PH2 battery connector (check polarity).
- Lion/LiPo battery charger.
- LED #GPIO5 (high side).
- The ESP32 runs on 3.3V power and logic. The GPIO pins are not 5V safe!
- Chip is Espressif ESP32D0WDQ6 (revision 1). Flash size: 4MB. The crystal oscillator frequency is 32 kHz.
- GPIO#35 / A13: this pin is not exposed, it is used for measuring the voltage on the battery. The voltage is divided by 2 so be sure to double it once you've done the analog read.
- When charging a battery the PCB gets very hot at the top and the bottom, more specifically the area around the TP4054 battery charging IC which is located next to the JST-PH2 battery connector. But I have been told it stays within spec. In this configuration the power is coming in via the MicroUSB connector and a Lion battery is wired up to the JST-PH2 jack



## Product

LOLIN D32 https://wiki.wemos.cc/products:d32:d32



## USB UART
- Chip: Jiangsu Qin Heng WCH CH340C

- Install the USBUART driver "UART WCH CH341 driver V3.4.2014.08 Wemos-D32.zip"
- The MS Windows PORT COM# varies.
- Configure the MS Windows Port. Device Manager -> COMx: -> tab Port Settings: set
  - - Bits Per Second = 115200 (baud speed).
    - Data Bits = 8
    - Parity = None
    - Stop Bits = 1
    - Flow control = None.
- Disable the Microsoft Serial Ballpoint device if it shows up in the Windows device manager.
- Disable the Microsoft Serial Mouse device if it shows up in the Windows device manager using the Registry Editor (it conflicts with some UART-to-Serial drivers). 
http://www.taltech.com/support/entry/windows_2000_nt_serial_mice_and_missing_com_port



## The PCB contains the official Espressif WROOM32 rev1 module.
````
## Display chip_id
$ python /c/myiot/esp/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32 --port COM3 --baud 2000000 chip_id

	Connecting.....
	Chip is ESP32D0WDQ6 (revision 1)
	Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse, Coding Scheme None
	MAC: 80:7d:3a:80:17:4c
	Warning: ESP32 has no Chip ID. Reading MAC instead.
	MAC: 80:7d:3a:80:17:4c


$ python /c/myiot/esp/esp-idf/components/esptool_py/esptool/espefuse.py --port COM3 summary

EFUSE_NAME             Description = [Meaningful Value] [Readable/Writeable] (Hex Value)
----------------------------------------------------------------------------------------
Security fuses:
FLASH_CRYPT_CNT        Flash encryption mode counter                     = 0 R/W (0x0)
FLASH_CRYPT_CONFIG     Flash encryption config (key tweak bits)          = 0 R/W (0x0)
CONSOLE_DEBUG_DISABLE  Disable ROM BASIC interpreter fallback            = 1 R/W (0x1)
ABS_DONE_0             secure boot enabled for bootloader                = 0 R/W (0x0)
ABS_DONE_1             secure boot abstract 1 locked                     = 0 R/W (0x0)
JTAG_DISABLE           Disable JTAG                                      = 0 R/W (0x0)
DISABLE_DL_ENCRYPT     Disable flash encryption in UART bootloader       = 0 R/W (0x0)
DISABLE_DL_DECRYPT     Disable flash decryption in UART bootloader       = 0 R/W (0x0)
DISABLE_DL_CACHE       Disable flash cache in UART bootloader            = 0 R/W (0x0)
BLK1                   Flash encryption key
  = 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 R/W
BLK2                   Secure boot key
  = 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 R/W
BLK3                   Variable Block 3
  = 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 R/W

Efuse fuses:
WR_DIS                 Efuse write disable mask                          = 0 R/W (0x0)
RD_DIS                 Efuse read disablemask                            = 0 R/W (0x0)
CODING_SCHEME          Efuse variable block length scheme                = 0 R/W (0x0)
KEY_STATUS             Usage of efuse block 3 (reserved)                 = 0 R/W (0x0)

Config fuses:
XPD_SDIO_FORCE         Ignore MTDI pin (GPIO12) for VDD_SDIO on reset    = 0 R/W (0x0)
XPD_SDIO_REG           If XPD_SDIO_FORCE, enable VDD_SDIO reg on reset   = 0 R/W (0x0)
XPD_SDIO_TIEH          If XPD_SDIO_FORCE & XPD_SDIO_REG, 1=3.3V 0=1.8V   = 0 R/W (0x0)
SPI_PAD_CONFIG_CLK     Override SD_CLK pad (GPIO6/SPICLK)                = 0 R/W (0x0)
SPI_PAD_CONFIG_Q       Override SD_DATA_0 pad (GPIO7/SPIQ)               = 0 R/W (0x0)
SPI_PAD_CONFIG_D       Override SD_DATA_1 pad (GPIO8/SPID)               = 0 R/W (0x0)
SPI_PAD_CONFIG_HD      Override SD_DATA_2 pad (GPIO9/SPIHD)              = 0 R/W (0x0)
SPI_PAD_CONFIG_CS0     Override SD_CMD pad (GPIO11/SPICS0)               = 0 R/W (0x0)
DISABLE_SDIO_HOST      Disable SDIO host                                 = 0 R/W (0x0)

Identity fuses:
MAC                    Factory MAC Address
  = 80:7d:3a:80:17:4c (CRC 3d OK) R/W
CHIP_VER_REV1          Silicon Revision 1                                = 1 R/W (0x1)
CHIP_VERSION           Reserved for future chip versions                 = 2 R/W (0x2)
CHIP_PACKAGE           Chip package identifier                           = 0 R/W (0x0)

Calibration fuses:
BLK3_PART_RESERVE      BLOCK3 partially served for ADC calibration data  = 0 R/W (0x0)
ADC_VREF               Voltage reference calibration                     = 1135 R/W (0x5)

Flash voltage (VDD_SDIO) determined by GPIO12 on reset (High for 1.8V, Low/NC for 3.3V).


## Display flash_id
$ python /c/myiot/esp/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32 --port COM3 --baud 2000000 flash_id

Connecting......
Chip is ESP32D0WDQ6 (revision 1)
Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse, Coding Scheme None
MAC: 80:7d:3a:80:17:4c
Manufacturer: c8
Device: 4016
Detected flash size: 4MB

````



## Using rechargeable batteries with the dev board

- The Lion battery will be trickle-charged (+-4.0V +-50-100mA) when the USB cable is connected. The LED next to the USB jack will also light up.

- Ensure the JST-PH2 (PH=2mm distance, 2=2pin) battery connector is wired correctly (polarity) else the dev board gets destroyed. https://en.wikipedia.org/wiki/JST_connector Solution: cross-connect the red/black wires from the battery holder, with the black/red wires that have the JST-PH2 connector. Solder them and mark clearly that the wires are cross connected.
- Never use this dev board in combination with a a LiFePO4 battery. The dev board contains a **LiPo battery** charger circuit (emits 4.2V) which is not suited for LiFePO4 batteries (nominal voltage of 3.6V)!



## Minimizing power usage of the board

- The LED next to the Micro-USB connector is possibly always on when the board is powered by 5V. Desolder it if you have to save as much power usage as possible.



## Using a 6V solar panel with the dev board

It is easy.  You do not need a TP4056 breakout board. https://www.youtube.com/watch?v=gcbzdtRmYrM&t=112 

This development board is equipped with a TP4054 battery charger. The ME6211voltage regulator allows input voltages up to **6.0V**.

- Wire up a 3 Watt or 6 Watt solar panel (**maximum 6.0V**) directly to the VUSB and GND pins (or the MicroUSB slot).
- Wire up the Lion battery to the JST-PH2 slot.



## Using an external 5V power source with the dev board

Sometimes you want to use an external 5V DC power source.

@example A 12V DC battery => a power-efficient 5V DC step-down voltage regulator => output 5V DC for the MCU.

Make sure the USB cable in the Micro-USB connector and the Lion battery in the battery connector are both disconnected from the dev board. Hooking up multiple power sources will destroy the board instantly.

Wiring:

- Connect the GND pin of the power source to the GND pin, or the Micro-USB connector, of the dev board.
- Connect the VOUT 5V DC pin of the power source to the 5V/VUSB pin, or the Micro-USB connector, of the dev board.

Soldering:

* Desolder the LEDs if you require low power consumption. The LED is possibly always on when you feed 5V to the board (via the 5V/VUSB pin or the MicroUSB connector).



## Using an external 3V3 power source with the dev board

Sometimes you want to use an external 3V3 power source.

@example A 12V DC battery => a power-efficient 3.3V step-down voltage regulator => output 3.3V DC for the MCU.

Make sure the USB cable in the Micro-USB connector and the Lion battery in the battery connector are both disconnected from the dev board. Hooking up multiple power sources will destroy the board instantly.

Wiring:

- Connect the GND pin of the power source to the GND pin of the dev board. 
- Connect the VOUT 3.3V output pin of the power source to the 3.3V input pin of the dev board. This bypasses the LiPo Charger and the Voltage Regulator, which is what we want.

The **VBAT SENSE pin (GPIO#35)** returns an invalid reading in this configuration. It normally exposes the battery voltage of the connected Lion battery. The VBAT SENSE pin is setup as a 50% voltage divider between VBAT and GND.

Remember that the 5V rail is not available in this configuration. This is a disadvantage if you want to power sensors or actuators that require 5V DC.



## Using -LiFePO4- batteries with the dev board

- LiFePO4 batteries: ensures the JST-PH2 (PH=2mm distance, 2=2pin) battery connector is wired correctly (polarity) else the dev board gets destroyed. https://en.wikipedia.org/wiki/JST_connector
- LiFePO4 batteries: low power configuration guidelines (important for Production):
  1. Connect the LiFePO4 battery directly to the GND pin & the 3.3V pin (check polarity!). This bypasses the Lion charger and the voltage regulator.
  2. Make sure the USB cable is disconnected when the battery is hooked up to the 3.3V pin, and vice versa!
- The dev boards cannot be used for charging a LiFePO4 battery (max 3.7V). The dev board contains a **Lion/LiPo battery** charger circuit that emits +-4.2V which is not suited for LiFePO4 batteries.