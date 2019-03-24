# ESP-IDF MJD AM2320 meteo sensor component
This is component based on ESP-IDF for the ESP32 hardware from Espressif.



## Example ESP-IDF project
my_am2320_temperature_sensor_using_lib



## Shop Product
AM2320 DC 3.1-5.5V Digital Capacitive Temperature And Humidity Sensor Module Single Bus / I2C.

Sensor AM2320 of the company Aosong.



## Wiring Instructions
### MCU
a. Adafruit HUZZAH32
- Pin #13 = Blue LED on the PCB
- Pin #14 = AM2320 sensor DATA pin (Huzzah32 GPIO#14: bottomright -2)

b. Wemos Lolin32 Lite
- Pin #22 = Blue LED on the PCB
- Pin #19 = AM2320 sensor DATA pin (Lolin32lite GPIO#13: topright +2)



### Sensor PIN layout
```
1   VCC
2   SDA / DATA
3   GND
4   SCL
```



### Sensor AM2320 for the custom 1-WIRE protocol (not the I2C protocol)

- Connect pin 1 VCC of the sensor to the MCU pin VCC 3V.
- Connect pin 2 SDA of the sensor to the MCU pin DATA (whatever pin you selected above).
- Connect pin 3 GND of the sensor to the MCU pin GND.
- Connect pin 4 SCL of the sensor to the MCU pin GND. \
  @important This signals to the sensor to use the 1-WIRE protocol, not the I2C protocol.
- Connect a +-5.1K PULL-UP resistor from the sensor pin 2 SDA to the MCU pin VCC 3.3V \
  @important You will often get checksum errors without this pullup resistor.
- OPTIONAL: connect a 100nF capacitor between pin 1 VCC and pin 3 GND of each sensor for power filtering. \
  @doc https://electronics.stackexchange.com/questions/2272/what-is-a-decoupling-capacitor-and-how-do-i-know-if-i-need-one



## Data Sheet
[Go to the _doc directory for more documents and images.]

https://akizukidenshi.com/download/ds/aosong/AM2320.pdf



## Aosong AM2320 sensor FAQ
- The AM2320 sensor is an Aosong DHT22 sensor with additionally support for a custom I2C protocol. The 1-Wire protocol is compatible with that of the Aosong DHT22 sensor.
- OK 3.3V
- 1-Wire custom protocol.
- Can use the ESP-IDF RMT driver.
- Metrics: temperature (unit Celsius in float), humidity (unit Percentage in float).
- Supports temperature range -40..+80 degrees Celsius (the DHT11 sensor does no negative degrees Celsius).
- The startup time of the sensor after power-on is 1 second.
- Recommended minimum reading time interval: 1x / minute.
- Special: an external pull-up resistor is required.



## Aosong AM2320 known ISSUES
- The I2C protocol of the device is a custom one (the standard ESP32 I2C driver does NOT work).



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

