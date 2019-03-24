# ESP-IDF MJD TI ADS1115 16-Bit ADC 4 Channel with Programmable Gain Amplifier
This is component for the ESP-IDF software framework of the ESP32 hardware from Espressif.

This component is developed for **the TI ADS1115 Analog to Digital Convertor**.

The ESP-IDF component configures the I2C slave device and returns **the raw readings and the voltage-adjusted readings** of the analog input pin(s) on the ADS1115.

More information:
- Video Ralph S Bacon #104 ADS1115 Analog-to-Digital Converter for Arduino (2018) <https://www.youtube.com/watch?v=8qGr6Q5Ymps>

The ADS1115 is typically used to process analog input signals from various devices (battery voltage, analog sensors, piezo vibration sensor). It measures the voltage (compared to GND) one an analog pin, or the voltage difference between 2 analog pins.



## Example ESP-IDF project(s)
```esp32_ads1115_adc_using_lib``` This project demonstrates the component mjd_ads1115. It uses a 50% voltage divider setup so you can measure GND (+-0V), VCC (3.3V) and the divided voltage (+-1.65V).

```esp32_tmp36_sensor_ads1115_adc_using_lib``` This project demonstrates the components mjd_tmp36 and mjd_ads1115. The component mjd_tmp36 instructs the component mjd_ads1115 to read the voltage from the Analog TMP36 analog sensor with the proper ADC settings and convert the voltage to degrees Celsius.



## Shop Product

