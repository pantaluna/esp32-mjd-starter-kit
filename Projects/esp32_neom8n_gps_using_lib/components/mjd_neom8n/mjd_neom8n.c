/*
 * Goto the README.md for instructions
 *
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_neom8n.h"

// Extra includes
#include <math.h>

/*
 * Logging
 */
static const char TAG[] = "mjd_neom8n";

/*
 * UART settings
 *  @rule ring buffer size > 128
 *  @rule baud speed MAX 9600 for the Ublox NEO-M8N GPS board. Other (USB) UART boards might support higher speeds.
 */

#define MY_NEOM8N_USE_STUB_UART_READBYTES (false)
/////#define MY_NEOM8N_USE_STUB_UART_READBYTES (true)

#define MY_UART_BAUD_SPEED (9600)
#define MY_UART_RX_RINGBUFFER_SIZE (2048)
#define MY_UART_READBYTES_BUF_SIZE (512)
#define MY_UART_READBYTES_BUF_SIZE_PLUS_ONE (513)
#define MY_UART_READBYTES_TIMEOUT (RTOS_DELAY_MAX)

#define MY_GPS_LATLONG_ACCURACY_THRESHOLD (0.00001)

static mjd_neom8n_data_t _neom8n_data;

static SemaphoreHandle_t _neom8n_service_semaphore = NULL;

static volatile TaskHandle_t _neom8n_gps_monitor_task_handle = NULL;

/**************************************
 * STUBS
 */
static int _stub_uart_read_bytes(uart_port_t uart_num, uint8_t* buf, uint32_t length, TickType_t ticks_to_wait) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    static int cycle = 0;

    cycle++;

    if (cycle == 1) {
        strcpy((char *) buf, "$GNGGA,141645.00,6626.63658,N,03347.50630,E,1,10,0.94,8.9,M,46.1,M,,*46\r\n");
    } else if (cycle == 2) {
        strcpy((char *) buf, "A\r\n");
    } else if (cycle == 3) {
        strcpy((char *) buf, "BL1\r\nBL2\r\nBL3\r\nBL4");
    } else if (cycle == 4) {
        strcpy((char *) buf, "CL1\r\nCL2");
    } else if (cycle == 5) {
        strcpy((char *) buf, "\r\n");
    } else if (cycle == 6) {
        strcpy((char *) buf, "");
    } else if (cycle == 7) {
        strcpy((char *) buf, "RETURN ERROR");
        return ESP_FAIL;
        /*} else if (cycle == 8) { // Trigger stack overflow (NMEA messages <= 80 chars, but UBX msgs can be larger)
         strcpy((char *) buf,
         "$GNGSA,A,3,15,24,12,,,,,,,,,,,,,,,,,3.31,1.6$.GNTXT,01,01,02,u-blox AG - www.u-blox.com*4E*******||"
         "$GNGSA,A,3,15,24,12,,,,,,,,,,,,,,,,,3.31,1.6$.GNTXT,01,01,02,u-blox AG - www.u-blox.com*4E*******||\r\n"
         "$GNGSA,A,3,15,24,12,,,,,,,,,,,,,,,,,3.31,1.6$.GNTXT,01,01,02,u-blox AG - www.u-blox.com*4E*******||"
         );*/
    } else {
        strcpy((char *) buf, "1234567890\r\n");
    }

    // Simulate in this stub the pace rate of the GPS device (default = 1000ms Measurement Period)
    vTaskDelay(RTOS_DELAY_1SEC);

    return (int) strlen((char *) buf);
}

/**************************************
 * MONITOR TASK
 *
 */
