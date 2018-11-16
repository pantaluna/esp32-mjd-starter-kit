# ESP-IDF MJD Melexis MLX90393 Triaxis magnetic field sensor (magnetometer)
This is component for the ESP-IDF software framework of the ESP32 hardware from Espressif.

This component is developed for the Melexis MLX90393 magnetic field sensor.

The ESP-IDF component returns the calculated metrics (X axis, Y axis, Z axis, compensated Temperature) and also the raw metric values for XYZ. This means you can also do your own calculations using the raw metric values.

The ESP-IDF component exposes functions to configure the device settings (gain, resolution, digital filter, oversampling, offsets...).

The ESP-IDF component exposes functions to read the read-only system-settings which are stored in the special Melexis area of the NVRAM (tref, sens_tc_lt, sens_tc_ht, ...). 

Lab setup: the MLX90393 is typically used together with a diametrically magnetized magnet with a diameter of 6 - 10 mm and a height of 2.5 - 3 mm. The distance between the magnet and the sensor must be at least 6 mm.

More information:
- Magnetic field sensors and the Hall Effect @ https://www.electronics-tutorials.ws/electromagnetism/hall-effect.html
- A 3D Magnetic Field Scanner using the MLX90393 at https://hackaday.io/project/11865-3d-magnetic-field-scanner#menu-description



## Example ESP-IDF project
my_mlx90393_sensor_using_lib



## Shop Product.
- SparkFun MLX90393 Triple Axis Magnetometer Breakout
- CJMCU-90393 MLX90393 Digital 3D Hall Sensor Displacement Angle Rotate 3D Position Board Module



## Wiring Instructions

### Sensor PIN layout for the I2C data protocol
[Goto the _doc folder for photo's.]

```
INT DRDY Data Ready pin (active high)
SCL
SDA
3.3V
GND
```

### I2C and wire length
- It is important to have stable connections for a breadboard setup: use good quality breadboards (many have bad contacts, especially ones that have been used for a while), use good quality Dupont cables. In my experience a stock Dupont cable of 30cm is too long when using breadboards (especially for the SCL and SDA connections). A 10cm Dupont cable always worked.
- It is better to solder everything together as soon as possible and use quality wires and the best connectors.
- Guidelines https://forum.arduino.cc/index.php?topic=509323.0

### Sensor for the I2C protocol
- Connect device pin 3.3V to the MCU pin VCC (3.3V).
- Connect device pin GND to the MCU pin GND.
- Connect device pin SCL to the MCU pin GPIO#21 (Huzzah32 #21 bottomleft)(Lolin32lite #13 bottomleft).
- Connect device pin SDA to the MCU pin GPIO#17 (Huzzah32 #17 bottomleft-1)(Lolin32lite #15 bottomleft-1).
- Connect device pin INT DRDY Data Ready to the MCU pin GPIO#14 (Huzzah32 #14 bottomleft-1)(Lolin32lite #19 topright-2). This is optional.

- @note It is not needed to wire external 10K pullup resistors from the device pins SCL and SDA to the MCU pin VCC (3V). The device breakout board contains those and they are enabled by default.

## Data Sheet
[Go to the _doc directory for more documents and images.]

https://www.melexis.com/en/product/MLX90393/Triaxis-Micropower-Magnetometer

https://learn.sparkfun.com/tutorials/qwiic-magnetometer-mlx90393-hookup-guide


## Sensor FAQ
- OK 3.3V
- Metrics: X axis, Y axis, Z axis, compensated Temperature
- Supports the I2C protocol and the SPI protocol (up to 10 MHz)
- The breakout board contains 10K pullup resistors for the I2C pins SCL and SDA.
- The breakout board contains no pulldown resistor for the INT DRDY Data Ready pin; so this ESP-IDF component enables the MCU's internal pullup resistor for that pin.
- These are the defaults for the most important parameter values from the NVRAM memory map:
```
    COMM_MODE: 0x0 (0)
    TCMP_EN:   0x0 (0)
    HALLCONF: 0xC (12)
    GAIN_SEL:  0x7 (7)
    Z_SERIES: 0x0 (0)
    BIST: 0x0 (0)
    EXT_TRIG: 0x0 (0)
    TRIG_INT_SEL: 0x0 (0)
    OSR: 0x3 (3)
    DIG_FILT: 0x7 (7)
    RES_XYZ:   X= 0x0 (0) | Y= 0x0 (0) | Z= 0x0 (0)
    SENS_TC_LT: 0x3D (61)
    SENS_TC_HT: 0x45 (69)
    OFFSET_*: X= 0x0 (0) | Y= 0x0 (0) | Z= 0x0 (0)
    TREF: 0xB668 (46696)    
```

- The sensitivity of the XYZ axes is programmable to support different applications (using the parameters GAIN_SEL and RES_XYZ).
- The MLX90393 allows for a total of 32 different scale factors (based on GAIN_SEL and RES_XYZ). The trade-off being full-scale range vs. precision. Unfortunately, the chip does not support auto-ranging / Automatic Gain control on-chip as do the other angular position sensors from Melexis.
- If you use a breakout board then make sure that the components of the breakout board cannot be magnetized, else the readings will never be accurate. This applies for example to the pins and the PCB.
- Your device needs to be calibrated if you want to get very accurate readings. The offset device parameters OFFSET_X, OFFSET_Y and OFFSET_Z are used for to store the constant offset correction, ***independent of temperature***, and programmable for each individual axis where i=X, Y, or Z. The current values in NVRAM are: X=0 Y=0 Z=0. These parameters are used to compensate the offset in case the temperature compensation is ENABLED. These should be initialized with the values the part generates in no magnetic field (this is an expensive setup). Note that these are NOT used with TCMP disabled. Please refer to the application note on the temperature compensation for more info.
- @limitation The component does not implement Temperature Compensation Enabled because it is rarely used. The TCMP Enabled settings make slight changes to the calculations for ambient temperatures below and above 35 degrees Celsius. The readings when Temperature Compensation is disabled are accurate enough for most applications. 
- @limitation The component implements Single Measurement Mode. The Burst Mode or Wake On Change Mode are not implemented because these typically require a tight integration with the main program and those are not a good candidate for a generic component.

## Sensor Protocol Selection I2C/SPI
- Default: I2C.
- The CS pin ("Chip Select") must be connected to VCC to enable exclusively the I2C interface. @important This is ALREADY DONE on this breakout board. 
- The CS pin ("Chip Select") must be connected to GND to enable exclusively the SPI interface (not used in this component).

## Sensor I2C Address
- I2C Address: 0xC (7 bits binary 0001100).
- Use the project my_i2c_scanner to detect the actual I2C address.
- The actual I2C address can be modified by modifying the A0 and A1 jumper settings on the breakout board. They represent the last 2 bit values of the 7-bits I2C address register: binary 0 0 0 0 1 1 A1 A0).

## Sensor I2C protocol
- The sensor acts as a slave.
- The sensor supports the I2C Standard, Fast and High Speed modes (standard mode: 100 Kbit/s, full speed: 400 Kbit/s, fast mode: 1 Mbit/s, high speed: 3.2 Mbit/s).



## Issues


## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