[ Goto the _doc folder for photo's.]

- Adafruit ADS1115 16-Bit ADC - 4 Channel with Programmable Gain Amplifier breakout board.

- GY-ADS1115 CJ-ADS1115 I2C ADS1115 16 Bit ADC 4 channel Module with Programmable Gain Amplifier breakout board.

  

## Wiring Instructions

### General

The specific wiring instructions are documented in the  ESP-IDF example project(s).



### PIN layout Of the breakout boards

[ Goto the _doc folder for photo's.]

```
PIN#  PIN NAME	  Description
----  ----------  -----------
 1    V VCC       Power supply (3.3V for the ESP32)
 2    G GND       Ground
 3    SCL         I2C Serial Clock
 4    SDA         I2C Serial Data
 5    ADDR        I2C slave address select
 6    ALERT/READY Comparator output or conversion ready
 7    A0          Analog input 0
 8    A1          Analog input 1
 9    A2          Analog input 2
10    A3          Analog input 3
```

@important The use of the ALERT/READY pin is optional.

@important The breakout offers 4 analog pins. You will only use the ones you need in your project.



### Wiring for the I2C protocol

- Connect device pin "V" to the MCU pin VCC (3.3V).
- Connect device pin "G" to the MCU pin GND.
- Connect device pin "SCL" to the MCU pin SCL. I use GPIO#21 on the HUZZAH32 dev board.
- Connect device pin "SDA" to the MCU pin SDA. I use GPIO#17 on the HUZZAH32 dev board)
- Connect device pin "ALERT/READY" to an MCU input pin. I use GPIO#16 on the HUZZAH32 dev board. The use of this pin is optional.



## Pullup Pulldown Resistors

The breakout boards mentioned above contain a 10K pullup resistor for these pins: SCL, SDA, ALR. It is not needed to wire external 10K pullup resistors from the device pins SCL and SDA to the MCU pin VCC (3V), or to enable the MCU's internal pullups for those pins. 

The breakout boards mentioned above contain a 10K pulldown resistor for these pins: ADR.



## Device I2C protocol

- The device acts as a slave.
- The IC supports various I2C clock speeds: Standard Mode (100 KHz), Fast Mode (400 KHz), Fast Mode Plus (1 Mhz), High Speed: 3.2 Mbit/s. @important **The maximum I2C speed of the ESP32 is 1 MHz.** @tip **Very slow speeds such as 10Khz when using long wires (>25cm) is also supported.**



### Device I2C Slave Address

- **Default I2C Address: 0x48.**
- @tip Use the project ```esp32_i2c_scanner``` to detect the actual I2C Address.
- The I2C Address can be modified. Read the instructions in the data sheet section "9.5.1.1 I2C Address Selection". And you also have to specify a different I2C Address when configuring the mjd_ads1115 component using the "i2c_slave_addr" property.



### Wire length on the I2C pins SCL and SDA 

- It is important to have stable I2C connections for a breadboard setup: use **good quality breadboards** (many have bad contacts, especially ones that have been used for a while), use ****short good quality Dupont cables. In my experience a stock Dupont cable of 30cm is too long when using these modules on a breadboards (especially for the SCL and SDA connections). **A 5cm Dupont cable always works.**
- It is better to solder everything on a PCB as soon as possible, use quality wires and the best connectors.
- Guidelines "Topic: I2C flakiness:  best strategy to identify and fix?" https://forum.arduino.cc/index.php?topic=509323.0



### Wire length on the Ax analog input pins

- Using **a 5 meter copper wire (22 AWG)** for the analog pin did not affect the voltage readings. Longer wires might.
- For longer lengths adding a small 750 Ohm resistor in series with the input pin helps to reduce bandwidth noise.
- For extreme long length it is advised to convert the voltage output of the device to a current output.



## Data Sheet
[Go to the _doc directory for more documents and images.]

<http://www.ti.com/product/ADS1115>




## IC/Module/ESP-IDF Component FAQ
- Why is an external ADC module essential in combination with an ESP32?
  The ADC readings of the ESP32 module's ADC are not linear (very important), they only 12-bit, contains 2 ADC's but one is actually allocated to the WiFi functionality.
  The ADS1115 is much better: does not lose linearity within the whole input range, officially 16-bit (actually 15-bit for positive voltages), a maximum of 4 input signals, includes a Programmable Gain Amplifier, supports differential conversions, incorporate a low-drift voltage reference, ...
- Operating voltage 2V - 5.5V.
- The market offers several breakout modules which are very handy. The ADS1115 IC is very small (X2QFN 10 pins 1.50 mm × 2.00 mm, VSSOP 10 pins 3.00 mm × 3.00 mm) and so can be hard to solder manually if you want to make your own breakout board.
- Check **the data sheet ** for detailed information.
- Check **the example projects** for practical information.
- Check **the documented sources of this component** for detailed information.
- The IC can only process **analog input voltages in the range of GND–0.3 V ... VCC + 0.3 V**. Note that VCC is 3.3V for an ESP32. Else you might fry the device.
- The **sampling frequency** is 250 kHz.
- The component implements the full range of up to 4 **Single Ended Input Signals**  or up to 2 **Differential Input Signals**. The input multiplexer is used to declare which one, and what type of, input signal you want to process (this settings can be changed anytime). The **default Input Multiplexer in the component** is set to "0_GND" and this means one single-ended input signal on pin A0 which is compared to GND. You can change this when configuring the component. Check out the functions ```mjd_ads1115_init()``` and ```mjd_ads1115_set_mux()```.
- The device can read bipolar differential signals but it **cannot process negative voltages on either input signal**.
- The **default Programmable Gain Amplifier** in the component is set to 4.096V and this covers the max voltage of 3.3V (see earlier). You can change this when configuring the component. Check out the functions ```mjd_ads1115_init()``` and ```mjd_ads1115_set_pga()```.
- The **default Output Data Rate** in the component is set to 8 Samples Per Second (#samples per second range 8 .. 860). You can change this when configuring the component. Check out the functions ```mjd_ads1115_init()``` and ```mjd_ads1115_set_data_rate()```. The **conversion time** is related to the samples per second setting (1/X).  The **amount of noise** is relative to the Output Data Rate setting.
- The use of **the ALERT/READY pin** is optional. If it is enabled and wired up then that pin is monitored to determine that a measurement is ready to be read. If it is not enabled then the component uses a calculated delay (based on Data Rate Samples Per Second) before reading the measurement.
- The component implements **Single Measurement Mode **. Note that you can also read conversions at relatively high speed using this mode.
- The component can be used to **read/write all documented properties in the device** registers. Check the source ```mjd_ads1115_defs.h``` for more information.
- **Continuous Conversion Mode** and **Threshold Alerting** is not implemented in this component because that typically requires a tight integration with the main program and those are not a good candidate for a generic component.
- The ADS1115 can still output slightly negative values in case the analog input is close to 0 V (GND) due to device offset.



## Issues

/



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