static char* _get_next_line(uart_port_t param_uart_port) {
    static char line[MINMEA_MAX_LENGTH] = "";
    char *ptr_line = line;

    static uint8_t data_rx[MY_UART_READBYTES_BUF_SIZE_PLUS_ONE]; // @important +1 for the appended \0 character
    static uint8_t *ptr_data_rx;
    static int counter_data_rx = 0;    // @important 0 signals the data_rx buffer is empty

    while (1) {
        if (counter_data_rx <= 0) {   // @important POST Decrement
            // Read data from external UART1
            if (MY_NEOM8N_USE_STUB_UART_READBYTES == false) {
                counter_data_rx = uart_read_bytes(param_uart_port, data_rx, MY_UART_READBYTES_BUF_SIZE,
                MY_UART_READBYTES_TIMEOUT);
            } else {
                counter_data_rx = _stub_uart_read_bytes(param_uart_port, data_rx, MY_UART_READBYTES_BUF_SIZE,
                MY_UART_READBYTES_TIMEOUT);
            }

            if (counter_data_rx == ESP_FAIL) {
                ESP_LOGE(TAG, "    _get_next_line(): uart_read_bytes() err %i (%s)", counter_data_rx, esp_err_to_name(counter_data_rx));
                // ABORT
                return NULL;
            }
            if (counter_data_rx == 0) {
                ESP_LOGW(TAG, "    _get_next_line(): uart_read_bytes() 0 bytes returned");
                // ABORT
                return NULL;
            }

            // DEVTEMP
            ESP_LOGV(TAG, "    _get_next_line(): ESP_LOG_BUFFER_HEXDUMP(): data_rx");
            ESP_LOG_BUFFER_HEXDUMP(TAG, data_rx, counter_data_rx, ESP_LOG_VERBOSE);

            ptr_data_rx = data_rx; // reset ptr
        }

        // Detect new line.
        // @doc 0xD 0xA => 0x0D 0x00 [0xD \r is the return character][0xA \n is the newline character]
        if (*ptr_data_rx == '\n') {
            *ptr_line = '\0'; // put marker BEFORE resetting the ptr_line
            if (*(ptr_line - 1) == '\r') { // Remove the \r right before the \n as well, but only if it exists @important Handle case where \n is not prefixed with \r
                *(ptr_line - 1) = '\0';
            }
            ptr_line = line; // reset ptr to line[0] BEFORE return-ing
            ++ptr_data_rx; // advance ptr to data_rx BEFORE return-ing
            --counter_data_rx; // DECREASE counter data_rx
            // RETURN
            return line;
        }

        // Copy 1 byte & advance ptr's
        *ptr_line++ = *ptr_data_rx++;
        --counter_data_rx; // DECREASE counter data_rx
    }
}

