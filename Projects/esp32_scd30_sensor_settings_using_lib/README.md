## Project Description
This project is used to verify that the sensor is working properly, to show **all the settings** of **the Sensirion SCD30 CO2 and RH/T sensor** and to be able to run the various **calibration modes**. Optionally, the project can use **an OLED 128x32/x64 SSD1306 display**.

The project demonstrates the basics of using the MJD ESP-IDF components "mjd_scd30" (sensor) and "mjd_ssd1306" (OLED) for the ESP32.

Please go to the component directories "**components/mjd_scd30**" and "**components/mjd_ssd1306**" for more documentation, installation and wiring instructions, data sheets, FAQ, photo's, etc.



## Related Projects

- ```esp32_scd30_sensor_readings_using_lib``` This project reads continuously the CO2 measurement, the related and derived measurements and the air quality index.



## What are the HW SW requirements of the ESP32 MJD Starter Kit?

### Hardware

- A decent ESP32 development board. I suggest to buy a popular development board with good technical documentation and a significant user base. Examples: [Adafruit HUZZAH32](https://www.adafruit.com/product/3405),  [Espressif ESP32-DevKitC](http://espressif.com/en/products/hardware/esp32-devkitc/overview), [Pycom WiPy](https://pycom.io/hardware/), [Wemos D32](https://wiki.wemos.cc/products:d32:d32).
- The peripherals that are used in the project.

- @tip The README of each component contains a section "Shop Products".

  

### Software: ESP-IDF v3.2

- A working installation of the **Espressif ESP-IDF *V3.2* development framework**** (detailed instructions @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).

```
mkdir ~/esp
cd    ~/esp
git clone -b v3.3 --recursive https://github.com/espressif/esp-idf.git esp-idf-v3.2
```

- A C Language editor or the Eclipse IDE CDT (instructions also @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).



## Lab Setup

The hardware setup consists of

* An **ESP32 development board**.
* A **Sensirion SCD30 CO2 and RH/T sensor**.
* Optional:  **128x32/128x64 SSD1306 OLED display**.



## Wiring Diagram

![](_doc\Wiring Diagram - Project esp32_scd30_oled_using_lib-01.png)

```
SCD30 SENSOR PIN LAYOUT:

PIN#  PIN NAME	 Description
----  ---------- -----------
1     VDD        Power supply (3.3V for the ESP32)
2     GND        Ground
3     SCL/TX     I2C Serial Clock / ModBus TX
4     SDA/RX     I2C Serial Data / ModBus RX
5     RDY        Data Ready pin
6     PWM        PWM output (not supported yet by the hardware)
7     SEL        Iface select. Default (floating/pulldown) for I2C. Pullup for ModBus.

WIRING DIAGRAM: MCU - SCD30:

SCD30 PIN# PIN NAME  MCU PIN
---------- --------  -------
    1      VDD       VCC 3.3V
    2      GND       GND
    3      SCL/TX    GPIO#21
    4      SDA/RX    GPIO#17
    5      RDY       N.C.
    6      PWM       N.C.
    7      SEL       N.C.

SSD3106 OLED PIN LAYOUT:
 
PIN# PIN NAME	Description
---- ---------- -----------
1    GND        Ground
2    VCC        Power supply (3.3V for the ESP32)
3    SCL        I2C Serial Clock line
4    SDA        I2C Serial Data line
 
WIRING DIAGRAM: MCU - SSD3106 OLED:

SSD3106 PIN# PIN NAME  MCU PIN
------------ --------  -------
        1    GND        Ground
        2    VCC        Power supply (3.3V for the ESP32)
        3    SCL        GPIO#23
        4    SDA        GPIO#22
```



## Running the example

The project is used to verify that the sensor is working properly, to read out all the persistent parametric settings of the sensor, and to activate various calibration options (ASC On, ASC Off, FRC). The output is shown in the UART debug log, and optionally on the OLED Display.

You can enable or disable the Automatic Self-Calibration (ASC). It is by default disabled.

You can run a Forced ReCalibration (FRC) in fresh air. This means putting the project board outdoors and probably running the project on batteries; it is in this setup also handy that some output is displayed on the OLED Display. The firmware will first read CO2 measurements continuously until the values have stabilized (typically 3 minutes), and then it will set the FRC value to 400 ppm (the typical ppm value of fresh air on Earth).

The sensor comes pre-calibrated from the factory. Please be knowledgeable when starting the calibration commands ASC or FRC!



- Run the ```make menuconfig```, select ``` MY PROJECT configuration  --->```, select ```SCD30 Command``` and make your choice. The technical documentation of these features (calibration modes, etc.) are described in the README of the mjd_scd30 component.

```
┌────────────────── SCD30 Commands ──────────────────────────────────────┐
│ (X) Show the settings of the device                        │
│ ( ) Set ASC=On Activate continuous calculation of reference value for Automatic Self-Calibration                                                              │
│ ( ) Set ASC=Off Deactivate continuous calculation of reference value for Automatic Self-Calibration                                                         │
│ ( ) Run a FRC Forced ReCalibration in fresh air (400ppm)               │
└────────────────────────────────────────────────────────────────────────┘                │                    <Select>      < Help >                              │
└────────────────────────────────────────────────────────────────────────┘
```

- Go back to the section  ``` MY PROJECT configuration  --->``` and disable the OLED Display if you do not have wired up such a display.
- @tip You can also change the log level in ```Components->Logging```. Use level INFO for normal operation, use level DEBUG for more detailed logging and to get insights in what the component is actually doing.

- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.



## An extract of the Output for the command```(X) Show the settings of the device```.

```
...
I (385) mjd: This is an ESP32 chip
I (385) mjd:   CPU cores:    2
I (385) mjd:   Silicon rev.: 1
I (385) mjd:   CPU clock frequency (Hz):   160000000
I (395) mjd:   APB Advanced Peripheral Bus clock frequency (Hz):  80000000
I (405) mjd:   Flash:        4MB external
I (415) mjd:   [ESP-IDF Version: v3.2]
I (2455) myapp: OK Task has been created, and is running right now
I (2845) myapp: do mjd_scd30_init()
I (4035) mjd_scd30: SCD30 Log Device Params (*Read again from registers*):
I (4065) mjd_scd30:   ==>  measurement_interval: 4 (seconds)
I (4105) mjd_scd30:   ==> data_ready_status: 0 (boolean)
I (4135) mjd_scd30:   ==> automatic_self_calibration: 1 (boolean)
I (4165) mjd_scd30:   ==> forced_recalibration_value: 400 (CO2 ppm)
I (4195) mjd_scd30:   ==> temperature_offset: 100 (x 0.01°C)
I (4225) mjd_scd30:   ==> altitude: 10 (meters)
I (4255) mjd_scd30:   ==> firmware_version: 3.66

```



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit



