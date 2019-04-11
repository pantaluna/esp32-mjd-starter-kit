# ZS042 blue large PCB model DS3231 RTC Real Time Clock Module
This board is typically used as an RTC Real Time Clock.

It can also be used as an accurate external crystal oscillator 32Khz for the ESP32.

## Example ESP-IDF project
my_ds3231_clock_using_lib

## Shop Product.
DS3231 ZS042 RTC Real Time Clock Module

## Board PIN layout
```
1 32K : only used when the board is used as an external crystal oscillator
2 SQW : not used
3 SCL : I2C Clock signal
4 SDA : I2C Data signal
5 VCC : 3.3V
6 GND : Ground
```
See also the diagram images.

## Remove resistors to reduce current usage and make the module safer
- Remove the charging circuit resistor and the power LED resistor from the circuit board. Open the image "ZS042 DS3231 RTC Real Time Clock -  Remove these 2 resistors.jpg" in the "_doc" directory for the exact location of these resistors on the board. Detailed information @ https://thecavepearlproject.org/2016/10/27/diy-arduino-promini-data-logger-2016-build-update/

### Wiring instructions for using the I2C protocol
- Connect RTC board pin VCC to the MCU pin VCC 3V.
- Connect RTC board pin GND to the MCU pin GND.
- Connect RTC board pin SCL to a MCU GPIO#21 (Huzzah32 #21 bottomleft)(Lolin32lite #13 bottomleft).
- Connect RTC board pin SDA to a MCU GPIO#17 (Huzzah32 #17 bottomleft-1)(Lolin32lite #15 bottomleft-1).

## Sensor I2C Address
- I2C Device address 0x68 = the RTC Maxim Integrated DS3231.
- I2C Device address 0x57 = the EEPROM Atmel 24C324. This chip is not used directly. The A0, A1 and A2 pads (open by default)) are used for changing the I2C address of the EEPROM memory chip. It is OK to leave the pads open. I have no information how these pads affect the I2C address.

## Sensor I2C protocol
- Sensor acts as a slave.

## SOP: setup the ZS042 board as an external crystal oscillator 32Khz for the ESP32

Goto document "Setup as external oscillator 32Khz for the ESP32.md"

## Data Sheet

[Go to the _doc directory for more documents.]

https://www.maximintegrated.com/en/products/digital/real-time-clocks/DS3231.html

## FAQ
- OK 3.3V - 5V.
- The board requires a CR2032 Lithium battery (non rechargeable!).
- Crystal oscillator frequency = 32kHz.
- Accuracy: 1 second.
- The contents of the time and calendar registers are in *the binary-coded decimal (BCD) format*.
- The board contains the required pull-up resistors when using the I2C protocol. This includes the I2C bus pins and the 32K and SQW pins. Nice :)

- Contains 1x chip "DS3231 Extremely Accurate I2C-Integrated RTC/TCXO/Crystal" (the biggest chip on the PCB) from vendor Maxim Integrated 
- Contains 1x EEPROM 24C342 memory chip from Atmel (the smallest chip on the PCB) which is not used. http://www.microchip.com/wwwproducts/en/en010361

- The DS3231 can be run in either 12-hour or 24-hour mode. Bit 6 of the hours register is defined as the 12- or 24-hour mode select bit. \
     When high, the 12-hour mode is selected, and bit 5 is the AM/PM bit with logic-high being PM. \
     ***When low,  the 24-hour mode is selected***, and bit 5 is the 20-hour bit (20–23 hours).
- The countdown chain is reset whenever the seconds register is written. Write transfers occur on the acknowledge from the DS3231. \
    Once the countdown chain is reset, to avoid rollover issues ***the remaining time and date registers must be written within 1 second***.
- Control Register (0Eh) Bit 7: Enable Oscillator (EOSC). \
          When set to logic 0, the oscillator is started. \
          When set to logic 1, the oscillator is stopped when the DS3231 switches to VBAT. \
          This bit is clear (logic 0) when power is first applied. \
          ***When the DS3231 is powered by VCC, the oscillator is always on regardless of the status of the EOSC bit.***

## Important: use CR2032 3V non-rechargeable Lithium Batteries and disable the battery charging circuit
- The DS3231 RTC and 24C32 EEPROM chips themselves do nothing to manage the charging of the battery.
- The ZS-042 board's additional battery charging circuit does not work when using a 3.3V microcontroller board; the charging circuit will only waste power. The circuit is also unsafe when using a 5V microcontroller board in combination with LIR2032 batteries.
- So please use CR2032 3V non-rechargeable Lithium batteries; a CR2032 will backup the RTC for many years. Do NOT use LIR2032 3.6V Li-Ion Rechargeable batteries because a) the battery will not charge at all; b) They cost significantly more than standard C2032 batteries.

## Known ISSUES
- The battery charger circuit does not work for 3.3V microcontrollers and LIR2032 batteries.

