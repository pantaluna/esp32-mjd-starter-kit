# ESP-IDF MJD DHT11 meteo sensor component
This is component based on ESP-IDF for the ESP32 hardware from Espressif.

## Example ESP-IDF project
esp32_dht11_temperature_sensor_using_lib

## Shop Product
KY-015 DHT11 Temperature Humidity Sensor Module For Arduino.

## WIRING INSTRUCTIONS
### MCU
a. Adafruit HUZZAH32
- Pin #13 = Blue LED on the PCB
- Pin #14 = sensor data pin (Huzzah32 GPIO#14: bottomright -2)

b. Wemos Lolin32 Lite
- Pin #22 = Blue LED on the PCB
- Pin #19 = sensor data pin (Lolin32lite GPIO#13: topright +2)

### Sensor PIN layout
```
1 VCC
2 OUT
3 GND
```

### Sensor for the custom 1-WIRE protocol
- Connect pin 1 VCC of the sensor to the MCU pin VCC 3V.
- Connect pin 2 OUT of the sensor to the MCU pin DATA (whatever pin you selected above).
- Connect pin 3 GND of the sensor to the MCU pin GND.

- Connect a 5K PULL-UP resistor from the sensor pin 2 OUT to the MCU pin VCC 3.3V  \
  @important You will often get checksum errors without this pullup resistor.

- OPTIONAL: connect a 100nF capacitor between pin 1 VCC and pin 3 GND of each sensor for power filtering. \
  @doc https://electronics.stackexchange.com/questions/2272/what-is-a-decoupling-capacitor-and-how-do-i-know-if-i-need-one \

## Data Sheet
[Go to the _doc directory for more documents and images.]

https://akizukidenshi.com/download/ds/aosong/DHT11.pdf

## Sensor FAQ
- OK 3.3V
- Metrics: temperature, humidity.
- Uses a custom 1-Wire protocol.
- The startup time of the sensor after power-on is 1 second.
- Recommended minimum reading time interval: 1x / minute.

## Sensor known ISSUES
- Only supports temperatures above 0 degrees Celsius.
- Burden: external pull-up resistors are always required.
- Burden: does not support the I2C protocol (but the timing sensitive 1-Wire protocol).
