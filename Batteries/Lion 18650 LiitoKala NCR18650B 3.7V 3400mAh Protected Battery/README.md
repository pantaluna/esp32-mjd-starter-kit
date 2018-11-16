Li-ion 18650 LiitoKala NCR18650B 3.7V 3400mAh Protected Battery
========================================================================

## Specs

- Product name: NCR18650B Rechargeable Li-ion Battery.
- Battery Chemistry: NCS18650B are "officially" Lithium Nickel Cobalt Aluminum Oxide (LiNiCoAlO2) chemistry but I do not know that is really so for Chinese clones.
- 3300mAh (versus 1800mAh for LiFePO4 ...).
- Dimensions: length 6.78cm x diameter 1.85cm
- Weight: 48g
- Built-in overcharge & undercharge protection circuit :) The battery shuts down when hitting these thresholds. Thresholds: max voltage is 4.3V, min voltage is 2.7V [I did tests myself to determine these threshold values].
- Nominal Voltage: 3.6V
- Minimum Voltage: 2.7V (shutdown by protection circuit)
- Maximum Voltage: 4.3V (shutdown by protection circuit) XOR 4.2V (the Liitokala Charger will stop automatically)
- Maximum charge current/time: 1.5C (means: you can charge the total 3300mAh in minimum 1.5 hours = max 2.2A charge current | OK I only charge at max 1A).
- Does not work with ambient temperatures below 0 degrees Celsius. Can not be charged at temperatures below 5 degrees Celsius.
- Advice to store them long term at max 50% charged.
- Danger: lithium batteries show unwanted chemical reactions when discharged ***below 2.7V*** and results in permanent damage and less capacity and shorter life!
- => The best battery type for ESP32 dev boards!
- => Typically 50% more capacity than LiFePO4 batteries with the same form factor.
- => High output current which is adequate for the ESP32 (with Wifi).
- => The discharge output voltage curve is not as flat as for LiFePO4 batteries but it doesn't matter. Each dev board has a LDO Voltage Regulator.



## Charger

The LiitoKala Lii-S1 18650 Charger can charge Li-On and LiFePO4 batteries.

My ESP32 dev boards contain a Lion/LiPo battery charger circuit (not compatible with LiFePO4 batteries).



## Charging Time

Using the LiitoKala Lii-S1 18650 Charger (2.7V -> 4.2V):  5h with a charge current of 0.5A



## Charge Procedure

Using the LiitoKala Lii-S1 18650 Charger:

1. Plugin battery.

2. Lion is the default battery type.

3. Starts charging until full.



## Perfect for ESP32 3.3V microcontrollers.

@danger Never disable the Voltage Regulator because the voltage level of the Lion battery is out of spec for the ESP32! It would blowup the ESP32.
@important Wire up a 1000uF capacitor between VCC and GND to improve handling of high current spikes (typical for Wifi connect phase).



+ These  batteries include overcharge and undercharge protection.

+ A good autonomy overall. 3300mAh.

+ The ESP32 stops working when voltage < 2.76V when you are using WIFI.
    The BrownOutDetector is setup by default to trigger at 2.43V. You can also disable the BOD if the battery has a protection circuit; and then you get a little more juice out of it.

    Making a wifi connection draws 500mA of current and puts a heavy load on the battery  and so the voltage will drop during the Wifi Connect phase. The BrownOutDetector (BOD) will detect the low voltage and reboot the device when the BOD threshold is reached.

- Deployment: use a 18650 battery holder and solder 2 wires and a male JST-PH2 (PH=2mm distance, 2=2pin) battery connector (from my stock) to it. So it can wired up to the ESP32 development board.
    See the pictures for more details.
    Doublecheck the polarity (a red wire from the battery holder is not ways +Positive!).

## Notes
- Tesla Model S battery packs are composed of Lithium Ion 18650B batteries. Prices are low.