static void _neom8n_gps_monitor_task(void *pvParameters) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_neom8n_data_t *ptr_data = &_neom8n_data;

    while (1) {
        // Start next Iteration
        /////ESP_LOGD(TAG, "\n\n***_neom8n_gps_monitor_task() NEXT ITER***\n");

        /////mjd_log_memory_statistics();

        char *line_uart = _get_next_line(UART_NUM_1);
        if (line_uart == NULL) {
            ESP_LOGW(TAG, "        _neom8n_gps_monitor_task(): line_uart == NULL (continue to next iter)");
            // CONTINUE
            continue;
        }

        // Mark data is being received (an indication that the GPS device is working)
        MJD_NEOM8N_SERVICE_LOCK();
        ptr_data->data_received = true;
        MJD_NEOM8N_SERVICE_UNLOCK();

        // Write data to internal UART0 (make monitor log output is mapped to that UART)
        ESP_LOGD(TAG, "        _neom8n_gps_monitor_task(): line_uart: %s", line_uart);

        // parse the line
        //  @doc minmea_tocoord() converts a raw coordinate to a human readable notation floating point value DD.DDD... Returns NaN for "unknown" values.
        switch (minmea_sentence_id(line_uart, false)) {
        case MINMEA_SENTENCE_RMC: {
            struct minmea_sentence_rmc frame;

            if (minmea_parse_rmc(&frame, line_uart) == true) {
                // latitude valid, and changed significantly?
                float new_latitude = minmea_tocoord(&frame.latitude);
                if ((isnan(new_latitude) == false) && (abs(new_latitude - ptr_data->latitude) > MY_GPS_LATLONG_ACCURACY_THRESHOLD)) {
                    MJD_NEOM8N_SERVICE_LOCK();
                    ptr_data->latitude = new_latitude;
                    MJD_NEOM8N_SERVICE_UNLOCK();
                    ESP_LOGD(TAG, "        _neom8n_gps_monitor_task(): $RMC Updated latitude: %f\n", ptr_data->latitude);
                }
                // longitude valid, and changed significantly?
                float new_longitude = minmea_tocoord(&frame.longitude);
                if ((isnan(new_longitude) == false) && (abs(new_longitude - ptr_data->longitude) > MY_GPS_LATLONG_ACCURACY_THRESHOLD)) {
                    MJD_NEOM8N_SERVICE_LOCK();
                    ptr_data->longitude = new_longitude;
                    MJD_NEOM8N_SERVICE_UNLOCK();
                    ESP_LOGD(TAG, "        _neom8n_gps_monitor_task(): $RMC Updated longitude: %f\n", ptr_data->longitude);
                }
            } else {
                ESP_LOGW(TAG, "        _neom8n_gps_monitor_task(): minmea_parse_rmc() invalid data format");
            }
            break;
        }

        case MINMEA_SENTENCE_GGA: {
            /*
             * frame.fix_quality = GPS Quality indicator
             *   0: Fix not valid
             *   1: GPS fix ***OK***
             *   2: Differential GPS fix, OmniSTAR VBS
             *   4: Real-Time Kinematic, fixed integers
             *   5: Real-Time Kinematic, float integers, OmniSTAR XP/HP or Location RTK
             *
             * frame.satellites_tracked = Number of satellites ("SV") in use, range from 00 through to 24+.
             */
            struct minmea_sentence_gga frame;

            if (minmea_parse_gga(&frame, line_uart)) {
                // fix quality changed?
                if (frame.fix_quality != ptr_data->fix_quality) {
                    MJD_NEOM8N_SERVICE_LOCK();
                    ptr_data->fix_quality = frame.fix_quality;
                    MJD_NEOM8N_SERVICE_UNLOCK();
                    ESP_LOGD(TAG, "        _neom8n_gps_monitor_task(): $GGA UPDATED fix quality: %i\n", ptr_data->fix_quality);
                }
                // number of satellites changed?
                if (frame.satellites_tracked != ptr_data->satellites_tracked) {
                    MJD_NEOM8N_SERVICE_LOCK();
                    ptr_data->satellites_tracked = frame.satellites_tracked;
                    MJD_NEOM8N_SERVICE_UNLOCK();
                    ESP_LOGD(TAG, "        _neom8n_gps_monitor_task(): $GGA UPDATED satellites_tracked: %d\n", ptr_data->satellites_tracked);
                }
            } else {
                ESP_LOGW(TAG, "        _neom8n_gps_monitor_task(): minmea_parse_gga() invalid data format");
            }
            break;
        }

        case MINMEA_INVALID: {
            ESP_LOGW(TAG, "        _neom8n_gps_monitor_task(): minmea_sentence_id() data MINMEA_INVALID (or unknown) for %s", line_uart);
            break;
        }

        default:
            break;
        }
    }

    // RTOS Delete service task (void)
    vTaskDelete(_neom8n_gps_monitor_task_handle);
    _neom8n_gps_monitor_task_handle = NULL;

}

/**************************************
 * PUBLIC.
 *
 */
