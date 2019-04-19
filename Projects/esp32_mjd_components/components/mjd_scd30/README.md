# ESP-IDF MJD SCD30 CO2 and RH/T Sensor Module

This is component for the ESP-IDF software framework of the ESP32 hardware from Espressif.

This component is developed for **the Sensirion SCD30 CO2 and RH/T Sensor Module**.

The ESP-IDF component configures the SCD30 device as an I2C slave device and exposes all settings and commands (including calibration) that are supported by the device as described in the data sheet and the interface description document.

More information:
- Sensirion SCD30 CO2 and RH/T Sensor Module
  https://www.sensirion.com/en/environmental-sensors/carbon-dioxide-sensors-co2/



## Example ESP-IDF project(s)

Go to the examples and learn how the component is used.

- ```esp32_scd30_sensor_settings_using_lib``` This project for the Sensirion SCD30 CO2 and RH/T Sensor Module is used to verify that the sensor is working properly, to show all the settings** of the sensor and to run the various calibration modes.

- ```esp32_scd30_sensor_readings_using_lib``` This project for the Sensirion SCD30 CO2 and RH/T Sensor Module reads continuously the CO2 measurement, the related and derived measurements and the air quality index.




## Shop Product

[ Goto the _doc folder for photo's.]

- Sensirion SCD30 CO2 and RH/T Sensor Module. This is a custom board from Sensirion. It is breadboard-friendly. We are happy to mention that we have not yet find cheap clones of this module.



## Wiring Instructions

### General

The specific wiring instructions are documented in the  ESP-IDF example project(s).

```
SCD30 SENSOR PIN LAYOUT:

PIN#  PIN NAME	 Description
----  ---------- -----------
1     VDD        Power supply (3.3V for the ESP32)
2     GND        Ground
3     SCL/TX     I2C Serial Clock / ModBus TX
4     SDA/RX     I2C Serial Data / ModBus RX
5     RDY        Data Ready Pin
6     PWM        PWM output (not supported yet by the hardware)
7     SEL        Iface select. Default (floating/pulldown) for I2C. Pullup for ModBus.
```

The pins #5, #6 and #7 are not used.



### Wiring for the I2C protocol

- Connect device pin "VDD" to the MCU pin VCC (3.3V).
- Connect device pin "GND" to the MCU pin GND.
- Connect device pin "SCL" to the MCU pin SCL. I use GPIO#21 on the HUZZAH32 dev board.
- Connect device pin "SDA" to the MCU pin SDA. I use GPIO#17 on the HUZZAH32 dev board).



## Device I2C protocol

- The device acts as a slave.
- The IC supports I2C clock speeds up to 100 Khz. Use a low speeds, such as 10 Khz, when using long wires (>25cm).



### Device I2C Slave Address

- **Default I2C Address: 0x61.**
- Use the project ```esp32_i2c_scanner``` to detect the actual I2C Address.



### I2C and wire length

- It is important to have stable I2C connections.
- Use **good quality breadboards**. Many have bad contacts, especially the ones that have been used for a month in the lab.
- Use **short, good quality Dupont cables**. In my experience a stock Dupont cable of 30cm is too long when using these modules on a breadboards (especially for the SCL and SDA connections). A 5cm Dupont cable always works.
- It is better to solder everything together on **a PCB** as soon as possible and use quality AWG22 wires and connectors.
- Guidelines "Topic: I2C flakiness:  best strategy to identify and fix?" https://forum.arduino.cc/index.php?topic=509323.0



## Exposed metrics of this component

The component is used to read continuously in fixed configurable intervals the CO2 measurements, the related and derived measurements and the air quality index. 

The returned metrics of each measurement are:

- The output metrics of the SCD30 sensor: 
  - CO2 ppm
  - Temperature Celsius
  - Temperature Fahrenheit
  - Relative humidity %
- Some computed metrics:
  - EU EN13779 IDA Air Quality Category **
  - Dew point Celsius
  - Dew point Fahrenheit



If the sensor returns a CO2 ppm value that is outside the allowed range of 400 -- 40000 pm then these are rejected by the component.



The EU EN13779 is the European Standard for ventilation and air conditioning in nonresidential buildings. The Indoor air quality (IDA) is classified by levels of carbon dioxide present in the air and fresh air levels being introduced per person.

```
Category  Desc                        CO2 level ppm Outside Air m3/h/person
--------  --------------------------  ------------- -----------------------
IDA 1     High indoor air quality     < 400         > 54
IDA 2     Medium indoor air quality   400 - 600     36 - 54
IDA 3     Moderate indoor air quality 600 - 1000    22 - 36
IDA 4     Low indoor air quality      > 1000        < 22
```

More info about the EU EN13779 Air Quality Standard for outdoor indoor:

- <https://www.airclean.co.uk/technical-bulletins/indoor-air-quality-european-standard/>
- <http://www.generalfilter.com/en/norms/en-13779/>



## Exposed settings and commands of this component

The component exposes various I2C settings and the sensor's persistent parametric settings, all the commands to read measurements (including calibration) and all the commands to activate various calibration options (ASC On, ASC Off, FRC) which are supported by the device <u>as described in the data sheet and the interface description document</u>.

I2C Properties:

- I2C Master Bus number (0 or 1)
- I2C Slave address
- SCL pin number
- SDA pin number
- Ticks to Wait during an I2C transaction

Sensor Properties:

* The measurement interval.
* The altitude compensation. The sensor is calibrated at sea level, and they are not designed to automatically compensate for changes to CO2 at higher altitudes.
* The temperature compensation. Do this if the sensor is mounted in an area/PCB/enclosure with a higher temperature than ambient air.



## Calibrating the sensor using this component

The sensor comes pre-calibrated from the factory. ASC is disabled by default. Please be knowledgeable when starting the calibration commands ASC or FRC! 

All calibration commands are supported by the component:

- Enable the Automatic Self-Calibration (ASC).
- Disable the Automatic Self-Calibration (ASC). It is by default disabled.
- Run a Forced ReCalibration (FRC) in fresh air. This means putting the project board outdoors and probably running the project on batteries; it is in this setup also handy that some output is displayed on the OLED Display. The firmware will first read CO2 measurements continuously until the values have stabilized (typically 3 minutes), and then it will set the FRC value to 400 ppm (the typical ppm value of fresh air on Earth).

The how-to's are described in the documentation of the example project ```esp32_scd30_sensor_settings_using_lib```.



## Data Sheet
[Go to the _doc directory for more documents and images.]

<https://www.sensirion.com/en/environmental-sensors/carbon-dioxide-sensors-co2/>




## IC/Module/ESP-IDF Component FAQ
- Operating voltage: 3.3V - 5.5V.
- Power consumption: average 19 mA, maximum 75 ma. These figures indicate that a project is not meant to be powered just on battery power.
- CO2 measurement range: 0 – 40000 ppm.
- The amount of CO2 is measured in parts-per-million (ppm). This is the relative proportion of carbon dioxide molecules in a volume of air.
- The amount of CO2 in fresh air is typically 400 ppm, or 0.04%.
- The device comes pre-calibrated from the factory. ASC and FRC are disabled by default.
- The sensor implements CRC Checksums for sending data and for receiving data. The mjd_scd30 component supports that.

- Check **the data sheet ** for detailed information.

- Check **the documented example projects and the sources of this component** for practical information.

  

## Issues

- **The SCD30 device actively uses I2C Clock Stretching**.  A stretch might occur before every ACK. The I2C timeout value of the sensor is documented as up to 12 milliseconds factory-default and up to 150 milliseconds when Automatic Self-Calibration (ASC) is enabled. These timeouts are extremely high and therefore the ESP32's I2C timeout property value had to be changed from the default of 0.4 milliseconds to 13.1 milliseconds (higher is not possible for an ESP32). It is advised to put the sensor on a separate I2C Master Bus so that it does not affect other I2C slaves when the I2C communication between the sensor or the ESP32 becomes unstable.
- **The 1st and 2nd measurements** that are read after running the command "Trigger Continuous Measurements" are often wrong e.g. the CO2 ppm is either 0 or very low (well below 400ppm). These measurements are identified by the mjd_sc30 component and are ignored.  @note The temperature and humidity metrics are always correct).
- The 1st CO2 ppm reading after running the command "**Soft Reset**" is always wrong; it is always 0.0 ppm. This measurement is identified by the mjd_sc30 component and is ignored.




## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

