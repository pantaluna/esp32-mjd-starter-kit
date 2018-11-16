# ESP-IDF MJD u-blox NEO-M8N GPS component
This is component based on ESP-IDF for the ESP32 hardware from Espressif.

Your app can initialize & de-initialize the GPS device, and read the actual GPS data (coordinates, fix_quality, number of satellites tracked) from the GPS device using this ESP-IDF component.

The component also exposes optional functions to control the power mode, to enable/disable the GNSS Receiver, to set the Measurement Rate, etc. These functions are typically used in a battery-powered solution.

The component includes its own RTOS Task to read the NMEA messages coming from the GPS Device via the UART interface. The component stores the latest GPS in its own data structure.

The app can read the actual / latest available data with a simple function call (see the example project) even when the device is powered-down at that moment.

The data structure consists of:
- Fix quality (0: no Fix | 1: 3D Fix)
- Latitude
- Longitude
- Number of satellites being tracked

## Example ESP-IDF project
my_neom8n_gps_using_lib

## Shop Product
u-blox NEO-M8N GPS Module with Shell.

## CHIP Info for this specific GPS Board
- The system software runs from ROM (opposed to from Flash). The module contains no Flash so firmware upgrades are not possible; this is fine because the ROM contains the recent version V3.x.
- The board supports UBX Protocol Version 18.00 (a recent version). A dump of the message UBX->MON->VER :

```
    swver: ROM CORE 3.01 (107888)
    hwver: 00080000
    ext:
        PROTVER=18.00
        FWVER=SGP 3.01
```

## Data Sheet

[Go to the _doc directory for more documents and images.]

https://www.u-blox.com/en/product/neo-m8-series



## Wiring Instructions

### MCU Adafruit HUZZAH32
- Pin #13 = Blue LED on the PCB
- Pin #23 = UART1 RX
- Pin #22 = UART1 TX

### GPS Board
You typically want to use the GPS Device with your ESP32 on a breadboard (female connectors) for development, but also with an FTDI Board (typically male connectors) so you can access and test the GPS board using the u-center software from u-blox on your PC.

This means that the wires from the GPS device must somehow have twin connectors. 

The best approach is to cut off the big white Pixhawk DF13 connector from the cable. For each of the 4 wires that came off of the DF13 connector Do get a male-to-female Dupont wire and cut it in half, and solder both ends to the wire. Goto the _doc folder for pictures of the GPS device and cables.

The function of each colored wire varies, even for models that seem the same on the outside. So open the black circular GPS cover (unscrew the back) and find inside the markings on the back of the PCB that explain what the function is for each wire (VCC, GND, RX, TX). You only have to document the 4 wires that were on the big white DF13 connector (not the smaller connector).

```
- External 6-pin DF13 connector.
This is the GPS UART interface.
    
MY DUPONT CABLE    GPS Cable    Function      ESP32 Pin / FTDI Pin
-----------------  ----------   ----------    --------------------
Green              RED          VCC           VCC
Grey               BLACK        GND           GND
Purple             YELLOW       UART TX       UART RX
Blue               ORANGE       UART RX       UART TX

- External 4-pin DF13 connector.
This is the COMPASS I2C interface ***NOTUSED***.

GPS Cable   Function  ESP32 Pin / FTDI Pin
---------   --------  --------------------
GREEN       I2C SCL   not used
BLACKISH    I2C SDA   not used
```


## Tool u-center V8.29 from the company u-blox
The first thing you want to do with this GPS product is to use this tool for MS Windows from the company u-blox to test and configure the device and learn how the NMEA and UBX messages work.

You need an FTDI USB-UART board to connect the GPS Board to the computer. Wire it up using the soldered Dupont wires that are described in the previous section. Pins: VCC, GND, UART TX, UART RX. Make sure to connect the TX wire of the GPS device to the RX connector of the FTDI board (and vice versa for the RX wire).

- @download https://www.u-blox.com/en/product/u-center-windows
- Set baudrate = 9600 in the software.
- Go to the ucenter Messages view to discover the hexadecimal code sequences for UBX commands (Slide up the lower half of the split window & copypaste to my programme!).
- All multi-byte values of the UBX commands are ordered in Little Endian format, unless otherwise indicated.
- The checksum of the UBX commands is calculated over the Message, starting and including the CLASS field, up until, but excluding, the Checksum Field.

## GPS FAQ
- OK 3.3V supply voltage.
- Features a GPS Receiver + Compass.
- The default baudrate for the UART1 is 9600 (this baudrate is also used by the u-blox u-center software).
- GNSS stands for Global Navigation Satellite System, and is an umbrella term that encompasses all global satellite positioning systems. This includes constellations of satellites orbiting over the earth’s surface and continuously transmitting signals that enable users to determine their position. The Global Positioning System (GPS) is one component of the Global Navigation Satellite System.
- The blue led on top of the GPS shell blinks if the GPs has a FIX (mostly "3D").

## Sending UBX CFG Messages using the u-center software
This section document some useful u-blox UBX Protocol commands that can be sent to the GPS device. These commands are also implemented in the ESP-IDF component so you can execute them by just calling a simple C function.