esp_err_t mjd_neom8n_init(mjd_neom8n_config_t* ptr_param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MUTEX
    if (!_neom8n_service_semaphore) {
        _neom8n_service_semaphore = xSemaphoreCreateMutex();
        if (!_neom8n_service_semaphore) {
            f_retval = ESP_FAIL;
            ESP_LOGE(TAG, "xSemaphoreCreateMutex() err %i (%s)", f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
    }

    // Configure the UART1 controller
    uart_config_t uart_config = { .baud_rate = MY_UART_BAUD_SPEED, .data_bits = UART_DATA_8_BITS, .parity =
            UART_PARITY_DISABLE, .stop_bits = UART_STOP_BITS_1, .flow_ctrl = UART_HW_FLOWCTRL_DISABLE };
    f_retval = uart_param_config(UART_NUM_1, &uart_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "uart_param_config() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = uart_set_pin(ptr_param_config->uart_port, ptr_param_config->uart_tx_gpio_num,
            ptr_param_config->uart_rx_gpio_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "uart_set_pin() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = uart_driver_install(ptr_param_config->uart_port, MY_UART_RX_RINGBUFFER_SIZE, 0, 0, NULL, 0);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "uart_driver_install() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // UBX Power up
    mjd_neom8n_power_up(ptr_param_config);

    // Do a GPS Cold Start if requested so
    if (ptr_param_config->do_cold_start == true) {
        mjd_neom8n_cold_start_forced(ptr_param_config);
    }

    // RTOS Create service task
    //      @important For stability (RMT + Wifi etc.): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate() which might run the code on PRO_CPU_NUM...]
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(_neom8n_gps_monitor_task, "_neom8n_gps_monitor_task",
    MJD_NEOM8N_GPS_MONITOR_TASK_STACK_SIZE, NULL, RTOS_TASK_PRIORITY_NORMAL,
            (TaskHandle_t * const ) (&_neom8n_gps_monitor_task_handle), APP_CPU_NUM);
    if (xReturned != pdPASS) {
        ESP_LOGE(TAG, "xTaskCreatePinnedToCore(_neom8n_gps_monitor_task_handle() err %i (%s)", xReturned, "!=pdPASS");
        f_retval = ESP_FAIL;
        // GOTO
        goto cleanup;
    }

    // INIT My static Data structure _neom8n_data
    mjd_neom8n_data_t *ptr_data = &_neom8n_data;
    ptr_data->data_received = false;
    ptr_data->fix_quality = -1;
    ptr_data->latitude = NAN;
    ptr_data->longitude = NAN;
    ptr_data->satellites_tracked = -1;

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_neom8n_deinit(mjd_neom8n_config_t* ptr_param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // RTOS Delete service task (void)
    if (_neom8n_gps_monitor_task_handle != NULL) {
        vTaskDelete(_neom8n_gps_monitor_task_handle);
        _neom8n_gps_monitor_task_handle = NULL;
    }

    // UBX Power down
    mjd_neom8n_power_down(ptr_param_config);
    vTaskDelay(RTOS_DELAY_15SEC);

    // Configure the UART1 controller
    f_retval = uart_driver_delete(ptr_param_config->uart_port);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "uart_driver_delete() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Do not erase my static Data structure _neom8n_data so the app can still retrieve the last readings (even when the GPS device is down)

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_neom8n_read(mjd_neom8n_config_t* ptr_param_config, mjd_neom8n_data_t* ptr_param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // Copy the actual data from my static Data structure (filled async'ly from the monitor task)
    MJD_NEOM8N_SERVICE_LOCK();
    mjd_neom8n_data_t *ptr_data = &_neom8n_data;
    ptr_param_data->data_received = ptr_data->data_received;
    ptr_param_data->fix_quality = ptr_data->fix_quality;
    ptr_param_data->latitude = ptr_data->latitude;
    ptr_param_data->longitude = ptr_data->longitude;
    ptr_param_data->satellites_tracked = ptr_data->satellites_tracked;
    MJD_NEOM8N_SERVICE_UNLOCK();

    // LABEL
    /////cleanup: ;

    return f_retval;
}

/**************************************
 * DEVICE STATE MANAGERS
 *
 * Some useful commands to control the device startup, device power state, and state of the GNSS Receiver.
 * Some useful commands to control the measurement rate related commands to control the GPS device.
 * These commands are optional.
 *
 */

/**
 * @brief Cold Start the device
 *
 * All satellite information is lost so it will take a while to get a 3D Fix on the satellites again.
 *
 * @param uart_num UART_NUM_0, UART_NUM_1 or UART_NUM_2
 *
 * @return
 *     - (-1) Parameter error
 *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
 */
esp_err_t mjd_neom8n_cold_start_forced(mjd_neom8n_config_t* ptr_param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // ===UBX-CFG-RST: Reset Receiver / Clear Backup Data Structures=== Startup=Coldstart + Reset=Forced(Watchdog)
    char data[] = { 0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0xFF, 0xB9, 0x00, 0x00, 0xC6, 0x8B };

    f_retval = uart_write_bytes(ptr_param_config->uart_port, data, ARRAY_SIZE(data));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
    }

    // @important Wait 5seconds so the device can boot properly BEFORE communicating with the GPS device.
    vTaskDelay(RTOS_DELAY_5SEC);

    return f_retval;
}

/**
 * @brief Power down the device and wakeup after 15 seconds.
 *
 * @important I wakes up earlier if you send it whatever UBX command.
 *
 * @param uart_num UART_NUM_0, UART_NUM_1 or UART_NUM_2
 *
 * @return
 *     - (-1) Parameter error
 *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
 */
esp_err_t mjd_neom8n_power_down_for_15_seconds(mjd_neom8n_config_t* ptr_param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // ===UBX-RXM-PMREQ: Requests a Power Management task=== Version=0 Duration=15000 Action=2: 15 seconds
    char data[] = { 0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x98, 0x3A, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x1F, 0x91 };

    f_retval = uart_write_bytes(ptr_param_config->uart_port, data, ARRAY_SIZE(data));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
    }

    return f_retval;
}

/**
 * @brief Power down the device infinitely (wakeup after 12 days).
 *
 * @important I wakes up earlier if you send it whatever UBX command.
 *
 * @param uart_num UART_NUM_0, UART_NUM_1 or UART_NUM_2
 *
 * @return
 *     - (-1) Parameter error
 *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
 */
esp_err_t mjd_neom8n_power_down(mjd_neom8n_config_t* ptr_param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // ===UBX-RXM-PMREQ: Requests a Power Management task=== Version=0 Duration=0 Action=2: infinite (=12 days!) It ALSO wakes up when you send whatever UBX command...
    char data[] = { 0xB5, 0x62, 0x02, 0x41, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x4D, 0x3B };

    f_retval = uart_write_bytes(ptr_param_config->uart_port, data, ARRAY_SIZE(data));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
    }

    return f_retval;
}

/**
 * @brief Power up the device after a power down cycle.
 *
 * The device will wake up after sending whatever (even invalid) data sequence to the device.
 *
 * @param uart_num UART_NUM_0, UART_NUM_1 or UART_NUM_2
 *
 * @return
 *     - (-1) Parameter error
 *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
 */
esp_err_t mjd_neom8n_power_up(mjd_neom8n_config_t* ptr_param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    char data[] = { 0xB5, 0x62 };

    f_retval = uart_write_bytes(ptr_param_config->uart_port, data, ARRAY_SIZE(data));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
    }

    return f_retval;
}

/**
 * @brief Stop the GNSS.
 *
 * Stop the GNSS Global Navigation Satellite System. Saves the actual satellite positions in memory.
 *
 * @param uart_num UART_NUM_0, UART_NUM_1 or UART_NUM_2
 *
 * @return
 *     - (-1) Parameter error
 *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
 */
esp_err_t mjd_neom8n_gnss_stop(mjd_neom8n_config_t* ptr_param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // ===UBX-CFG-RST: Reset Receiver / Clear Backup Data Structures=== GNSS=stop + Startup=User Defined + Reset=*NONE: (only stops GNSS  = low power!)
    char data[] = { 0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x08, 0x00, 0x16, 0x74 };

    f_retval = uart_write_bytes(ptr_param_config->uart_port, data, ARRAY_SIZE(data));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
    }

    return f_retval;
}

