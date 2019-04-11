# Adafruit ChronoDot V2.0
Differences with the ZS042 board:
- Much smaller board.
- It does NOT contain the EEPROM 24C342 memory chip from Atmel. Which I do not need.
- It does not contain a battery charger circuit. Which I do not need.
- It consumes less current.

## Example ESP-IDF project
my_ds3231_clock_using_lib

## Shop Product.
1pcs RTC real-time clock module DS3231SN ChronoDot V2.0 I2C for Arduino Memory DS3231 module

https://www.adafruit.com/product/255

### Board PIN layout
Pins @ left side:

```
2 32K
4 RST (not used)
```

Pins @ right side:

```
1 GND
2 VCC
3 I2C SCL
4 I2C SDA
```

See also the images.

### Wiring instructions for using the I2C protocol
- Connect RTC board pin VCC to the MCU pin VCC 3V.
- Connect RTC board pin GND to the MCU pin GND.
- Connect RTC board pin SCL to a MCU GPIO#21 (Huzzah32 #21 bottomleft)(Lolin32lite #13 bottomleft).
- Connect RTC board pin SDA to a MCU GPIO#17 (Huzzah32 #17 bottomleft-1)(Lolin32lite #15 bottomleft-1).

## Sensor I2C Address
- I2C Device address 0x68 = IC RTC Maxim Integrated DS3231.

## Sensor I2C protocol
- Sensor acts as a slave.

## Data Sheet
[Go to the _doc directory for more documents and images.]

http://docs.macetech.com/doku.php/chronodot_v2.0

https://www.maximintegrated.com/en/products/digital/real-time-clocks/DS3231.html

## SOP: setup the ChronoDot board as an external oscillator 32Khz for the ESP32

Goto document "Setup as external oscillator 32Khz for the ESP32.md"

## FAQ

- OK 3.3V - 5V.
- The battery holder is designed for a CR1220 3V lithium coin cell.
- Accuracy: 1 second.
- The contents of the time and calendar registers are in *the binary-coded decimal (BCD) format*.
- The board does not contain pull-up resistors when using the I2C protocol but those are needed because the ESP32's internal pullups are activated in the ds3231 component.
- The DS3231 can be run in either 12-hour or 24-hour mode. Bit 6 of the hours register is defined as the 12- or 24-hour mode select bit. \
     When high, the 12-hour mode is selected, and bit 5 is the AM/PM bit with logic-high being PM. \
     ***When low,  the 24-hour mode is selected***, and bit 5 is the 20-hour bit (20–23 hours).
- The countdown chain is reset whenever the seconds register is written. Write transfers occur on the acknowledge from the DS3231. \
    Once the countdown chain is reset, to avoid rollover issues ***the remaining time and date registers must be written within 1 second***.
- Control Register (0Eh) Bit 7: Enable Oscillator (EOSC). \
      ​    When set to logic 0, the oscillator is started. \
      ​    When set to logic 1, the oscillator is stopped when the DS3231 switches to VBAT. \
      ​    @default This bit is clear (logic 0) when power is first applied. \

## Known ISSUES
- The board does not fit on a small breadboard if you want to use the pin headers on both the left side (power + I2C pins) and the right side (32K pin). 
