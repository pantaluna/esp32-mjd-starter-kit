# ESP-IDF MJD LED RGB LED component for Worldsemi WS2812, WS2812B and WS2813x
This component has been developed to make it easy to work with the RGB LED packages of the manufacturer Worldsemi. The most popular product line that uses these RGB LED packages is the Adafruit Neopixels. The component will also work for simple environments such as a few RGB LED's on a PCB.

The component performs very well because it uses the fast RMT peripheral of the ESP32 hardware. The ESP-IDF RMT Driver (a standard ESP-IDF component) is configured by this component to generate signal pulses with an accuracy of 50 nanoseconds; that is accurate enough for these RGB LED chips. The accuracy can be increased to 25 nanoseconds if that is required for future products.

Your app can initialize & de-initialize & work with up to 8 LED strips at the same time, and get/set the color of each LED of the LED strip.

The app has to specify the type of RGB LED package, the number of LED's, the GPIO Output PIN number which is wired to the DATA IN pad of the LED strip, and the relative brightness (optional).

The component exposes helper functions to reset all LEDs of a strip to black (zero), and to identify the position of each LED of a LED array.

You can use these building blocks to develop special effects algorithms for your LED strip.

App programming considerations:
- Change the CPU Frequency from 160 Mhz (=default) to 240 Mhz for optimal performance, especially for long LED strips. You can change this setting in `make menuconfig` Component => ESP-32 Specific => CPU Frequency.
- The ESP LOG LEVEL must be set to INFO (or WARN or ERROR) for production. However, the ESP LOG LEVEL can be set to DEBUG or VERBOSE for debugging; this dumps a lot of data to the UART, so the LED timings will be wrong, but at least the data dumps are correct and you can see exactly in the logs what is going on.
- Tip: use a logic analyzer, or better an oscilloscope, to analyze the timings of the data signal from the MCU to the LED Board.

Future development: support RGBW LED's, make helper funcs to "write" text on large LED matrixes.



## Supported RGB LED packages
- Worldsemi WS2812
- Worldsemi WS2812B ***the most popular RGB LED package in 2017***
- Worldsemi WS2813A, WS2813B, WS2813C, WS2813D.
- The Adafruit Neopixels product line which integrates the above mentioned WSx packages.



Some characteristics:
+ WS2812: RGB LEDs, SMD 6 legs, 5V only (this one does not support 3.3V power nor 3.3V data signals so it can only be used with an ESP32 in combination with a 3.3V->5V logic level shifter board).
+ WS2812B: RGB LEDs, SMD 4 legs, supports 3.3V and 5V.
+ WS2813A, WS2813B, WS2813C, WS2813D: RGB LEDs, SMD 6 legs, supports 3.3V and 5V, separate power pins for the IC and the LED's, dual-signal wires meaning it has an extra BIN Backup Input Data Pin. The variations A..D feature a different maximum brightness of the LED's and hence a different power consumption.



## Example ESP-IDF project
my_ledrgb_using_lib

This example project work for all shop products mentioned in this article.



## Shop Products
- CJMCU   1 Bit WS2812 5050 RGB LED Driver Development Board
- CJMCU   4 Bit WS2812 5050 RGB LED Driver Development Board
- CJMCU   8 Bit WS2812 5050 RGB LED Driver Development Board
- CJMCU 4x4 Bit WS2812 5050 RGB LED Driver Development Board
- BTF-LIGHTING WS2813 led pixel strip, DC 5V, length 1m/4m/5m, 30/60/144 pixels/strip/m, IP30/IP65/IP67



I would suggest to first get some experience with a small LED board.

Notes:
- The CJMCU 1 Bit, 4 Bit and 4x4 Bit products listed above are advertised as "WS2812" LED chips but they are actually "WS2812B" LED chips (the "B" ones are newer and better).
- The CJMCU 8 Bit product contains old "WS2812" LED chips (no "B"). The timings are different...
- The BTF-LIGHTING WS2813 led pixel strip contains "WS2813B" LED chips.



