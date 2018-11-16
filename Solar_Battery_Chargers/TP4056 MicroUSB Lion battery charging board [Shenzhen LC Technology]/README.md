# Micro USB TP4056 Lithium battery charging board, discharge over-current over-voltage protection

Aug2018.

## Overview

- Product Page [http://www.chinalctech.com/index.php?_m=mod_product&_a=view&p_id=1079](http://www.chinalctech.com/index.php?_m=mod_product&_a=view&p_id=1079)

- Red Green LED explanation
    ```
    Charge state                Red LED     Greed LED
    ============                =======     =========
    Charging                    *ON         *OFF
    
    Charge Terminated           *OFF        *ON
    
    Vin too low;                *OFF        *OFF
    Battery temp too low/high;
    No battery;
    
    BAT PIN Connect             Flicker     *ON
    10u Capacitance;
    ```

## FAQ
- ***CORRECTION for the data sheet*** Simultaneous charging and discharging **is** supported! And the output current to the load will NOT be cut.
- Input Supply Voltage (VCC): -0.3V -> 8V.
  @important The minimum input voltage is actually 4.25V before it really starts charging (and the RED led & GREEN led turn on).
- What is the voltage @ the OUT+ pin?
  => When charging using a MicroUSB phone charger: 4.16V (it is higher than max 3.5V max for the MCU board but a dev board contains an LDO voltage regulator).
  => When USB not connected (not charging): the battery's voltage.
- GOOD: can be used with INPUT solar panels or USB (smartphone charger) and OUTPUT and charge batteries at the same time.
- GOOD: also has OUTPUT pins for the external load (e.g. the MCU).
- BAD: cannot be used to charge LiFePO4 batteries.
- BAD: cannot be used for Lion batteries with a capacity lower than 1000mAh (because with default RPROG 1.2KOhm it charges max with IBAT 1A which is too much for smaller batteries).
- BAD: Many TP4056 chips are fake and are potentially not working. The fake ones do not have the original logo resembling a "F". Please check at least one charge cycle with your IC to verify it works correctly (and don't use a protected battery for that as it would protect the cell and hide the error).
- Optimized for charging 1 Lion battery.
- IC TP4056 is made "by Nanjing Top Power ASIC" http://www.tp-asic.com/te_product_a/2008-04-09/2236.chtml  
- The breakout board is often made by "Shenzhen LC technology co., LTD..." http://www.chinalctech.com/index.php?_m=mod_product&_a=view&p_id=1079

## PCB
- TP4056 ST291P battery charger IC (largest chip).
- 8205A 1745DC battery protection IC for 4-series or 5-series cell pack from ABLIC.com (second largest).
- DW01A one-cell Lion battery protection IC from Fortune Semiconductor Corp (third largest).



## Spiess videos:
PLAYLIST "Solar Power for Small Devices" https://www.youtube.com/playlist?list=PL3XBzmAj53Rl6hxunDxEm4V98qK_75g-m

- ID #142 Solar Power for the ESP8266, Arduino, etc. https://www.youtube.com/watch?v=WdP4nVQX-j0&index=4&list=PL3XBzmAj53Rl6hxunDxEm4V98qK_75g-m

- ID #154 Solar Charger for Microcontrollers  https://www.youtube.com/watch?v=dBx-g1dkdDQ

- ID #155 The 5 Best Solar ChargerBoards for Arduino and ESP8266 https://www.youtube.com/watch?v=ttyKZnVzic4

- ID #183 How to select voltage regulators for small projects? (ESP8266, ESP32, Arduino) https://www.youtube.com/watch?v=ffLU7PSuI5k&index=1&list=PL3XBzmAj53Rl6hxunDxEm4V98qK_75g-m
