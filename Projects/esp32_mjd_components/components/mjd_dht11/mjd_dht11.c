/*
 * DHT11 Sensor: 1-Wire custom protocol of Aosong
 *
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_dht11.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_dht11";

/*
 * MAIN
 */

static portMUX_TYPE sensor_port_mux = portMUX_INITIALIZER_UNLOCKED;  // @source portmacro.h

/*********************************************************************************
 * PUBLIC.
 * DHT11: RMT receiver initialization
 *
 * @important Custom for the DHT11 sensor!
 * @doc RMT source clock = APB Clock 80Mhz. It ticks 80,000,000 times a second or 80,000 times a MILLIsecond or 80 times a MICROsecond or 0.08 times a NANOsecond. This is VERY FAST!
 * @doc RMT if we divide the base clock by 80 then the granularity unit becomes 1 microsecond (=enough granularity for our sensors, and very easy to reason with that).
 *********************************************************************************/
esp_err_t mjd_dht11_init(const mjd_dht11_config_t* config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    const int RMT_CLK_DIV = esp_clk_apb_freq() / 1000000; // Final result = RMT Logical Clock @ 1Mhz | RMT counter clock divider CLK_DIV @formula 80 = 80,000,000 / 1000000
    const int RMT_TIMEOUT_US = 1000; // =1000 MICROsec =1 MILLIsec | RMT receiver timeout value (1 tick = 1 MICROsec) | @dependency My logical clock of 1Mhz!
    //const int RMT_FILTER_TICKS_THRESHOLD_US = 100; // =100 MICROsec =0.1MILLIsec @source infrared_nec_main.c uses 100... why? | @dependency My logical clock of 1Mhz!

    rmt_config_t rmt_rx =
        { 0 };
    rmt_rx.gpio_num = config->gpio_pin;
    rmt_rx.channel = config->rmt_channel;
    rmt_rx.rmt_mode = RMT_MODE_RX;
    rmt_rx.clk_div = RMT_CLK_DIV; // A clock divider, the range of pulse length differentiated by the RMT receiver. value = [1 .. 255]. The RMT source clock is mostly the APB CLK, 80Mhz by default.
    rmt_rx.mem_block_num = 1;
    rmt_rx.rx_config.filter_en = false;  // Do NOT filter on the input of the RMT receiver
    rmt_rx.rx_config.filter_ticks_thresh = 0;
    rmt_rx.rx_config.idle_threshold = RMT_TIMEOUT_US; // In RX Mode: the receive process is stopped when no edge is detected on the input signal for longer than idle_threshold channel clock cycles. (1 tick = 1 microsec)
    if (rmt_config(&rmt_rx) != ESP_OK) {
        ESP_LOGE(TAG, "rmt_config() FAIL");
        return MJD_ERR_ESP_RMT; // EXIT
    }
    if (rmt_driver_install(rmt_rx.channel, 1000, 0) != ESP_OK) {
        ESP_LOGE(TAG, "rmt_driver_install() FAIL");
        return MJD_ERR_ESP_RMT; // EXIT
    }

    return ESP_OK;
}

/*********************************************************************************
 * Processing the pulse data into temp and humidity
 *
 * @rules
 *  duration1 <  50 microsec : bitvalue=0
 *  duration1 >= 50 microsec : bitvalue=1
 * @usedby dht11_rmt_rx()
 *********************************************************************************/
