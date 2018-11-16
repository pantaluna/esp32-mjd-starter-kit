#ADAFRUIT HUZZAH32 ESP32 dev board

BOUGHT Jan2018 20EUR @ Antratek.be

## USB Power Connector
USB Micro (more popular than the older USB Mini).

## IMPORTANT
- Breadboard friendly (***2*** free rows at one side of the bb, ***1*** free row at the other side of the bb).
- Product Dimensions: length 50.0mm | width 23.5mm
- Lipo/Lion battery charger included :)

## UART:
- Chip: Silicon Labs CP2104 \
[OK. This is a high-quality USB UART chip].
https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers

- - Install the driver "Silicon Labs CP210x_Universal_Windows_Driver v10.1.3 [May2018]" \
@important The previous version "Silicon Labs CP210x_Universal_Windows_Driver v10.1.1 [Nov2017]" DOES NOT WORK.\
@important IF the "Windows 10 Universal" driver does not work THEN install the "non-universal" driver "Silicon Labs CP210x_Windows_Drivers v6.7.6 [Jun2018]"\
@important RESTART the computer after installing the driver.\

- The MS Windows PORT COMx varies.
- Configure the MS Windows Port: set baud speed = 115200.
- Disable the Microsoft Serial Ballpoint device if it shows up in the Windows device manager.
- Disable the Microsoft Serial Mouse device via the Registry Editor (it conflicts with some UART-to-Serial drivers such as Silicon Labs CPx!). \
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
#MSYS2
$ python /c/myiot/esp/esp-idf/components/esptool_py/esptool/espefuse.py --port COM3 summary
    Identity fuses:
        MAC                    MAC Address		  = 30:ae:a4:30:95:ac (CRC 35 OK) R/W
        CHIP_VERSION           Chip version               = 8 R/W (0x8)
        CHIP_PACKAGE           Chip package identifier    = 0 R/W (0x0)
    Flash voltage (VDD_SDIO) determined by GPIO12 on reset (High for 1.8V, Low/NC for 3.3V).
````

## FLASH CHIP info
````
#
# Display flash_id
$ python /c/myiot/esp/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32 --port COM3 --baud 2000000 flash_id
    Connecting....
    Manufacturer: c8
    Device: 4016
    Detected flash size: 4MB
````

## Pin info
Some pins are special about the ESP32 - here's a list of 'notorious' pins to watch for!

* A2 / I34 - this pin is input only! You can use it as an analog input so we suggest keeping it for that purpose.
* A3 / I39 - this pin is input only! You can use it as an analog input so we suggest keeping it for that purpose.
* IO12 - this pin has an internal pulldown, and is used for booting up. We recommend not using it or if you do use it, as an output only so that nothing interferes with the pulldown when the board resets.
* A13 / I35 - this pin is not exposed, it is used only for measuring the voltage on the battery. The voltage is divided by 2 so be sure to double it once you've done the analog reading.

## FACTS
- The Adafruit Huzzah crystal oscillator frequency is 32 kHz.
- The ESP32 runs on 3.3V power and logic. The GPIO pins are not 5V safe!
- Chip is ESP32D0WDQ6 (revision 1).
- Flash size: 4MB.
- Flash remote burning data interface up to 4 Mbps.
- A yellow LED next to the USB jack, which will light up while the battery is charging. This LED might also flicker if the battery is not connected, it's normal.
- GPIO #0 is the BOOT Button (the physical button is not present on the HUZZAH32).
- GPIO #??? is connected to the reset button.
- GPIO #6-#11 reserved for the flash chip.
- GPIO #34-#39 are input-only, and do NOT have internal pull-up or pull-down circuitry.

## Using batteries with the ADAFRUIT HUZZAH32 ESP32 dev board

@important Wire up a decoupling 1000uF capacitor between the development board pins VCC and GND to improve handling of high current spikes (typical for Wifi connect phase) ELSE the MCU will die much sooner! It keeps the voltage more constant. https://electronics.stackexchange.com/questions/2272/what-is-a-decoupling-capacitor-and-how-do-i-know-if-i-need-one

## Using LiFePO4 batteries with the ADAFRUIT HUZZAH32 ESP32 dev board
- LiFePO4 batteries: ensures the JST-PH2 (PH=2mm distance, 2=2pin) Battery Connector is wired correctly ELSE **SMOKING MCU**. https://en.wikipedia.org/wiki/JST_connector
  + SOLUTION: cross-connect the red/black wires from the battery holder, with the black/red wires that have the JST-PH2 connector. Solder them and mark clearly that the wires are cross connected.

    + Adafruit advice: There are two ways to arrange the positive and negative wires in a JST-PH2 plug, and you can find both versions in the wild. 
  	  We use the slightly more common version, which puts ***the black/negative wire closest to the USB connector when the LiPo is plugged into a Feather***.  
  	  A LiPo whose connector is wired the other way will reverse-bias all the chips and overload their internal protection circuits!
  @photo Lipo Battery Connector - black wire at right side (else blown MCU!).jpg
  PROBLEM: I plugged once the battery in the onboard connector and I GOT A SMALL POP AND THE BOARD HEATED UP AND GAVE OFF SMOKE. The dev board's battery input via the LiPo connector no longer works...

- LiFePO4 batteries: low power configuration guidelines: ***IMPORTANT for Production***
    + SOLUTION: 
        1. Connect the LiFePO4 battery directly to the 3.3V pin & GND using my wires with the JST-PH2 Battery Connector. Plugin the JST-PH2 connector in the +-rail of the breadboard (check POLARITY!).
        2. @important Make sure the USB cable is disconnected when the battery is hooked up to the 3.3V pin, and vice versa!!!
        3. Wire the pin ENABLE pin to the pin GND to disable the Voltage Regulator. No resistor is needed.

    + Disable the Voltage Regulator to lower the current usage.
        You save 20% power/current consumption.
        The LiFePO4 battery voltage is within safe range for the ESP32 chip, so the voltage regulator can be disabled!
        LiFePO4's give a fixed 3.3V so no voltage divider needed - opposed to USB Power and other Li* battery types. So Disable the voltage divider chip (the BAT and USB pins will still be powered):
        => Simply wire the ENABLE pin to GND (without a resistor between them).

    + Adafruit advice: connecting a LiFePO4 battery to the 3.3V pin is the correct choice, because that would bypass the LiPo charger and the voltage regulator.
      If you connect the LiFePo to the HUZZAH's 3.3v pin, the connection will be downstream from the regulator.
      The onboard 3.3V regulator takes its input from the BAT pin, which is connected directly to the LiPo and the output from the MCP73831 LiPo charger.

    + Adafruit advice: if you want to use an external (solar) charger with the LiFePo, you need to build more or less the same circuit: charger connected to battery, 
      both feeding the regulator, and the regulator connected to the HUZZAH's 3.3v pin.
      If the input voltage to a regulator falls below the output-plus-dropout level, the regulator's output will no longer meet the chip's published specs. That might be something minor, 
      like a small loss of ripple rejection, or it could be major, like the regulator simply refusing to work. You have to test the parts and find out.

- @danger LiFePO4 Battery - Never use my ESP32 dev boards for charging the LiFePO4 battery.
    The dev board contains a LiPO battery charger circuit (emits 4.2V!) which is not suited for LiFePO4 batteries!
    A Li-on circuit (or LiPO which is the same as Lion) stops charging at 4.2V; this is a serious issue for LiFePO4 batteries which should only be charged up to a max of 3.65V!