/**
 * @brief Start the GNSS.
 *
 * Start the GNSS stands for Global Navigation Satellite System. Restores the last known satellite positions.
 *
 * @param uart_num UART_NUM_0, UART_NUM_1 or UART_NUM_2
 *
 * @return
 *     - (-1) Parameter error
 *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
 */
esp_err_t mjd_neom8n_gnss_start(mjd_neom8n_config_t* ptr_param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // ===UBX-CFG-RST: Reset Receiver / Clear Backup Data Structures=== GNSS=start + Startup=User Defined + Reset=*NONE: (only starts GNSS = normal power!)
    char data[] = { 0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x09, 0x00, 0x17, 0x76 };

    f_retval = uart_write_bytes(ptr_param_config->uart_port, data, ARRAY_SIZE(data));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
    }

    return f_retval;
}

/**
 * @brief Set the Measurement Rate 1000ms (the default rate).
 *
 * Each measurement triggers the measurements generation and raw data output. 1000ms is
 *
 * @param uart_num UART_NUM_0, UART_NUM_1 or UART_NUM_2
 *
 * @return
 *     - (-1) Parameter error
 *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
 */
esp_err_t mjd_neom8n_set_measurement_rate_1000ms(mjd_neom8n_config_t* ptr_param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // ===UBX-CFG-RATE: Navigation/Measurement Rate Settings=== TimeSource="1-GPS Time" MeasurementPeriod=1000ms: ***DEFAULT***
    char data[] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0xE8, 0x03, 0x01, 0x00, 0x01, 0x00, 0x01, 0x39};

    f_retval = uart_write_bytes(ptr_param_config->uart_port, data, ARRAY_SIZE(data));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
    }

    return f_retval;
}

