LiFePO4 18650 Soshine 3.2V 1800mAh Protected Battery
====================================================

## Specs

- 1800mAh (versus 3000mAh for Li-on...).
- Dimensions: length 6.75cm x diameter 1.82cm
- Built-in overcharge & undercharge protection circuit :).
- Nominal Voltage: 3.20V
- Minimum Voltage: 2.50V (battery empty) => @important The battery protection circuit will shutdown the battery unit!
- Maximum Voltage: 3.65V (battery full).
- Does not work with ambient temperatures below 0 degrees Celsius. Can certainly not be charged at temperatures below 5 degrees Celsius.
- => Advice to store them long term at 50% charged.
- => High output current which is sufficient for the ESP32 (with Wifi).
- => An relatively flat discharge output voltage curve of +-3.20V



## Charger

- LiitoKala Lii-S1 18650 Charger can charge Li-On and LiFePO4 batteries.
- @danger Always use the Liitokala S1 LiFePO4 dedicated charger. Never use an ESP32 dev board for charging (most ESP32's contain a Li-On/LiPo battery charger circuit, not suited for LiFePO4 batteries!). A Lion LiPo battery charger circuit only stops charging at 4.2V; this is a serious issue for LiFePO4 batteries which should only be charged up to a max of 3.65V else they get damaged!



## Charging Time

Using the LiitoKala Lii-S1 18650 Charger (2.76V -> 3.65V): 3h



## Charge Procedure

Using the LiitoKala Lii-S1 18650 Charger:

1. Plugin battery.

2. Press button on the charger until the red LED of "LiFePO4 3.2V" lights up (that is the NOT the default battery type).

3. Starts charging until full.



## Perfect for ESP32 3.3V microcontrollers.

+ These Soshine LiFePO4 batteries include an overcharge and undercharge protection module.

+ The Nominal Voltage is kept a relatively long time across the power draw lifecycle (a unique characteristic of LiFePO4).

+ A good autonomy overall. 1800mAh results in 3 months autonomy which is sufficient for my projects [deep sleep 15minutes intervals].

+ The ESP32 stops working when battery voltage < 2.76V when you are using Wifi.

+ @important Wire up a 1000uF capacitor between VCC and GND to improve handling of high current spikes (typical for Wifi connect phase).

+ The BrownOutDetector is by default setup to trigger at 2.43V. You can also disable the BOD if the battery has a protection circuit; and the you get a little more juice out of it.

+ The battery is  not yet completely empty @ 2.76V (but almost). You can safely lower the BOD voltage level, or disable BOD, and get some more juice out of the battery.

+ The battery will then either shutdown at +-2.50V (its protection circuit) XOR the ESP32 does not get enough voltage/current anymore and it will crash (crash=not recommended!).

+ Making a wifi connection draws 500mA of current and puts a heavy load on the battery and so the voltage will drop during the wifi connect phase, the BrownOutDetector (BOD) will detect the low voltage and reboot the device when the BOD threshold is reached.



## MCU differences

The Lolin32Lite dev board consumes 15% LESS power than the Adafruit HUZZAH32 dev board.



## Deployment

- Use a 18650 battery holder and solder 2 wires and a male JST-PH2 (PH=2mm distance, 2=2pin) battery connector (from my stock) to it. So it can plugged in into my ESP32 development boards.
    See the pictures for more details.
- Doublecheck the polarity (a red wire from the battery holder is not ways +Positive!).