static esp_err_t parse_items(rmt_item32_t* ptr_item, int num_items, mjd_dht11_data_t *data) {
    // Clock divisor (base clock is 80MHz)
    const int CLK_DIV = 100;
    // Number of clock ticks that represent 10us.  10 us = 1/100th msec.
    const int TICK_10_US = 80000000 / CLK_DIV / 100000;

    int i = 0;
    unsigned humidity = 0, temperature = 0, checksum = 0;

    // DEBUG Dump
    ESP_LOGD(TAG, "  num_items = %i", num_items); // logdebug
    rmt_item32_t *p = ptr_item; // Create a temporary pointer (=pointing to the beginning of the item array)
    for (i = 0; i < num_items; i++) {
        ESP_LOGD(TAG, "  %2i :: [level 0]: %3d - %3d us, [level 1]: %3d - %3d us | value uint32: %u", i, p->level0, p->duration0 * 10 / TICK_10_US,
                p->level1, p->duration1 * 10 / TICK_10_US, p->val); // logdebug
        p++; // Advance p
    }

    // Check we have enough pulses
    if (num_items < /*42*/41) {  // @RHMOD 42=>41 for my working modded MongooseOS version
        ESP_LOGE(TAG, "FATAL num_items < 42, num_items is %i", num_items);
        return MJD_ERR_INVALID_DATA; // EXIT
    }

    // Skip the start of transmission pulse
    /////item++; // @RHMOD Do comment for my working modded MongooseOS version

    // Extract the humidity data
    for (i = 0; i < 16; i++, ptr_item++)
        humidity = (humidity << 1) + (ptr_item->duration1 < 50 ? 0 : 1);

    // Extract the temperature data
    for (i = 0; i < 16; i++, ptr_item++)
        temperature = (temperature << 1) + (ptr_item->duration1 < 50 ? 0 : 1);

    // Extract the checksum
    for (i = 0; i < 8; i++, ptr_item++)
        checksum = (checksum << 1) + (ptr_item->duration1 < 50 ? 0 : 1);

    // Check the checksum
    if ((((temperature >> 8) + temperature + (humidity >> 8) + humidity) & 0xFF) != checksum) {
        ESP_LOGE(TAG, "Checksum failure %4X %4X %2X", temperature, humidity, checksum);
        return MJD_ERR_CHECKSUM; // EXIT
    }

    // Process humidity (uint16)
    ESP_LOGD(TAG, "  humidity:        %u", humidity);
    ESP_LOGD(TAG, "  humidity >> 8:   %u", humidity >> 8);
    ESP_LOGD(TAG, "  humidity & 0xFF: %u", humidity & 0xFF);
    data->humidity_percent = (humidity >> 8) + (0.1 * (humidity & 0xFF));

    // Process temperature (uint16)
    ESP_LOGD(TAG, "  temperature:        %u", temperature);
    ESP_LOGD(TAG, "  temperature >> 8:   %u", temperature >> 8);
    ESP_LOGD(TAG, "  temperature & 0xFF: %u", temperature & 0xFF);
    data->temperature_celsius = (temperature >> 8) + (0.1 * (temperature & 0xFF));

    return ESP_OK;
}

/*********************************************************************************
 * Use the RMT receiver to get the DHT11 data
 *
 * @important IRAM_ATTR = Forces code into IRAM instead of flash.
 * @important No Logging here (else timing goes wrong!)
 * @doc When the communication between MCU and DHT11 begins,
 *      then the MCU must set Data Single-bus voltage level from high to low and this process must take AT LEAST 18ms to ensure DHT’s detection of MCU's signal,
 *      then the MCU must pull up voltage and WAIT 20-40us for DHT’s response.
 *********************************************************************************/
IRAM_ATTR static bool dht11_wait(int pin, int lvl, uint32_t usecs) { // @source MongooseOS
    uint32_t t = 0;

    while (gpio_get_level(pin) != lvl) {
        if (t == usecs) {
            t = 0;
            break;
        }
        ets_delay_us(1);
        t++;
    }

    return t != 0;
}

IRAM_ATTR static esp_err_t dht11_rmt_rx(const mjd_dht11_config_t* config, mjd_dht11_data_t* data) {
    esp_err_t retval = ESP_OK;

    RingbufHandle_t rb = NULL;
    size_t rx_size = 0;
    rmt_item32_t* item;

    //get RMT RX ring buffer
    ESP_ERROR_CHECK(rmt_get_ringbuf_handle(config->rmt_channel, &rb));
    if (!rb) {
        return MJD_ERR_ESP_RMT; // EXIT
    }

    // # Bring the RMT code into the cache: rmt_rx_start & rmt_rx_stop (BEFORE pulling up the input signal to high @ MCU Part#2!)
    ESP_ERROR_CHECK(rmt_rx_start(config->rmt_channel, true));
    ESP_ERROR_CHECK(rmt_rx_stop(config->rmt_channel));

    // # Configure GPIO initial state
    portENTER_CRITICAL(&sensor_port_mux);
    gpio_set_direction(config->gpio_pin, GPIO_MODE_INPUT);
    gpio_pullup_en(config->gpio_pin);
    ets_delay_us(10 * 1000); // @important WAIT 10MILLIsec after configuring the GPIO for OUTPUT, ELSE failure later on! Do NOT USE vTaskDelay()!
    portEXIT_CRITICAL(&sensor_port_mux);

    // # MCU Start signals.
    //      @important The data sheet is wrong!
    // # MCU Start signal part#1. MCU will set Data Single-bus voltage level from high to low. Wait at least 18ms (=DHT11 specific).
    // # MCU Start signal part#2. Change to input state, and pull up the data bus (INPUT+PULLUP=>the data line also will be high!). Wait at least 20-40us for DHT11's to respond.
    portENTER_CRITICAL(&sensor_port_mux);
    gpio_set_direction(config->gpio_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(config->gpio_pin, 0);
    ets_delay_us(18 * 1000); // @important WAIT at least 18microsec. OK=???. Do NOT USE vTaskDelay()!
    gpio_set_level(config->gpio_pin, 1);
    gpio_set_direction(config->gpio_pin, GPIO_MODE_INPUT);
    gpio_pullup_en(config->gpio_pin);
    ets_delay_us(40); // @important WAIT at least 20-40MICROsec. OK=???. Do NOT USE vTaskDelay()!
    portEXIT_CRITICAL(&sensor_port_mux);

    //////////////////////////////////////////////////
    // Once DHT11 detects the start signal, the sensor sets the bus low for 80us as a response signal, then sets the bus high for 80us. Device is now ready to send data.
    // @important Do not use the 2nd part of the MongooseOS code, else parse_items() FAILS!

    portENTER_CRITICAL(&sensor_port_mux);
    if (!dht11_wait(config->gpio_pin, 1, 90)) {
        portEXIT_CRITICAL(&sensor_port_mux);
        return MJD_ERR_ESP_GPIO; // EXIT
    }
    /*if (!dht11_wait(gpio_pin, 0, 90)) {
     portEXIT_CRITICAL(&sensor_port_mux);
     return MJD_ERR_ESP_GPIO;
     }*/
    portEXIT_CRITICAL(&sensor_port_mux);

    ////////////////////////////////////////////////
    // Start the RMT receiver for the data
    if (rmt_rx_start(config->rmt_channel, true) != ESP_OK) {
        return MJD_ERR_ESP_RMT; // EXIT
    }

    /////////////////////////////////////////////////
    // Pull the data from the ring buffer
    //   xRingbufferReceive() param#3 ticks_to_wait MAXIMUM nbr of Ticks to wait for items in the ringbuffer ELSE Timeout (NULL).
    item = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, RTOS_DELAY_100MILLISEC); // ORIGINALVAL=100 MYVAL=RTOS_DELAY_100MILLISEC
    if (item == NULL) {
        rmt_rx_stop(config->rmt_channel); // cleanup
        return MJD_ERR_ESP_RTOS; // EXIT
    }
    int n;
    n = rx_size / sizeof(rmt_item32_t); // 4 => sizeof(rmt_item32_t)
    ESP_LOGD(TAG, "  sizeof(rmt_item32_t) = %i", sizeof(rmt_item32_t)); // logdebug
    ESP_LOGD(TAG, "  n                    = %i", n); // logdebug

    // Parse data value from ringbuffer.
    retval = parse_items(item, n, data);
    // After parsing the data, return spaces to ringbuffer.
    vRingbufferReturnItem(rb, (void*) item);

    /////////////////////////////////////////////////
    // Stop the RMT Receiver
    if (rmt_rx_stop(config->rmt_channel) != ESP_OK) {
        return MJD_ERR_ESP_RMT;
    }

    return retval; // @ the retval from parse_items
}

/*********************************************************************************
 * PUBLIC.
 * Read the DHT11 data
 *
 * @hardwarebug Retry 2 times, each after a 1 second delay.
 *********************************************************************************/
esp_err_t mjd_dht11_read(const mjd_dht11_config_t* config, mjd_dht11_data_t* data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t retval = ESP_OK;

    /*
     * Reset receive values
     */
    data->humidity_percent = 0.0;
    data->temperature_celsius = 0.0;

    /*
     * RMT
     */
    retval = dht11_rmt_rx(config, data);
    if (retval == ESP_OK) {
        return retval; // EXIT (OK)
    }
    ESP_LOGW(TAG, "Sensor 1st failure - retrying after 2 second(s)"); // @important Minimum 2 seconds!
    vTaskDelay(RTOS_DELAY_2SEC);

    retval = dht11_rmt_rx(config, data);
    if (retval == ESP_OK) {
        return retval; // EXIT (OK)
    }

    ESP_LOGW(TAG, "Sensor 2nd failure - retrying after 2 second(s)"); // @important Minimum 2 seconds!
    vTaskDelay(RTOS_DELAY_2SEC);

    retval = dht11_rmt_rx(config, data);
    if (retval == ESP_OK) {
        return retval; // EXIT (OK)
    }

    ESP_LOGE(TAG, "Sensor total failure (error) - RETURN FATAL");
    return ESP_FAIL; // EXIT (ERROR)
}