```
===UBX-RXM-PMREQ: Requests a Power Management task=== power down (time delimited or infinite)
HEX Commands:
- ucenter Version=0 Duration=15000 Action=2: 15 seconds
    B5 62 02 41 08 00 98 3A 00 00 02 00 00 00 1F 91
    {0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x98, 0x3A, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x1F, 0x91}

- ucenter Version=0 Duration=0 Action=2: infinite (=12 days!) It ALSO wakes up when you send whatever UBX command...
    B5 62 02 41 08 00 00 00 00 00 02 00 00 00 4D 3B
    {0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4D, 0x3B}

---Message Structure---
Header (uint8 uint8)
    0xB5 0x62
Class ID (uint8)
    0x02
ID (uint8)
    0x41
Length (uint16) The number format of the length field is a Little-Endian unsigned 16-bit integer.
    8
Payload
    Offset  NbrFmt            Scaling Name      Unit  Description
    0       U4 Unsigned Long  -       duration  ms    Duration of the requested task, set to zero for infinite duration. The maximum supported time is 12 days.
    4       X4 Bitfield       -       flags     -     Task flags (bit#1 = "backup"
                                  The receiver goes into backup mode for a time period defined by duration. 
                                  Provided that it is not connected to USB)
Checksum (The two 1-byte CK_A and CK_B fields hold a 16-bit checksum whose calculation is defined below)
    CK_A    uint8
    CK_B    uint8


===UBX-CFG-RST: Reset Receiver / Clear Backup Data Structures===
HEX Commands:
- ucenter Startup=Coldstart + Reset=Forced(Watchdog): ***All information is lost so the GPS has to start finding all satellites again (this takes a lot of time)***
    B5 62 06 04 04 00 FF B9 00 00 C6 8B
    {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0xFF, 0xB9, 0x00, 0x00, 0xC6, 0x8B}

- ucenter GNSS=stop + Startup=User Defined + Reset=*NONE: (only stops GNSS  = low power!)
    B5 62 06 04 04 00 00 00 08 00 16 74
    {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x16, 0x74}

- ucenter GNSS=start + Startup=User Defined + Reset=*NONE: (only starts GNSS = normal power!)
    B5 62 06 04 04 00 00 00 09 00 17 76
    {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x09, 0x00, 0x17, 0x76}

---Message Structure---
Header (uint8 uint8)
    0xB5 0x62
Class ID (uint8)
    0x06
ID (uint8)
    0x04
Length (uint16) The number format of the length field is a Little-Endian unsigned 16-bit integer.
    4
Payload
    Offset NbrFmt    Scaling Name         Unit     Description
    0      uint16    -       navBbrMask    -       BBR Sections to clear. The following Special Sets apply: 
                                    0x0000 Hot start
                                    0x0001 Warm start
                                    0xFFFF Cold start
    2      uint8     -       resetMode     -        Reset Type: 
                                    0x00 - HW reset
                                    0x01 - Controlled SW reset
                                    0x02 - Controlled SW reset (GNSS only)
                                    0x04 - HW reset after shutdown
                                    0x08 - Controlled GNSS stop
                                    0x09 - Controlled GNSS start
    3      uint8    -        reserved1    -    Reserved
Checksum (The two 1-byte CK_A and CK_B fields hold a 16-bit checksum whose calculation is defined below)
    CK_A    uint8
    CK_B    uint8


===UBX-CFG-RATE: Navigation/Measurement Rate Settings===
HEX Commands:
- ucenter TimeSource="1-GPS Time" MeasurementPeriod=1000ms: ***DEFAULT***
    B5 62 06 08 06 00 E8 03 01 00 01 00 01 39
    {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xE8, 0x03, 0x01, 0x00, 0x01, 0x00, 0x01, 0x39}

- ucenter TimeSource="1-GPS Time" MeasurementPeriod=100ms:
    B5 62 06 08 06 00 64 00 01 00 01 00 7A 12
    {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7A, 0x12}

- ucenter TimeSource="1-GPS Time" MeasurementPeriod=5000ms:
    B5 62 06 08 06 00 88 13 01 00 01 00 B1 49
    {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x88, 0x13, 0x01, 0x00, 0x01, 0x00, 0xB1, 0x49}

---Message Structure---
Header (uint8 uint8)
    0xB5 0x62
Class ID (uint8)
    0x06
ID (uint8)
    0x08
Length (uint16) The number format of the length field is a Little-Endian unsigned 16-bit integer.
    6
Payload
    Offset  NbrFmt  Scaling Name      Unit   Description
    0       uint16  -       measRate  ms     The elapsed time between GNSS measurements, 
                                             which defines the rate, e.g. 100ms => 10Hz, 1000ms => 1Hz, 10000ms => 0.1Hz
                                   75ms = 0x4B 0x00 FASTEST
                                 1000ms = 0xE8 0x03 DEFAULT
                                 5000ms = 0x88 0x13 SLOWER
                                50000ms = 0x50 0xC3 SLOWEST
    2       uint16  -       navRate   cycle  The ratio between the number of measurements 
                                             and the number of navigation solutions, 
                                             e.g. 5 means five measurements for every navigation solution. 
                                             Max. value is 127.
                                1 = DEFAULT (OK).
Checksum (The two 1-byte CK_A and CK_B fields hold a 16-bit checksum whose calculation is defined below)
    CK_A    uint8
    CK_B    uint8
```


## FYI PixHawk flight controller instructions:
Connect the GPS’s Hirose DF13 6-pin connector to the Pixhawk GPS port and the compass’s Hirose DF13 4-pin connector to the I2C port.




## Issues


## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

