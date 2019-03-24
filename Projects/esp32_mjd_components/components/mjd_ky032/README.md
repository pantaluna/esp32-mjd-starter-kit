# ESP-IDF MJD KY-032 Infrared Obstacle Avoidance Sensor component
This is component based on ESP-IDF for the ESP32 hardware from Espressif.



## Example ESP-IDF project
esp32_obstacle_sensor_using_lib

## Shop Product
KY-032 Infrared Obstacle Avoidance Sensor For Arduino Smart Car Robot



## Wiring Instructions
### MCU Adafruit HUZZAH32
Pin #13 = Blue LED on the PCB

### Sensor PIN layout (frontside)
```
1 DATA OUT
2 GND
3 VCC
```

### Sensor wiring up
- Connect device pin DATA/OUT to a MCU GPIO#14 (Huzzah32 #14 bottomright-2)(Lolin32lite #19 topright+2)
- Connect device pin GND to the MCU pin GND.
- Connect device pin VCC to the MCU pin VCC 3V.
- 

## Data Sheet
[Go to the _doc directory for more documents and images.]

http://henrysbench.capnfatz.com/henrys-bench/arduino-sensors-and-input/arduino-ir-obstacle-sensor-tutorial-and-manual/

http://irsensor.wizecode.com/

https://www.youtube.com/watch?v=gRtdcxOXojo

https://arduino.stackexchange.com/questions/9082/how-to-increase-the-detection-distance-on-arduino-ky-032-obstacle-avoidance-sens



## Sensor Voltage Levels
- Operating Voltage: DC 3.3V - 5V :)
- Voltage level on the DATA PIN: DC 3.3V (OK for ESP32 boards).
- 

## Data Pin Logic (no protocol)
- The pin is HIGH (3.3V) when no movement is detected.
- The pin is LOW (0V) when real movement is detected.
- The wiring requires a 10K pullup resistor (DATA->VCC). 
  @important No extra pullup resistor is needed on the DATA PIN because I have enabled the ESP32 internal pullup resistor for the data pin in the software.



## Distance potentiometer
- Recommended setting: positioned at 50%.
- Counter clockwise decreases distance. At 0% the pin value is always high (1) and it will detect nothing.
- Clockwise increases distance. At 100% the pin value is always low (0) and it signals it detects an obstacle all the time.



## Sensor FAQ
- The red LED (the left one) on the PCB will light up when an obstacle is detected.
- IR TX module: the NE555 chip generates a 38kHz square wave. The 38kHz signal is used to illuminate an Infrared LED.
- IR RX module: the Vishay HS0038BD chip.



## Issues
- The detection distance is very limited, e.g. up to 2cm. If you want a better quality product then shop for the Adafruit Sharp GP2Y0D810Z0F Digital Distance Sensor with Pololu Carrier (https://www.adafruit.com/product/1927).
- The device does suffer from contact bounce (which causes interrupt handling of GPIO_INTR_ANYEDGE GPIO_INTR_POSEDGE GPIO_INTR_NEGEDGE to not work properly). You have to implement debounce logic in software (this is done for you in this component).



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