## Data Sheets
### Data Sheet WS2812B
[Go to the _doc directory for more documents and images.]

http://www.world-semi.com/DownLoadFile/108

http://www.world-semi.com/solution/list-4-1.html

### Data Sheet WS2813A WS2813B WS2813C WS2813D
[Go to the _doc directory for more documents and images.]

http://www.world-semi.com/solution/list-4-1.html



## Soldering instructions, if the board/strip comes without wires
[See images in the _doc directory]

Most smaller LED boards often just have solder pads on the back.

A LED board with WS2812B's has these PIN connections: VCC, GND, DATA IN. If 2 GND pads are present on the board then you only have to wire up one pad.



Instructions for small WS2812B LED boards with no wires:
- Use 3 male-male Dupont wires.
- Solder the male pin of each Dupont wire to an input pad on the LED board: VCC, GND, DATA IN.
- The male connectors on the other side of the Dupont cable are used later to hook up the breadboard/MCU or the external power supply.

Instructions for LED Strips:
- Most bigger LED boards and LED strips have sets of soldered wires, often with a handy 3pin/4pin SM JST connector. Each set of wires is connected to the strip every +-15 LED's. You need to connect all VCC/GND wires to the power supply to ensure the power is distributed correctly across the whole strip.

Instructions for WS2813x LED Strips:
- These strips have an extra signal wire (BACKUP DATA IN) which can be connected in case that the wire of the DATA IN through the LED board/strip is not working anymore. I leave this wire unconnected by default.



## Wiring Instructions
### MCU Adafruit HUZZAH32 (ESP32)
- GPIO OUTPUT PIN#14 = 1-Wire TX (you can use another PIN# as long as the PIN# is not reserved by the ESP32 system).

Goto the directory ../../development_boards/ for images with the GPIO PIN layout for some development boards.

### LED board/strip
[See images in the _doc directory]
- Connect input pin GND to the GND pin of the MCU.
- Connect input pin VCC to the VCC 3.3V pin of the MCU.
- Connect input pin DATA IN, via a +-470 Ohm resistor, to the GPIO output pin on the MCU.
- Only for the package WS2813x with the new Backup Data Input pin (BIN): if the DATA IN is not working anymore then connect the BIN pin, via a +-470 Ohm resistor, to the same GPIO output pin on the MCU as the input pin DATA IN.

Notes:
- If you want to combine multiple LED strips, and address them as one long LED strip, then connect them in series. The IN/OUT pads or wires are present on each LED strip. And use the MJD component's init() func to declare the total number of LED's you actually want to use in your app.
- If you want to send the same data signal to multiple LED strips then connect the same GPIO output pin to each LED strip's DATA IN pin. This is handy e.g. when having a cube of LED matrixes and you want to show the same pattern on each side of the cube.



## Data Protocol of WS2812 WS2812B WS2813
You do not have to understand this protocol in order to use this MJD component.

- The embedded LED driver chip communicates via a unique "NZR" (NonReturnToZero) one-wire interface.
- All LED chips send the received data synchronously to each LED segment when the DIN port receives a RESET signal.
- Color (R,G,B) values [0..255]
    @range 0x00..0xFF
    @doc 0x20 is bright enough for indoors!
- One LED: GRB Composition of 24bit Data:
    G7 G6 G5 G4 G3 G2 G1 G0 R7 R6 R5 R4 R3 R2 R1 R0 B7 B6 B5 B4 B3 B2 B1 B0



## LED RGB FAQ
- The manufacturer is Worldsemi http://www.world-semi.com/
- These LED chips are typically called "SMD 5050" because the dimensions of the SMD package are 5.0mm x 5.0mm.
- The WS2812 / WS2812B / WS2813x packages contain an embedded version of the WS2811 constant-current LED driver package, and 3 individually controlled LEDs; 1 red, 1 green, 1 blue.
- The WS2811 LED driver package includes:
    + An internal oscillator
    + A signal reshaping and amplification circuit
    + A data latch
    + A 3-channel, programmable constant current output drive
    + 2 digital ports (serial output/input)
