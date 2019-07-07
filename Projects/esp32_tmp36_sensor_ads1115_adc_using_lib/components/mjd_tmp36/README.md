# ESP-IDF MJD TMP36 Analog Temperature Sensor
This is a component for the ESP-IDF software framework of the ESP32 hardware from Espressif.

This component mjd_tmp36 is developed for **the TMP36 Analog Temperature Sensor** from Analog Devices.

This component mjd_tmp36 converts the raw voltage reading of the sensor (read by the ADC) to the ambient temperature in Degrees Celsius.

More information about the device:
- Adafruit Learning: TMP36 Temperature Sensor <https://learn.adafruit.com/tmp36-temperature-sensor/overview>



## Example ESP-IDF project(s)
```esp32_tmp36_sensor_ads1115_adc_using_lib``` This project demonstrates the components mjd_ads1115 and mjd_tmp36. The **mjd_ads1115 component** for the TI ADS1115 Analog-To-Digital-Convertor is used to read, accurately and linearly, the voltage output of the analog temperature sensor. The **mjd_tmp36 component** for the TMP36 sensor is used to convert the raw voltage reading of the ADC to the ambient temperature in Degrees Celsius transparently.



## Shop Product

[ Go to the _doc folder for photo's.]

- Adafruit TMP36 Analog Temperature Sensor.



## Wiring Instructions

The specific wiring instructions are documented in the  ESP-IDF example project(s).

```
- Connect a 0.1uF ceramic capacitor between the TMP36's pins VCC and GND. It should be as close as possible to the VCC pin of the sensor.
- Connect a +-2.2uF tantalum capacitor between the TMP36's pins VCC and GND when the
device is operated in the presence of high frequency radiated or conducted noise.
```



## PIN layout Of the device

[ Goto the _doc folder for photo's.]

```
PIN#  PIN NAME	  Description
----  ----------  -----------
 1    VCC         Power supply (3.3V for the ESP32)
 2    VOUT        Voltage Output
 3    GND         Ground
```



## Data Sheet

[Go to the _doc directory for more documents and images.]

<https://www.analog.com/en/products/tmp36.html>



## Harsh Environments

- It is recommended to seal the device using electrically non-conductive epoxy paint or PCB varnish.
- The sensor can be made waterproof by using shrink tubes with hot glue inside them.



## IC / ESP-IDF Component FAQ

- Operating voltage: 2.7V  - 5.5V
- Offset Voltage: 500 milliVolt
- Output Voltage Scaling: 10 milliVolt / °C
- Output Voltage at 25°C: 750 milliVolt
- Do not read the output at very high frequencies else the unit might overheat and influence the readings.
- Less than 50 µA quiescent current
- The TMP36 is functionally compatible with the LM50.
- For long wires (> 1 meter): add a 750 Ohm resistor on the VOUT output pin.
- Check **the data sheet ** for detailed information.
- Check **the example projects** for practical information.
- Check **the documented sources of this component** for more information.



## Issues

/



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

