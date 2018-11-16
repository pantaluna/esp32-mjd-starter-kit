# ESP-IDF MJD HC-SR501 PIR motion sensor component
This is component based on ESP-IDF for the ESP32 hardware from Espressif.



## Example ESP-IDF project
my_hcsr501_pir_sensor_using_lib

## Shop Product
HC-SR501 PIR Human Infrared Sensor Module Including Lens



## Wiring Instructions

### MCU
a. Adafruit HUZZAH32
- Pin #13 = Blue LED on the PCB

b. Wemos Lolin32 Lite
- Pin #22 = Blue LED on the PCB

### Sensor PIN layout (backside)
```
1 GND
2 DATA
3 VCC
```

### Sensor wiring up
- Connect device pin VCC to the MCU pin VCC 3V.
- Connect device pin GND to the MCU pin GND.
- Connect device pin DATA/OUT to a MCU GPIO#14 (Huzzah32 #14 bottomright-2)(Lolin32lite #19 topright+2).



## Data Sheet
[Go to the _doc directory for more documents and images.]

https://www.mpja.com/download/31227sc.pdf



## Sensor Voltage Levels
- Operating Voltage: the spec says DC 4.5V-20V but the board also works with DC 3.3V (OK for ESP32 boards).
- Voltage level on the DATA PIN: DC 3.3V (OK for ESP32 boards).



## Data Pin Logic (no protocol)
- DATA Pin logic: High 3.3V, Low 0V
- The pin is HIGH (3.3V) when movement is detected (and stays so for X seconds during the time decay period).
- The pin is LOW (0V) when no movement is detected.
- The wiring requires a 10K pullup resistor (DATA->VCC).
- @important No extra pullup resistor needed on the DATA PIN because I have enabled the ESP32 internal pullup resistor for the data pin in the software.



## Time Decay potentiometer
- Dial the pot (the left one) to fully-left which is its minimum time decay.
- Recommended setting: minimum 3 seconds.
- Defines how long the signal stays ON (and blocks further detections) after it detects a movement. Minimum = 3 seconds. Maximum = 300 seconds.



## Distance Sensitivity potentiometer
- Dial the pot (the right one) to fully-left for its maximum range.
- Recommended setting: Minimum=3 meters.
- Implies how far the sensor works. Minimum=3 meters. Maximum=8 meters.
    + Clockwise = Increase sensitivity. Fully right: the range is max 7 meter.
    + Counter-Clockwise = Decrease sensitivity. Fully left: the range is max 3 meter.


## Jumper "Trigger Mode"
- Recommended to use the Single trigger mode ("L").
- Repeatable trigger mode ("H"): output remains HIGH when sensor is retriggered repeatedly during the TIME DECAY period. Output is LOW when idle.
- Single trigger mode ("L"): output goes HIGH then LOW when triggered during the TIME DECAY period. Continuous motion results in repeated HIGH/LOW pulses. Output is LOW when idle.
- Jumper in the "H" position => uses a repeatable trigger.    "H" position: connect the two pins that are the furthest away from the corner the board.
- Jumper in the "L" position => uses a normal single trigger. "L" position: connect the two pins that are the closest to the corner of the board.



## Sensor FAQ
- The sensor is powered up and ready after 60 seconds. The sensor might output HIGH several times during that period. There should be as little motion as possible in the sensor's field of view during that period.
- The device does NOT suffer from contact bounce (which causes interrupt handling of GPIO_INTR_ANYEDGE GPIO_INTR_POSEDGE GPIO_INTR_NEGEDGE to not work properly).
- The PIR (Passive Infra-Red) Sensor is a pyroelectric device that detects motion by measuring changes in the infrared levels emitted by surrounding objects. This motion can be detected by checking for a high signal on a single I/O pin. It measures heat levels so moving a wooden stick does not make it go *ON.
- Sensing angle: 110 degrees.
- The sensor is designed to adjust to slowly changing conditions that would happen normally as the day progresses and the environmental conditions change.
- The sensor is sensitive to sudden wind flow and strong closeby light sources.
- The module contains the IC BISS0001 PIR motion detector. It processes the output of the analog sensor and transforms it in a digital signal.
- The module contains the IC Holtek HT7133-1 regulator. It has a 1.7V Voltage Dropout, which means it reduces voltage from 5V to 3.3V :)



## Issues

* Optional: battery powered PIR: instructions @ https://forum.mysensors.org/topic/1088/battery-powered-pir



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

