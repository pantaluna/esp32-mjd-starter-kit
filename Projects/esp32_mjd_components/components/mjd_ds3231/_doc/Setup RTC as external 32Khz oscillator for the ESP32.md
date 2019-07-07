# SOP: setup the RTC board as an external oscillator 32Khz for the ESP32

## General
The ESP32's internal 150Hz crystal oscillator is not very accurate for e.g. data loggers which wake up from deep sleep at exact regular intervals. It deviates +-1 minute every 4 hours.

Luckily the ESP32 also supports a more accurate RTC external 32kHz oscillator.

The ESP32 hardware design guidelines (as documented in KConfig ESP32_RTC_CLOCK_SOURCE) state that the external clock signal must be connected to the 32K_XP pin. The amplitude should be < 1.0V in case of a square wave signal. The common mode voltage should be 0.1 < Vcm < 0.5Vamp, where Vamp is the signal amplitude. A 1 - 10 nF capacitor must be connected between 32K_XN pin and GND.

**The 32K pin of the RTC board** is the square wave output signal of the 32K oscillator. It is **open-drain** so you need to attach **a (10K) pullup resistor** to read this signal correctly from a microcontroller pin.

@important The ZS-042 RTC board and the Chronodot RTC board can be used for this purpose. The black/yellow RTC board cannot be used because the 32K output pin of the DS3231 chip is not exposed on that breakout board.

@important The setup to use the DS3231 as an RTC Real Time Clock via the I2C protocol, as described in the main document for each RTC board, is not required.

@important A battery on the RTC board is not required if you only use this board as an external 32Khz oscillator for the ESP32.



## Wiring
```
- RTC board pin VCC => ESP32 pin VCC 3.3V
- RTC board pin GND => ESP32 pin GND
- RTC board pin 32K => 10K pullup resistor => a power rail < 1.0V > 0.5V
- ESP32 pin GPIO#32 (pin 32K_XP) => RTC board pin 32K
- ESP32 pin GPIO#33 (pin 32K_XN) => 1-10nF ceramic capacitor => GND
```

A power rail of 0.8V can be made from VCC 3.3V using a voltage divider with Rtop=100K and Rbottom to 33K.

https://learn.adafruit.com/adafruit-ds3231-precision-rtc-breakout



## Setup

- Goto the project directory, for example `cd ~/esp32_ds3231_clock_using_lib`
- Run `make menuconfig`.
- Select "Component config  ---> ESP32-specific  ---> RTC clock source ()  --->".
- Change from "Internal 150kHz RC oscillator" to "(X)  **External 32kHz oscillator**".
- Exit menuconfig.
- `make flash monitor`



## Notes
- **Status Register (0Fh) Bit 3 "Enable 32kHz Output (EN32kHz)"**: this bit controls the status of the 32kHz pin. When set to logic 1, the 32kHz pin is enabled and outputs a 32.768kHz square-wave signal. When set to logic 0, the 32kHz pin goes to a high-impedance state. **The initial power-up state of this bit is logic 1, and a 32.768khz square-wave signal appears at the 32khz pin** after power is applied to the DS3231.

