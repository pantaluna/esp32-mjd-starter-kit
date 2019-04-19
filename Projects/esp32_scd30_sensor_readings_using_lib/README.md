## Project Description
This project is used to read the CO2 measurements, the related and derived measurements and the air quality index, continuously of **the Sensirion SCD30 CO2 and RH/T sensor**. Optionally, the project can use **an OLED 128x32/x64 SSD1306 display**.

The project demonstrates the basics of using the MJD ESP-IDF components "mjd_scd30" (sensor) and "mjd_ssd1306" (OLED) for the ESP32.

Please go to the component directories "**components/mjd_scd30**" and "**components/mjd_ssd1306**" for more documentation, installation and wiring instructions, data sheets, FAQ, photo's, etc.



## Related Projects

- ```esp32_scd30_sensor_settings_using_lib``` This project is used to verify that the sensor is working properly, to show all the settings** of the sensor and to run the various calibration modes.



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

- An **ESP32 development board**.
- A **Sensirion SCD30 CO2 and RH/T sensor**.
- Optional:  **128x32/128x64 SSD1306 OLED display**.



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
5     RDY        Data Ready Pin
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

The project is used to read continuously in fixed intervals the CO2 measurements, the related and derived measurements and the air quality index, and to show it in the UART debug log and optionally on the OLED Display.

The metrics of each measurement:

- The output metrics of the SCD30 sensor: 
  - CO2 ppm
  - Temperature Celsius
  - Temperature Fahrenheit
  - Relative humidity %
- Some computed metrics:
  - EU IDA Air Quality Category
  - Dew point Celsius
  - Dew point Fahrenheit

The firmware also reports a summary of the averages, minima and maxima.



More info about the EU EN13779 Air Quality Standard for outdoor indoor:

- <https://www.airclean.co.uk/technical-bulletins/indoor-air-quality-european-standard/>
- <http://www.generalfilter.com/en/norms/en-13779/>





- Run `make menuconfig` and review/modify the section "MY PROJECT CONFIGURATION".
  Enable the OLED Display if you have wired up such a display device.
  @tip You can also change the log level in Components->Logging: use level INFO for normal operation, use level DEBUG for more detailed logging and to get insights in what the component is actually doing.
- Run `make flash monitor` to build and upload the example to your board and connect to its serial terminal.



## An extract of the UART Debugging Output

Note: the 1st and 2nd measurement readings are always ignored because those readings are often incorrect.

```
...
I (425) mjd: ESP free HEAP space: 293732 bytes | FreeRTOS free STACK space (current task): 2044 bytes
I (455) myapp: @doc Wait 2 seconds after power-on (start logic analyzer, let peripherals become active, ...)
I (2465) myapp: OK Task has been created, and is running right now
I (3015) myapp: do mjd_scd30_init()
I (4165) mjd_scd30: SCD30 Log Device Params (*Read again from registers*):
I (4195) mjd_scd30:   measurement_interval: 5 (seconds)
I (4225) mjd_scd30:   data_ready_status: 0 (boolean)
I (4255) mjd_scd30:   automatic_self_calibration: 0 (boolean)
I (4295) mjd_scd30:   forced_recalibration_value: 400 (CO2 ppm)
I (4325) mjd_scd30:   temperature_offset: 100 (x 0.01°C)
I (4355) mjd_scd30:   altitude: 10 (meters)
I (4385) mjd_scd30:   firmware_version: 3.66
I (4385) myapp:   mjd_scd30_cmd_trigger_continuous_measurement()...
I (4415) myapp: LOOP: NBR_OF_MEASUREMENT_RUNS 10
I (4415) myapp:   ***SCD30 MEAS#1 of 10***...
I (11635) myapp:     ...data_ready_status: 1
I (11635) myapp:   mjd_scd30_cmd_read_measurement...
E (11665) mjd_scd30: mjd_scd30_cmd_read_measurement(). ABORT Ignore the 1st and 2nd measurement reading. The values are somet
imes wrong || err 264 (ESP_ERR_INVALID_RESPONSE)
E (12165) myapp: peripheral_task(). Cannot read measurement | err 264 (ESP_ERR_INVALID_RESPONSE)
I (12165) myapp:   ***SCD30 MEAS#2 of 10***...
I (17315) myapp:     ...data_ready_status: 1
I (17315) myapp:   mjd_scd30_cmd_read_measurement...
E (17345) mjd_scd30: mjd_scd30_cmd_read_measurement(). ABORT Ignore the 1st and 2nd measurement reading. The values are somet
imes wrong || err 264 (ESP_ERR_INVALID_RESPONSE)
E (17845) myapp: peripheral_task(). Cannot read measurement | err 264 (ESP_ERR_INVALID_RESPONSE)
I (17845) myapp:   ***SCD30 MEAS#3 of 10***...
I (22995) myapp:     ...data_ready_status: 1
I (22995) myapp:   mjd_scd30_cmd_read_measurement...
I (23025) myapp:     CO2:  792.8 | Temp C:   23.0 | Temp F:   73.4 | RelHum:   37.1 | DewPnt C:    7.5 | DewPnt F:   51.8
I (23025) myapp:     EU IDA Air Quality Category: 4 - IDA 4 (Low indoor air quality)
I (23195) myapp:   ***SCD30 MEAS#4 of 10***...
I (28355) myapp:     ...data_ready_status: 1
I (22995) myapp:   mjd_scd30_cmd_read_measurement...
I (28385) myapp:     CO2:  838.9 | Temp C:   23.0 | Temp F:   73.4 | RelHum:   37.1 | DewPnt C:    7.5 | DewPnt F:   51.8

...

I (55305) myapp:   ***SCD30 MEAS#10 of 10***...
I (60455) myapp:     ...data_ready_status: 1
I (60455) myapp:   mjd_scd30_cmd_read_measurement...
I (60485) myapp:     CO2:  859.9 | Temp C:   23.0 | Temp F:   73.4 | RelHum:   37.5 | DewPnt C:    7.7 | DewPnt F:   52.0
I (60485) myapp:     EU IDA Air Quality Category: 4 - IDA 4 (Low indoor air quality)
I (60655) myapp:
I (60655) myapp: REPORT:
I (60655) myapp:   NBR_OF_MEASUREMENT_RUNS: 10
I (60655) myapp:   nbr_of_valid_runs:       8
I (60665) myapp:   nbr_of_errors:           2
I (60665) myapp:     METRIC                        avg        min        max
I (60675) myapp:     ------                 ---------- ---------- ----------
I (60685) myapp:     CO2 ppm                   841.556    792.847    859.907
I (60685) myapp:     Temperature Celsius        22.986     22.966     23.018
I (60695) myapp:     Temperature Fahrenheit     73.374     73.339     73.432
I (60705) myapp:     Relative Humidity          37.276     37.061     37.505
I (60715) myapp:     Dew Point Celsius           7.616      7.537      7.719
I (60715) myapp:     Dew Point Fahrenheit       51.867     51.760     52.016
I (60725) myapp:   mjd_scd30_cmd_stop_continuous_measurement()...
I (60765) myapp:   mjd_scd30_deinit()...
I (60765) mjd: *** 19700101000100 Thu Jan  1 00:01:00 1970

```



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit


