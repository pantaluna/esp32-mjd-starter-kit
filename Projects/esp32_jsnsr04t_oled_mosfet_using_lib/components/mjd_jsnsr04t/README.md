# JSN-SR04T-2.0 Waterproof Ultrasonic Sensor Module [automotive style]
This component "mjd_jsnsr04t" is developed for the JSN-SR04T V2.0 Waterproof Ultrasonic Sensor Module in combination with an ESP32 V3.3 development board.

This is a component for the ESP-IDF software framework of the ESP32 hardware from Espressif.

Features:

* Implements all configuration options of the board. It comes with the proper defaults for all properties.
* Can define the number of sample measurements to make for one deducted measurement. Default: 5. These are the samples taken by the software, not the hardware itself which also takes 6 samples for one measurement).
* Can define the distance between the sensor and the artifact to be monitored. Default: 0 cm. This is typical for a stilling well setup. This distance is subtracted from the actual measurement. It is also used to circumvent the dead measurement zone of the sensor (+-25cm).
* Exposes functions to perform distance measurements. Detects invalid measurements. Detects out of range measurements. Detects invalid measurements that do not comply with the Range (a statistic).



## Example ESP-IDF project(s)
- `esp32_jsnsr04t_using_lib` This project demonstrates all the features of the module in combination with an ESP32 development board. It dumps the measurements in the debug log.
- `esp32_jsnsr04t_oled_mosfet_using_lib` This project demonstrates all the features in combination with an ESP32 development board, an OLED display module, a Power MOSFET to turn the sensor on and off (to save power consumption during deep sleep), and a deep sleep cycle.



## Shop Product.
- JSN-SR04T-2.0 Waterproof Ultrasonic Sensor. Make sure to get the V2.0.



## Wiring Instructions

### General
[Goto the _doc folder for photo's and documents.]

Connect the long cable with the black sensor to the sensor board. Connect the data pins and the power pins to the ESP32 dev board.

Check out the example projects for detailed wiring instructions.



### PIN Layout
```
Sensor board    Description
------------	-----------------------
1   5V          The input voltage pin. It supports 3.3V and 5V.
2   Trig        The trigger pin (input).
3	Echo        The echo pin (output).
4   GND         The GND pin.
```



## Data Sheet
[Goto the _doc folder for the documents.]



## FAQ Sensor Board
- Works with 3.3V operating voltage. **The following specifications are only valid when powering the board with 3.3V (opposed to 5V).**
- Current consumption: in the low mA range (which is significant).
- Minimum distance: 25 centimeters.
- Maximum distance: 350 centimeters.
- Probing frequency: 40 Khz.
- Measuring angle: 75 degrees.
- Distance accuracy: 1 centimeter.
- The board performs 6 samples and deducts one measurement. The EDP-IDF component "mjd_jsnsr04t" performs its own sampling on top of that in order to get weighted measurements and detect invalid measurements.
- The sensor board  has no resistor in the R27 spot so it works in the default operating mode (must trigger measurements manually, uses a custom data protocol, does not use the UART interface). See the datasheet for more information.



## Issues

None.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit





