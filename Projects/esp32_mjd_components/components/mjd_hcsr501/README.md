# ESP-IDF MJD HC-SR501 PIR motion sensor component
This is a component for the ESP-IDF software framework for the ESP32 hardware from Espressif.

It has been developed for the following PIR motion sensor models:

- HC-SR501 PIR motion sensor module with lens (the original product). This module is fully configurable without having to solder/unsolder tiny SMD resistors.
- MH-SR602 Micro PIR motion sensor module with lens (even smaller). The pin order is different (the pins on the PCB are labeled).

- AM312 Mini PIR motion sensor module with lens (smaller). This module is very sensitive to noise and therefore not recommended for new projects.



## Example ESP-IDF project
my_hcsr501_pir_sensor_using_lib

## Shop Product
- HC-SR501 PIR motion sensor module with lens.
- AM312 Mini PIR motion sensor module with lens.
- MH-SR602 Micro PIR motion sensor module with lens.



## Wiring Instructions

### Sensor PIN layout (backside)
```
1 GND
2 DATA
3 VCC
```

### Sensor wiring up
- Connect device pin VCC to the MCU pin VCC 3.3V.
- Connect device pin GND to the MCU pin GND.
- Connect device pin DATA/OUT to a MCU GPIO#14 (Huzzah32 #14 bottomright-2).
- @important A 10K pullup resistor on the DATA PIN is not needed because the mjd_hcsr501 software component enables the ESP32's internal pullup resistor for that pin.
- Connect a ferrite bead on the wire to the device pin VCC, and as close as possible to the device. This can be a ferrite clamp, or an axial ferrite with a wires through it.
- Connect a ferrite bead on the wire to the device pin DATA, and as close as possible to the device. This can be a ferrite clamp, or an axial ferrite with a wires through it.
- Wire up a 100uF ceramic capacitor between the pins VCC and GND of the module (as close as possible).



## Data Sheet
[Go to the _doc directory for more documents and images.]

https://www.mpja.com/download/31227sc.pdf



## Sensor Voltage Levels
- **Operating Voltage**: the specs say DC 4.5V-20V but the board also works with DC 3.3V (OK for ESP32 boards).
- Voltage level on the **DATA PIN**: DC 3.3V (OK for ESP32 boards).



## Data Pin Logic (no protocol)
- DATA Pin logic: High 3.3V, Low 0V.
- The pin is HIGH (3.3V) when movement is detected (and stays so for X seconds during the time decay period). The time decay period is typically 2.5 seconds.
- The pin is LOW (0V) when no movement is detected.



## Time Decay potentiometer (only on the HC-SR501 model)
- The pot on the left (see image).
- Defines how long the signal stays ON (and blocks further detections) after it has detected a movement. 
- Recommended setting: minimum 3 seconds.
- Minimum = 3 seconds (fully left).
- Maximum = 300 seconds(fully right).
- @important When the time decay ends then the motion detector is disabled for 3 seconds; the output signal will go Low.



## Distance Sensitivity potentiometer (only on the HC-SR501 model)
- The pot on the right (see image).
- Implies how far the sensor works. Minimum=3 meters. Maximum=8 meters.
- Recommended setting: Minimum=3 meters.
- Fully left: max range 7 meter, increases sensitivity.
- Fully right: max range 3 meter, decreases sensitivity.




## Jumper "Trigger Mode" (only on the HC-SR501 model)
- Recommended to use the Single trigger mode ("L").
- Modes:
  - Single Trigger Mode ("L"): output goes HIGH then LOW when triggered during the TIME DECAY period. Continuous motion results in repeated HIGH/LOW pulses. Output is LOW when idle.
  - Repeatable Trigger Mode ("H"): output remains HIGH when sensor is retriggered repeatedly during the TIME DECAY period. Output is LOW when idle.
- Jumper in the "L" position => uses Single Trigger Mode. Jumper-connect the two pins that are the closest to the corner of the board.

- Jumper in the "H" position => uses Repeatable trigger mode. Jumper-connect the two pins that are the furthest away from the corner the board.



## Sensor FAQ
- Sensing angle: 110 degrees.
- The sensor is designed to adjust to slowly changing conditions that would happen normally as the day progresses and the environmental conditions change.
- The module contains the BISS0001 PIR motion detector IC. It processes the output of the analog sensor and transforms it in a digital signal.
- The module contains the Holtek HT7133-1 regulator IC. It has a 1.7V Voltage Dropout, which means it reduces voltage from 5V to 3.3V :)

- The sensor is only ready for use after 60 seconds after being powered-on. The sensor might output HIGH several times during that period. There should be as little motion as possible in the sensor's field of view during that period.
- The device does NOT suffer from contact bounce.
- The PIR (Passive Infra-Red) Sensor is a pyroelectric device that detects motion by measuring changes in the infrared levels emitted by surrounding objects. This motion can be detected by checking for a high signal on a single I/O pin. It measures heat levels so moving a wooden stick does not make it go *ON.
- The sensor is sensitive to strong light sources (sunlight) and sudden wind flow (air outlets, airco's).



## Issues

* Optional: battery powered PIR: instructions @ https://forum.mysensors.org/topic/1088/battery-powered-pir



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

