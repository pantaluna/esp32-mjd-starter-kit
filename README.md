# ESP32 MJD Starter Kit Software
August 2018.

## Introduction

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Are you ready to discover how you can get started quickly?



## Why would you need this ESP32 MJD Starter Kit Software?
The ESP-IDF framework (and its documentation) is very powerful and extensive.

I found it difficult to get started quickly. I'm just a seasoned full stack developer (backend/frontend) without much experience developing IoT solutions using embedded systems.

More specifically, I could understand all the features of the ESP-IDF framework but I had a hard time gluing everything together, and quickly develop real projects for real solutions using specific peripherals such as sensors, LoRa boards, GPS boards and LED strips. For example, I wanted to start with projects controlling various sensors in a network and analyzing the data on a central server, and then move on to more complex projects.

Secondly, it was difficult to find good documentation (data sheets, diagrams, photo's of the wiring) of the various peripheral devices such as meteo sensors, GPS boards, RGB LED's, etc. And how to use these devices in combination with an ESP32-based development board.

So I developed over time these extra components, good documentation, and many working projects targeting a whole suite of peripherals that are typically used in IoT projects.

Now is a good time to give something back to the ESP32 community and release everything I learned so far as open source, so everyone can benefit from this work.



## Why choose the ESP-IDF framework?
You have 2 options to start developing for the ESP32 chip:

1. Use "ESP-IDF", the extensible official Espressif IoT Development Framework of Espressif. \
This is the official development framework for the ESP32 chip. It is targeted for C/C++ applications and it includes a port of the popular Amazon FreeRTOS O.S. This is the most powerful framework of the two and it contains a ton of excellent libraries so you can use all features of the ESP32 environment. It assumes you are at least an intermediate C Developer and it has a stiff learning curve. It is closed to the metal than the Arduino framework for ESP32.

2. Use the official "Arduino Core For ESP32" framework of Espressif for the Arduino IDE. \
This is a hardware abstraction layer for Arduino IDE so you can target the ESP32 chip. The big advantage is that you can empower your existing knowledge of the Arduino IDE. The downside is that it is not that feature-rich compared to ESP-IDF when it comes to specific ESP32 functionality. And the development pace is slower. And not all features of ESP-IDF have been ported to Arduino.



It is important to know that both frameworks are stable and usable but they are still under significant development by Espressif, and major new releases are coming out on a regularly basis; I expect this to continue at least until 2018Q4.

After experimenting with both frameworks I decided to go with the ESP-IDF framework, more specifically V3.1 and higher. I always try to release libraries that are compatible with the last stable release.



## What is in the ESP32 MJD Starter Kit?

### Development Boards Documentation

This part contains basic information about some ESP32 development boards that I have used initially:
- Adafruit HUZZAH32.
- Wemos Lolin32 Lite.



### USB To TTL Serial Adapters Documentation

An ESP32 development board can only be programmed (easily) by such an adapter.

This part documents a few products and how to configure them.



### Batteries Guidelines

Rechargeable batteries are often used in IoT projects.

This part a popular battery charger, a self-made battery discharger, and details about popular batteries such as the Lion batteries and LiFePO4 batteries. It also contains some specs about professional LiSOCI2 Lithium-Thionyl-Chloride non-rechargeable batteries for use in harsh conditions.



### Solar Battery Chargers

Documentation about the TP4056 module.



## -Working Projects-

The Starter Kit includes various working projects that you can run instantly - opposed to snippets that you have to glue together yourself, which is not easy for a beginner.



These projects:
- Give insights in how to actually use the official ESP-IDF framework efficiently.

- Include a ton of best coding practices and configuration practices.

- Demonstrate how to use the new ESP-IDF components of this Starter Kit, such as RGB LED strips and meteo sensors.



Let's highlight a few projects that demonstrate how to use the core ESP-IDF framework.
- `esp32_button_basics` How to interface with buttons (switches).
- `esp32_deep_sleep_wakeup_basics` Demonstrates how to use a switch or a magnetic door/window sensor to wake up an ESP32 from deep sleep.
- ```esp32_http_client``` Demonstrates the basics of using the standard ESP-IDF component "esp32_http_client".
- `esp32_gpio_basics` How to interact with GPIO pins of the development board.
- `esp32_gpio_scanner` How to scan all GPIO pins and discover their I/O function.
- `esp32_i2c_scanner` How to scan all slave devices on the I2C pins, and identify their I2C slave address. This is handy when working with new I2C slave devices.
- `esp32_ledc_pwm_basics` How to use the standard ESP-IDF LEDC driver (a LED Controller driver using PWM).
- `esp32_nvs_basics` How to use the standard ESP-IDF NVS (Non-Volatile Storage) driver with a custom NVS partition.
- `esp32_rmt_basics` How to use the standard ESP-IDF RMT driver.
- `esp32_spiffs_basics` How to use the standard ESP-IDF SPIFFS file system driver.
- `esp32_sw180_tilt_sensor` How to interface with this tilt sensor (no extra components needed).
- `esp32_timer_basics` How to use the standard ESP-IDF Timer driver.
- `esp32_uart_basics` How to use the standard ESP-IDF UART driver.
- ```esp32_udp_client``` Demonstrates the basics of implementing an UDP Client using the ESP-IDF framework.



The special project `esp32_mjd_components`:

- **All the ESP-IDF MJD components are centralized in this project.**
- The project is also runnable and it demonstrates best practices when using ESP-IDF and example code for all non-peripheral components such as linked lists, ESP chip interfaces, Wifi, networking, MQTT, .... 




Let's highlight a few projects that demonstrate how to use the extra components of the ESP32 Starter Kit.
- ```esp32_ads1115_adc_using_lib``` Demonstrates the basics of using the MJD ESP-IDF component "mjd_ads1115" for the ESP32 and the popular breakout boards of the TI ADS1115 Ultra-Small, Low-Power, I2C-Compatible, 860-SPS, 16-Bit ADCs With Internal Reference, Oscillator, and Programmable Comparator using the I2C Bus.
- ```esp32_am2320_temperature_sensor_using_lib``` How to read data from the Aosong AM2320 meteo sensor.
- `esp32_bh1750fvi_lightsensor_using_lib` How to read data from the BH1750 light intensity sensor.
- `esp32_bme280_sensor_using_lib` How to read data from the Bosch BME280 meteo sensor.
- `esp32_bmp280_sensor_using_lib` How to read data from the Bosch BMP280 meteo sensor.
- `esp32_dht11_temperature_sensor_using_lib` How to read data from the Aosong DHT11 temperature sensor.
- `esp32_dht22_temperature_sensor_using_lib` How to read data from the Aosong DHT22/AM2302 temperature sensor.
- `esp32_door_sensor_reed_switch` Demonstrates how to use a magnetic door/window sensor which is based on a reed switch.
- `esp32_ds3231_clock_using_lib` How to get/set data from the DS3231 ZS042 RTC realtime clock board.
- `esp32_hcsr501_pir_sensor_using_lib` How to read data from the HC-SR501 PIR human infrared sensor.
- `esp32_huzzah32_battery_voltage_using_lib` How to use specific features of the Adafruit HUZZAH32 development board. example: read battery voltage level. 
- `esp32_ky032_obstacle_sensor_using_lib` How to read data from the KY-032 infrared obstacle avoidance sensor.
- `esp32_ledrgb_using_lib` How to control RGB LED strips (such as the Adafruit Neopixels and BTF-LIGHTNING products).
- `esp32_linked_list_basics` How to use the Linked List component.
- `esp32_lorabee_using_lib` How to interact with the SODAQ LoraBee breakout board (Microchip RN2843 LoRa transceiver). This project demonstrates the basic commands to configure the device and to read/write the NVM.
- `esp32_lorabee_rx_using_lib` How to interact with the SODAQ LoraBee breakout board (Microchip RN2843 LoRa transceiver). This project demonstrates the Lora RX Receive functionality. Note: it uses Lora P2P and not LoraWAN.
- `esp32_lorabee_tx_using_lib` How to interact with the SODAQ LoraBee breakout board (Microchip RN2843 LoRa transceiver). This project demonstrates the Lora TX Transmit functionality. Note: it uses Lora P2P and not LoraWAN. 
- `esp32_lorabee_using_pc_usbuart` This project demonstrates how to issue basic commands to the LoraBee module using a Windows PC and a USB-UART board (such as an FTDI). This is an easy way to get familiar with the features of the LoraBee / Microchip RN2843A board.
- `esp32_mlx90393_using_lib` How to get magnetic field data using the Melexis MLX90393 magnetic field sensor.
- `esp32_neom8n_gps_using_lib` How to get GPS data from the GPS Ublox NEO-M8N module.
- ```esp32_tmp36_sensor_ads1115_adc_using_lib``` This project demonstrates the components mjd_ads1115 and mjd_tmp36. The mjd_ads1115 component for the TI ADS1115 Analog-To-Digital-Convertor is used to read the voltage output of the analog temperature sensor. The mjd_tmp36 component for the TMP36 sensor is used to convert the raw voltage reading of the ADC to the ambient temperature in Degrees Celsius transparently.
- `esp32_wifi_device_scanner` How to scan all Wifi channels and discover the devices.
- `esp32_wifi_ssid_cloner` How to clone existing Access Points.
- `esp32_wifi_ssid_scanner` How to scan all Wifi channels and discover the Access Points.
- `esp32_wifi_ssid_spammer` How to create additional Access Points in the area.
- `esp32_wifi_stress_test` This app run a stress test for the ESP32 dev board in the role as Wifi Station. The purpose is to verify the stability of the ESP32 Wifi software driver of a specific version of the ESP-IDF framework; to verify its correct operation with Wifi Access Point products of various vendors.



### Extra ESP-IDF Components
I noticed that many coding patterns came back again and again in the first projects that I developed for the ESP32.

So after a while I started putting those coding patterns in separate libraries. The ESP-IDF is an extensible framework so these libraries are implemented as new ESP-IDF components which can be injected easily in any ESP-IDF based project.

**All the ESP-IDF MJD components are centralized in the project ```esp32_mjd_components```.** 



The components can roughly be divided in 3 groups:

1. Related to programmming in the C language (which has its own quirks as all other programming languages). Example: linked lists.

2. Related to the ESP32 environment and the specifics of embedded systems. Examples: an easy Wifi component. They make those ESP-IDF features easier to use.

3. Related to networking. Some examples: interfacing with an MQTT server and some DNS functions. The component abstracts the complexity, and makes it easier to use.

4. Related to the peripherals that you wire up to the ESP32 chip or ESP32 module. Some examples: Lora board, RGB LED, temperature sensors, GPS boards, RTC clocks, PIR sensors and obstacle sensors. The component abstracts the complexity of the peripheral.



This is the list of new components:
- `mjd` The base component which contains general purpose functions.
- ```mjd_ads1115``` Component for the TI ADS1115 Analog-To-Digital-Convertor 16-bit.
- `mjd_am2320` Component for the Aosong AM2320 meteo sensor.
- `mjd_bh1750fvi` Component for the BH1750 light intensity sensor.
- `mjd_bme280` Component for the Bosch BME280 meteo sensor.
- `mjd_bmp280` Component for the Bosch BMP280 meteo sensor.
- `mjd_dht11` Component for the Aosong DHT11 temperature sensor.
- `mjd_dht22` Component for the Aosong DHT11/AM2302 temperature sensor.
- ```mjd_ds3231``` Component for the DS3231 ZS042 RTC real-time clock board.
- `mjd_hcsr501` Component for the HC-SR501 PIR human infrared sensor.
- `mjd_huzzah32` Component for the Adafruit HUZZAH32 development board (read battery voltage level).
- `mjd_ky032` Component for the KY-032 infrared obstacle avoidance sensor.
- `mjd_ledrgb` Component for controlling various RGB LED strips (WorldSemi WS28xx chips such as the Adafruit Neopixels product line).
- `mjd_list` Component that implements the Linked Lists as used in the Linux Kernel.
- `mjd_log` Component to facilitate logging in the app.
- `mjd_lorabee` Component to interact with the SODAQ LoraBee Microchip RN2483A board (contains a Microchip RN2843 868Mhz LoRa chip).
- `mjd_mlx90393` Component for the Melexis MLX90393 magnetic field sensor (X Y Z axis and Temperature metrics).
- `mjd_mqtt` Component for interacting with an MQTT server (as an MQTT client).
- ```mjd_nanopb``` Component to work with Google Protocol Buffers. It includes the common C files of the Nanopb library v0.3.9.2. It also declares Nanopb specific project-wide compilation directives (-D) in Makefile.projbuild
- `mjd_net` Component to facilitate various networking features (getting IP address, DNS resolve hostnames, etc.). 
- `mjd_neom8n` Component for the GPS u-blox NEO-M8N module.
- ```mjd_tmp36``` Component for the TMP36 Analog Temperature Sensor from Analog Devices. To be used together with an ADC.
- `mjd_wifi` Component to facilitate, as a Wifi Station, a connection to a Wifi Access Point.



Let's categorize these components in more detail:

#### C Language
- Linked lists using the Linux Kernel implementation.
- Manage strings.
- Manage date & time.
- Manage BCD (binary-coded-decimal).

#### ESP32 General
- Logging.
- Deep sleep.

#### ESP32 Networking
- LoRa.
- Synchronizing the datetime (SNTP).
- Resolve hostnames using DNS.
- Get current IP address.
- Check if Internet is available.

#### ESP32 Wifi
- Manage Wifi connections.

#### ESP32 MQTT
- Manage MQTT publish/subscribe.

#### Adafruit HUZZAH32 (ESP32 development board)
- Read the battery voltage level.
- Obtain the Voltage Reference for your device (+-1100mV).

#### ESP32 Peripherals - Devices
- ADC TI ADS1115 16-bit.
- GPS u-blox NEO-M8N module.
- Real Time Clock DS1302 ZS-042.

#### ESP32 Peripherals - RGB LED's
This component supports several RGB LED packages. It comes with the essential documentation such as data sheets, schematics, and instructions on how to wire them to your development board and eventually an extra power supply.

- RGB LED packages such as the WS2812, WS2812B, WS2813xs chips from the manufacturer Worldsemi http://www.world-semi.com/solution/list-4-1.html 
- Adafruit's Neopixels that use the aforementioned packages.

#### ESP32 Peripherals - Sensors
These components come with the essential documentation such as data sheets, schematics, and instructions on how to wire them to your development board.

- AM2320 temperature sensor by Aosong.
- DHT11 temperature sensor by Aosong.
- DHT22/AM2302 temperature sensor by Aosong.
- BH1750FVI light sensor.
- Bosch BME280 meteo sensor.
- Bosch BMP280 meteo sensor.
- HC-SR501 PIR motion sensor.
- KY-032 Infrared obstacle avoidance sensor.
- Analog Devices TMP36 analog temperature sensor.



## What are the HW SW requirements of the ESP32 MJD Starter Kit?

### Hardware

- A decent ESP development board. I suggest to buy a popular development board with good technical documentation and a significant user base. Examples: [Adafruit HUZZAH32](https://www.adafruit.com/product/3405),  [Espressif ESP32-DevKitC](http://espressif.com/en/products/hardware/esp32-devkitc/overview), [Pycom WiPy](https://pycom.io/hardware/), [Wemos D32](https://wiki.wemos.cc/products:d32:d32).

- The peripherals that are used in the project.
  @tip The README of each component contains a section "Shop Products".
  @example A Bosch BME280 meteo sensor.


### Software - ESP-IDF v3.1.1

- A working installation of the Espressif ESP-IDF ***V3.1.1*** development framework (detailed instructions @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).

```
mkdir ~/esp
cd    ~/esp
git clone -b v3.1.1 --recursive https://github.com/espressif/esp-idf.git esp-idf-v3.1
```

- A C language editor or the Eclipse IDE CDT (instructions also @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).



## How to use the ESP32 MJD Starter Kit?

### First get familiar with the ESP32 ecosystem

- Hardware: reading at least the technical reference ( https://www.espressif.com/en/products/hardware )
- Software: the ESP-IDF software framework ( https://docs.espressif.com/projects/esp-idf/en/v3.1.1/get-started/index.html ). Installation and Getting Started.
- Developer's Forum: https://www.esp32.com/



### Then start using this Starter Kit

Procedure:
1. A working installation of the Espressif **ESP-IDF V3.1.1** development framework (instructions @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).
   @important Make sure to clone **the version v3.1.1** of the framework when following the Espressif Getting Started instructions e.g. `git clone -b v3.1.1 --recursive https://github.com/espressif/esp-idf.git esp-idf`

2. A C language editor such as Notepad++ or the Eclipse IDE CDT (instructions @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).

3. Clone this Github repository. `git clone https://github.com/pantaluna/esp32-mjd-starter-kit.git`

4. Read the documentation about development boards, USB to TTL boards, batteries, etc.

5. `cd` into the directory of the project you want to explore under `./projects`.

6. Read the instructions in the README for the hardware, wiring, and software setup.

7. Read the instructions in the README of all the extra components that are used in this project in the ./components directory. Remember that the wiring instructions are always documented in the component's directory ./_doc/ (not in the project directory). The documentation typically documents the wiring for the Adafruit HUZZAH32 (a good ESP32 development board) and that info can easily be extrapolated to other ESP32 dev boards.

8. Run `make menuconfig` to modify the settings of the project that you want to run (e.g. GPIO PIN#, WiFi credentials, ...).

9. Run `make flash monitor` to build and upload the example to your dev board and monitor the execution via the serial terminal.



## FAQ
- The ESP32 Starter Kit gets you started quickly. If you need extra features of an existing component, or you wish to propose a new component, then please submit an issue.

- All the MJD components are centralized in the project ```mjd_components```.

- The Kit is not designed to implement all conceivable features of any ESP32 project. If a new feature is very specific for your project then the best approach is to make your own bundle of ESP-IDF components with the functionality that you want. You can use these components as the foundation; please do not forget to mention that you obtained the components from this Starter Kit.

- What does "MJD" stands for? It is meaningless codeword and it is used in the C language to make identifiers unique. This approach ensures that you can use these new ESP-IDF components in any other C project.

- Why are all projects and components stored in one Github repository (opposed to have a Github repo for each project and each component)? I think this makes the Starter Kit easier to use for beginners. In the future the Kit might be setup using Git submodules.



## Known Issues

Check Github.



## What Is Next for me
- Release extra components for Dust particle sensors, TFT Displays and RGB LED matrixes.

- Release extra projects to demonstrate OTA Updates (upgrade the firmware remotely).

- To release an IOT Platform to the public so you can manage the devices in the field and analyze the incoming data.

- To make a new website for the technical documentation of this Kit.



## What Is Next for you
- Please share your experiences using this ESP32 MJD Starter Kit with the ESP32 community.
- I hope you get inspired by this Starter Kit to develop useful IoT projects and endorsing the Espressif products.
