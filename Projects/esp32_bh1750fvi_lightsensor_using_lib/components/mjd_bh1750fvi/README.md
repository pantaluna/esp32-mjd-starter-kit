# ESP-IDF MJD BH1750FVI light sensor component
This is component based on ESP-IDF for the ESP32 hardware from Espressif.

## Example ESP-IDF project
esp32_bh1750fvi_lightsensor_using_lib

## Shop Product.
GY-032 BH1750FVI Digital Light Intensity Sensor Module For AVR Arduino 3V-5V

## WIRING INSTRUCTIONS
### MCU
a. Adafruit HUZZAH32
- Pin #13 = Blue LED on the PCB

b. Wemos Lolin32 LiteLolin32 Lite
- Pin #22 = Blue LED on the PCB

### Sensor PIN layout
```
1   VCC
2   GND
3   SCL
4   SDA
5   ADDR
```

### Sensor for the I2C protocol
- Connect device pin 1 VCC to the MCU pin VCC 3V.
- Connect device pin 2 GND to the MCU pin GND.
- Connect device pin 3 SCLK to a MCU GPIO#21 ((Huzzah32 #21 bottomleft)(Lolin32lite #13 bottomleft)
- Connect device pin 4 SDA to a MCU GPIO#17  ((Huzzah32 #17 bottomleft-1)(Lolin32lite #15 bottomleft-1)

## Data Sheet
[Go to the _doc directory for more documents and images.]

http://www.mouser.com/ds/2/348/bh1750fvi-e-186247.pdf

## Sensor FAQ
- OK 3.3V
- I2C protocol. I2C default device address = 0x23.
- Maximum I2C SCL Clock Frequency: 400 kHz
- The driver uses the default sensitivity of 1.0.
- Lux unit: darkness is typically less than 10 lux.

## Sensor & I2C protocol
- I2C device slave address: 0x23.
- It is possible to change the I2C slave-address with PIN 5 ADDR: \
      Low  (<= 0.3V) = 0x23 0b0100011 ***DEFAULT \
      High (>= 0.7V) = 0x5C 0b1011100
- Sensor supports the I2C Standard Speed 100 Kbit/s and Full Speed 400 Kbit/s; not Fast Speed 1 Mbit/s and High Speed 3.2 Mbit/s.
- The ESP32 maximum I2C clock speed is 1Mhz.

## Sensor known ISSUES