- Each WSx package consumes +-50mA (18mA / color LED) when fully *ON (white 0xFF 0xFF 0xFF at full brightness).
- The voltage of the power supplied to the LED board, and the voltage of the output data signal coming from the MCU, must match according to the data sheets. But you will see later that is not always a requirement (luckily).



## Notes for large RGB LED arrays and power supply (more than 20 RGB LED packages)
## Introduction
Do not power large LED arrays using the 3.3V VCC pin of an ESP32 MCU because its max amperage is too low (+-300mA left for the LEDs, even less when using Wifi). Some ESP32 MCU boards also have a 5V PIN but the max amperage is also too low. It also depends on the length of the wires between the MCU and the LED board, and on the length of the wires between the LED's on the LED board.

So you always have to use an additional 5V power supply for a large LED array. A "DC Power Plug Jack Adapter Connector 5.5x2.1mm" is handy to connect the wires of the LED Strip to the power supply connector. Also connect a 1000uF capacitor between VCC and GND of the connector of the power supply.

Remember to also wire GND of the power supply to the pin GND of the MCU.

Mega LED arrays (>250 LEDS) probably require thicker wires than standard AWG22 Dupont wires for connecting the power supply.

First try using a 3.3V data signal with a 5V power supply. The products listed in the beginning of this article all work in this configuration. This is the simplest configuration. If this scenario does not work for your product then use the other scenario "5V data signal and a 5V power supply".

## Scenario 3.3V data signal and 5V power supply
Connect the VCC and GND wire of the power supply to the respective wire/pad of the LED board/strip.

Connect the GPIO OUTPUT PIN of the ESP32 development board directly to the DATA IN wire of the LED board/strip.


## Scenario 5V data signal and 5V power supply
Connect the VCC and GND wire of the power supply to the respective wire/pad of the LED board/strip.

You have to use a logic level shifter board to shift the data signal from 3.3V to 5V (a uni-directional logic level shifter is sufficient and faster than a bi-directional one). And you want a DIP package if you want to use it on a breadboard.

@important The popular bi-directional logic level shifters with a BSS138 MOSFET do not work because the switching characteristics are not good enough!



A few logic level shifters are known to work for the WS28xx LED strips:
- TI SN74HCT245 DIP-20 Octal Bus Transceivers With 3-State Outputs
- TI SN74AHCT125N DIP-20 Quadruple Bus Buffer Gates With 3-State Outputs (this one has the fastest switching characteristics but it is the most expensive one)
- TI SN74HCT08N DIP-14 Quadruple 2-Input Positive-AND Gates

Check the data sheet of the specific logic level shifter for the exact wiring instructions.



Instructions:
- Connect the 3.3V VCC PIN of the ESP32 development board to the VCC pin on the 3.3V side of the logic level shifter board.
- Connect the GND PIN of the ESP32 development board to the GND pin on the 3.3V side of the logic level shifter board.
- Connect the 5V VCC PIN of the power supply to the VCC pin on the 5V side of the logic level shifter board.
- Connect the GND PIN of the power supply to the GND pin on the 5V side of the logic level shifter board.
- Connect the GPIO OUTPUT PIN of the ESP32 development board to the 3.3V input pin of the logic level shifter board.
- Connect the 5V OUTPUT PIN of the logic level shifter board to the DATA IN wire of the LED board/strip.



## Issues
- You often have to solder wires to the input pads on the back before you can use these products. I prefer boards with pre-soldered wires & connectors.
- The timings of the WS2812 / WS2812B packages are different.
- Different batches of the same package might also have different timings. This has been reported often. So the spec has changed over time without making that apparent in the product number...



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

