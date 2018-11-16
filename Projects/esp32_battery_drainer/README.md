# Drain Lion batteries quickly but safely

Drain the 18650 3300mAh Lion battery relatively quickly with a number of 2-3V forward voltage LED's and 10 Ohm resistors.

## Circuit Diagram Simulator

http://www.falstad.com/circuit/circuitjs.html?cct=$+1+0.000005+1.1208435524800693+43+1+50%0A172+-32+48+-32+0+0+6+4.2+4.2+2.75+0+0.5+Voltage%0Aw+608+288+608+352+3%0Ag+608+352+608+384+0%0Ar+48+160+48+240+0+10%0Aw+-32+48+48+48+3%0Aw+48+112+48+160+3%0Aw+48+48+128+48+3%0Aw+128+48+208+48+3%0Aw+208+48+288+48+3%0Aw+288+48+368+48+3%0Aw+448+48+528+48+3%0Aw+368+48+448+48+3%0Aw+208+112+208+160+3%0Aw+288+112+288+160+3%0Aw+368+112+368+160+3%0Aw+448+112+448+160+3%0Aw+528+112+528+160+3%0Aw+128+112+128+160+3%0Aw+48+240+48+288+3%0Ar+208+160+208+240+0+10%0Ar+288+160+288+240+0+10%0Ar+368+160+368+240+0+10%0Ar+448+160+448+240+0+10%0Ar+528+160+528+240+0+10%0Ar+128+160+128+240+0+10%0Aw+208+240+208+288+3%0Aw+288+240+288+288+3%0Aw+368+240+368+288+3%0Aw+448+240+448+288+3%0Aw+528+240+528+288+3%0Aw+128+240+128+288+3%0Aw+48+288+128+288+3%0Aw+208+288+288+288+3%0Aw+128+288+208+288+3%0Aw+368+288+448+288+3%0Aw+448+288+528+288+3%0Aw+288+288+368+288+3%0Aw+528+288+608+288+3%0A162+48+48+48+112+1+2.1+0+1+0+0.02%0A162+208+48+208+112+1+2.1+0+1+0+0.02%0A162+288+48+288+112+1+2.1+0+1+0+0.02%0A162+368+48+368+112+1+2.1+0+1+0+0.02%0A162+448+48+448+112+1+2.1+0+1+0+0.02%0A162+528+48+528+112+1+2.1+0+1+0+0.02%0A162+128+48+128+112+1+2.1+0+1+0+0.02%0A



## Instructions

Put as many in parallel to drain max 500mA. Use Ohms law.

- Take a PCB board.
- Wire the LED + resistor in parallel.
- Wire each set (LED + resistors) in series.



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

