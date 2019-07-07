# Small black (with yellow capacitor) for Raspberry Pi RTC Real Time Clock Module.
This board is typically used as an RTC Real Time Clock.

It is designed for fitting on a header of the 5V Raspberry PI (https://pinout.xyz/pinout/i2c) but it also works fine on 3.3V microcontrollers.

Differences with the ZS042 board:
- It cannot be used as an accurate external 32Khz oscillator for the ESP32 because the 32K pin is not exposed.
- It does NOT contain the EEPROM 24C342 memory chip from Atmel (which we do not need).



## Example ESP-IDF project
esp32_ds3231_clock_using_lib



## Shop Product.
DS3231 for Raspberry Pi DS3231 RTC Real Time Clock Module



### Board PIN layout
```
1 VCC
2 I2C SDA
3 I2C CLK
4 Not Used
5 GND
```

See also the images.



### Wiring instructions for using the I2C protocol
- Connect RTC board pin VCC to the MCU pin VCC 3V.
- Connect RTC board pin SDA to a MCU GPIO#17 (Huzzah32 #17 bottomleft-1).
- Connect RTC board pin SCL to a MCU GPIO#21 (Huzzah32 #21 bottomleft).
- Connect RTC board pin GND to the MCU pin GND.



## Sensor I2C Address
- I2C Device address 0x68 = RTC Maxim Integrated DS3231.



## Sensor I2C protocol
- Sensor acts as a slave.



## SOP: setup the RTC board as an external 32Khz oscillator for the ESP32

Not possible.



## Data Sheet
[Go to the _doc directory for more documents.]

https://www.maximintegrated.com/en/products/digital/real-time-clocks/DS3231.html



## FAQ
- OK 3.3V - 5V.
- The board contains a CR927 Lithium battery (not rechargeable); it is tack-welded so it cannot be removed/replaced easily. A CR1032 battery also fits.
- Accuracy: 1 second.
- The contents of the time and calendar registers are in *the binary-coded decimal (BCD) format*.
- The board contains the required pull-up resistors when using the I2C protocol.
- Contains 1x chip "DS3231 Extremely Accurate I2C-Integrated RTC/TCXO/Crystal" (the biggest chip on the PCB) from vendor Maxim Integrated 
- The DS3231 can be run in either 12-hour or 24-hour mode. Bit 6 of the hours register is defined as the 12- or 24-hour mode select bit.
     When high, the 12-hour mode is selected, and bit 5 is the AM/PM bit with logic-high being PM.
     ***When low,  the 24-hour mode is selected***, and bit 5 is the 20-hour bit (20–23 hours).
- The countdown chain is reset whenever the seconds register is written. Write transfers occur on the acknowledge from the DS3231.
    Once the countdown chain is reset, to avoid rollover issues ***the remaining time and date registers must be written within 1 second***.
- Control Register (0Eh) Bit 7: Enable Oscillator (EOSC).
          When set to logic 0, the oscillator is started.
          When set to logic 1, the oscillator is stopped when the DS3231 switches to VBAT.
          This bit is clear (logic 0) when power is first applied.
          ***When the DS3231 is powered by VCC, the oscillator is always on regardless of the status of the EOSC bit.***



## Known ISSUES
None.