/**
 * @brief Set the Measurement Rate 100ms (faster rate).
 *
 * Each measurement triggers the measurements generation and raw data output. 1000ms is the default rate.
 *
 * @param uart_num UART_NUM_0, UART_NUM_1 or UART_NUM_2
 *
 * @return
 *     - (-1) Parameter error
 *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
 */
esp_err_t mjd_neom8n_set_measurement_rate_100ms(mjd_neom8n_config_t* ptr_param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // ===UBX-CFG-RATE: Navigation/Measurement Rate Settings=== TimeSource="1-GPS Time" MeasurementPeriod=100ms:
    char data[] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00, 0x7A, 0x12};

    f_retval = uart_write_bytes(ptr_param_config->uart_port, data, ARRAY_SIZE(data));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
    }

    return f_retval;
}

/**
 * @brief Set the Measurement Rate 5000ms (slower rate).
 *
 * Each measurement triggers the measurements generation and raw data output. 1000ms is the default rate.
 *
 * @param uart_num UART_NUM_0, UART_NUM_1 or UART_NUM_2
 *
 * @return
 *     - (-1) Parameter error
 *     - OTHERS (>=0) The number of bytes pushed to the TX FIFO
 */
esp_err_t mjd_neom8n_set_measurement_rate_5000ms(mjd_neom8n_config_t* ptr_param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // ===UBX-CFG-RATE: Navigation/Measurement Rate Settings=== TimeSource="1-GPS Time" MeasurementPeriod=5000ms:
    char data[] = {0xB5, 0x62, 0x06, 0x08, 0x06, 0x00, 0x88, 0x13, 0x01, 0x00, 0x01, 0x00, 0xB1, 0x49};

    f_retval = uart_write_bytes(ptr_param_config->uart_port, data, ARRAY_SIZE(data));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
    }

    return f_retval;
}

