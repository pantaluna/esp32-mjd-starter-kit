#include "mjd.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig:
 */

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * UART1 settings
 *  @rule ring buffer size > 128
 */
#define MY_LORABEE_RESET_GPIO_NUM (14)

#define MY_UART_NUM         (UART_NUM_1)
#define MY_UART_TX_GPIO_NUM (22)
#define MY_UART_RX_GPIO_NUM (23)
#define MY_UART_RX_BUFFER_SIZE          (512)
#define MY_UART_RX_BUFFER_SIZE_PLUS_ONE (513)
#define MY_UART_RX_RINGBUFFER_SIZE      (512 * 2)
#define MY_UART_RX_TIMEOUT (RTOS_DELAY_100MILLISEC)

static QueueHandle_t _uart1_queue;

/*
 * hELpErS
 * TOD String to HEX convertor
 * TODO Method for radio_tx because it returns 2 responses (one that cmd was received, one that data was transmitted or in error)
 * TODO Method for sleep because it does not give back a response
 */

static char* _get_next_line_uart(uart_port_t param_uart_port) {
    static char line[MY_UART_RX_BUFFER_SIZE] = "";
    char *ptr_line = line;

    static uint8_t data_rx[MY_UART_RX_BUFFER_SIZE_PLUS_ONE]; // @important +1 for the appended \0 character
    static uint8_t *ptr_data_rx;
    static int counter_data_rx = 0;    // @important 0 means that the data_rx buffer is empty

    while (1) {
        if (counter_data_rx <= 0) {
            // Read data from external UART1
            counter_data_rx = uart_read_bytes(param_uart_port, data_rx, MY_UART_RX_BUFFER_SIZE, MY_UART_RX_TIMEOUT);
            if (counter_data_rx == ESP_FAIL) {
                ESP_LOGE(TAG, "    _get_next_line_uart(): uart_read_bytes() err %i (%s)", counter_data_rx, esp_err_to_name(counter_data_rx));
                // ABORT
                return NULL;
            }
            if (counter_data_rx == 0) {
                ESP_LOGW(TAG, "    _get_next_line_uart(): uart_read_bytes() 0 bytes returned");
                // ABORT
                return NULL;
            }

            // DEVTEMP
            /////ESP_LOGV(TAG, "    _get_next_line_uart(): ESP_LOG_BUFFER_HEXDUMP(): data_rx");
            /////ESP_LOG_BUFFER_HEXDUMP(TAG, data_rx, counter_data_rx, ESP_LOG_VERBOSE);

            ptr_data_rx = data_rx; // reset ptr to the newly read data from uart
        }

        // Detect new line pattern \r\n [detect end of new response from Microchip] and RETURN the accumulated data (without \r\n)
        //   @doc 0xD 0xA => 0x00 0x00 [0xD \r is the return character][0xA \n is the newline character]
        if (*ptr_data_rx == '\n') {
            *ptr_line = '\0'; // put marker BEFORE resetting the ptr_line
            if (*(ptr_line - 1) == '\r') { // Remove the \r right before the \n as well, but only if it exists @important Handle case where \n is not prefixed with \r
                *(ptr_line - 1) = '\0';
            }
            ptr_line = line;   // reset ptr to line[0] BEFORE return-ing
            ++ptr_data_rx;     // advance ptr of data_rx BEFORE return-ing
            --counter_data_rx; // DECREASE counter data_rx BEFORE return-ing
            // RETURN data
            return line;
        }

        // Copy 1 byte & advance ptr's afterwards
        *ptr_line++ = *ptr_data_rx++;
        --counter_data_rx; // DECREASE counter data_rx
    }
}

static esp_err_t _cmd(const char* param_ptr_command, int param_rtos_delay) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    char command[320] = ""; // size at least = command byte length + max lora payload of 255 bytes + \r\n
    strcpy(command, param_ptr_command);
    ESP_LOGD(TAG, "command (without cr newline): len=%zu val=%s", strlen(command), command);
    strcat(command, "\r\n");

    f_retval = uart_write_bytes(MY_UART_NUM, command, strlen(command));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // mandatory wait after TX!
    vTaskDelay(param_rtos_delay);

    // RX response(s)
    char *line_uart;
    int len_line_uart;

    // RX response#1 (except `sys sleep`)
    if (mjd_string_starts_with(command, "sys sleep") == false) {
        ESP_LOGD(TAG, "Wait for RX response#1");
        line_uart = _get_next_line_uart(MY_UART_NUM);
        if (line_uart == NULL) {
            ESP_LOGW(TAG, "    _cmd(): line_uart == NULL (continue)");
            // GOTO
            goto cleanup;
        }
        len_line_uart = strlen(line_uart);
        ESP_LOGI(TAG, "line_uart=(HEXDUMP)");
        ESP_LOG_BUFFER_HEXDUMP(TAG, line_uart, len_line_uart, ESP_LOG_DEBUG);
    }

    // RX response#2 (only for `radio tx`)
    if (mjd_string_starts_with(command, "radio tx") == true) {
        ESP_LOGD(TAG, "Wait for RX response#2");
        line_uart = _get_next_line_uart(MY_UART_NUM);
        if (line_uart == NULL) {
            ESP_LOGW(TAG, "    _cmd(): line_uart#2 == NULL (continue)");
            // GOTO
            goto cleanup;
        }
        len_line_uart = strlen(line_uart);
        ESP_LOGI(TAG, "line_uart#2=(HEXDUMP)");
        ESP_LOG_BUFFER_HEXDUMP(TAG, line_uart, len_line_uart, ESP_LOG_DEBUG);
    }
    // LABEL
    cleanup: ;

    return f_retval; // @special f_retval = nbr of bytes transmitted (or -1 for error)(or 0 for no data transmitted but not in error)
}

/********************************************************************************
 * LoraBee: RESET COMMAND
 *   @dep I sacrificed an extra pin for this feature: MY_LORABEE_RESET_GPIO_NUM
 *   @pcb RN2483 PIN #32 "RESET" Input Active-low device Reset | LoraBee Xbee Board PIN#17 ("IO7") | Parallax Xbee Adapter Board PIN#IO3
 *   @dep The pads of SJ1 on the LoraBee board must be solder-joined.
 *   @logic GPIO Config + Set LOW for 1 millisec (should reset the device!) + Set HIGH again
 *   @important The device is available for use again no earlier than 100 millisecond after the reset
 */
static esp_err_t _cmd_reset() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    gpio_config_t reset_io_conf =
        { 0 };

    ESP_LOGI(TAG, "[Strict timing is required - do not put the log() calls between the time-sensitive commands]");
    ESP_LOGI(TAG, "gpio_config();");
    ESP_LOGI(TAG, "gpio_set_level(MY_LORABEE_RESET_GPIO_NUM, 0);");
    ESP_LOGI(TAG, "delay 1 millisec (arbitrarily)");
    ESP_LOGI(TAG, "gpio_set_level(MY_LORABEE_RESET_GPIO_NUM, 1);");

    reset_io_conf.pin_bit_mask = (1ULL << MY_LORABEE_RESET_GPIO_NUM);
    reset_io_conf.mode = GPIO_MODE_OUTPUT;
    reset_io_conf.pull_up_en = GPIO_PULLUP_ENABLE;    // UART standard = pullup
    reset_io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    reset_io_conf.intr_type = GPIO_INTR_DISABLE;
    f_retval = gpio_config(&reset_io_conf);

    f_retval = gpio_set_level(MY_LORABEE_RESET_GPIO_NUM, 0);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "gpio_set_level() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    ets_delay_us(1 * 1000);

    f_retval = gpio_set_level(MY_LORABEE_RESET_GPIO_NUM, 1);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "gpio_set_level() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Wait 1 sec to let the device come alive
    vTaskDelay(RTOS_DELAY_100MILLISEC);

    // LABEL
    cleanup: ;

    return f_retval;
}

/********************************************************************************
 * LoraBee: WAKEUP COMMAND
 *   @doc The host system needs to transmit to the MicroChip module a BREAK condition followed by one 0x55 character ('U') at the new baud rate.
 *   @doc A break condition is signaled to the module by keeping the UART_TX pin low for longer than the time to transmit a complete character (1 char = 10 bits) at that baudrate. Choose 10x2 bits to be sure!
 *   @logic UART 9600: 1 char = +-1millisec so a BREAK := set Low for at least 1+1 millisec + set High again
 *
 *   @problem Cannot use uart_write_bytes_with_break() because a) Sends the BREAK after the data! 2) The data cannot be empty!
 */
static esp_err_t _cmd_wakeup() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // Simulate a BREAK condition: inverse the standard TX signal level from HIGH to LOW for a specific duration
    uart_set_line_inverse(MY_UART_NUM, UART_INVERSE_TXD);
    ets_delay_us(5 * 1000);
    uart_set_line_inverse(MY_UART_NUM, UART_INVERSE_DISABLE);

    // TX 'U'
    char autobaud_char = 0x55;
    f_retval = uart_write_bytes(MY_UART_NUM, &autobaud_char, sizeof(autobaud_char));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    // A standard delay of 100 milliseconds after a standard RN2843 command (except after transmitting data which takes much much longer to get a response!)
    vTaskDelay(RTOS_DELAY_100MILLISEC);

    // Mark OK
    f_retval = ESP_OK;

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    // LABEL
    cleanup: ;

    return f_retval;
}

/*
 * TASKS
 */
static void _uart_event_task(void *pvParameters) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    static const char *EVENT_TASK_TAG = "EVENT_TASK";
    esp_log_level_set(EVENT_TASK_TAG, ESP_LOG_INFO);

    uart_event_t event;
    for (;;) {
        // Wait for UART event.
        if (xQueueReceive(_uart1_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            switch (event.type) {
            case UART_DATA:
                /*
                 // Event of UART receving data. Handle the data event very fast else the queue might be full.
                 ESP_LOGI(EVENT_TASK_TAG, "[event: UART DATA] size=%d", event.size);

                 uart_read_bytes(MY_UART_NUM, dtmp, event.size, RTOS_DELAY_MAX);
                 ESP_LOGI(EVENT_TASK_TAG, "[event: UART DATA] data=%s", (const char* ) dtmp);
                 ESP_LOG_BUFFER_HEXDUMP(EVENT_TASK_TAG, dtmp, event.size + 1, ESP_LOG_INFO);  // +1 to see what is there
                 */
                break;
            case UART_FIFO_OVF:
                // Event: HW FIFO overflow detected
                ESP_LOGI(EVENT_TASK_TAG, "[EVENT: hw fifo overflow]");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // As an example, we directly flush the rx buffer and reset the queue in order to read more data.
                uart_flush_input(MY_UART_NUM);
                xQueueReset(_uart1_queue);
                break;
            case UART_BUFFER_FULL:
                // Event: of UART ring buffer full
                // If buffer full happened, you should consider increasing your buffer size. As an example, we directly flush the rx buffer and reset the queue in order to read more data.
                ESP_LOGI(EVENT_TASK_TAG, "[EVENT: event ring buffer full");
                uart_flush_input(MY_UART_NUM);
                xQueueReset(_uart1_queue);
                break;
            case UART_BREAK:
                // Event: UART RX break detected
                ESP_LOGI(EVENT_TASK_TAG, "[EVENT: event uart rx break");
                break;
            case UART_PARITY_ERR:
                // Event: UART parity check error
                ESP_LOGI(EVENT_TASK_TAG, "[EVENT: event uart parity error");
                break;
            case UART_FRAME_ERR:
                // Event: UART frame error
                ESP_LOGI(EVENT_TASK_TAG, "[EVENT: event uart frame error");
                break;
            case UART_PATTERN_DET:
                //Event: UART pattern detected
                ESP_LOGI(EVENT_TASK_TAG, "[EVENT: UART PATTERN DETECTED]");
                break;
            default:
                // Event: UNKNOWN!
                ESP_LOGI(EVENT_TASK_TAG, "[EVENT: unknown uart event type %d!!!]", event.type);
                break;
            }
        }
    }
    vTaskDelete(NULL);
}

void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /********************************************************************************
     * STANDARD Init
     */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer, let peripherals become active, ...)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /********************************************************************************
     * UART1 Setup
     */
    uart_config_t uart_config =
        { .baud_rate = 57600, // @validvalues 9600 57600 115200 921600
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE };
    f_retval = uart_param_config(MY_UART_NUM, &uart_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "uart_param_config() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = uart_set_pin(MY_UART_NUM, MY_UART_TX_GPIO_NUM, MY_UART_RX_GPIO_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "uart_set_pin() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = uart_driver_install(MY_UART_NUM, MY_UART_RX_RINGBUFFER_SIZE, 0, 20, &_uart1_queue, 0);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "uart_driver_install() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /**********
     * CREATE TASK: monitor UART events (such as DATA and errors)
     */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&_uart_event_task, "_uart_event_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task has been created, and is running right now");
    }
    // Wait to let the task come alive
    vTaskDelay(RTOS_DELAY_100MILLISEC);

    /********************************************************************************
     * LoraBee Reset
     */
    /*
     f_retval = _cmd_reset();
     if (f_retval != ESP_OK) {
     // GOTO
     goto cleanup;
     }

     // DEVTEMP: HALT
     /////mjd_rtos_wait_forever();
     */

    /********************************************************************************
     * LoraBee Wakeup
     */

    f_retval = _cmd_wakeup();
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    /********************************************************************************
     * UART1: send some Lora commands
     *   @doc https://www.disk91.com/2015/technology/networks/first-step-in-lora-land-microchip-rn2483-test/
     */

    // PREPARE: flush any old data in the RX buffer
    uart_flush_input(MY_UART_NUM);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    // TX Command. GPIO0 = LED
    f_retval = _cmd("sys set pinmode GPIO0 digout", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }
    f_retval = _cmd("sys set pindig GPIO0 1", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    // TX Command. the preprogrammed EUI node address
    f_retval = _cmd("sys get hweui", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    // TX Command: disable LoraWAN (and implicitly enable lora)
    f_retval = _cmd("mac pause", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    // TX Command
    f_retval = _cmd("radio get wdt", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    // TX Command
    f_retval = _cmd("radio get mod", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    // TX Command
    f_retval = _cmd("radio get freq", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    // TX Command
    f_retval = _cmd("radio get sf", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    // TX Command
    f_retval = _cmd("radio get pwr", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    // TX Command
    f_retval = _cmd("radio get snr", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    // TX Command: the configured bit rate for FSK communications
    f_retval = _cmd("radio get bitrate", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    // TX Command
    f_retval = _cmd("radio set pwr 14", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    // TX Command
    /*
     f_retval = _cmd("radio set wdt 0", RTOS_DELAY_100MILLISEC);
     if (f_retval == ESP_FAIL) {
     // GOTO
     goto cleanup;
     }
     */

    // TX Command: @important Mandatory LONG WAIT 2SEC AFTER SEND PACKET (time varies with quality of Lora network/environment!)
    char cmd_radio_tx[320];
    for (uint32_t i = 1; i <= 5; ++i) {
        sprintf(cmd_radio_tx, "radio tx %02X0102030405060708090a0b0c0d0e0f10", i);
        f_retval = _cmd(cmd_radio_tx, RTOS_DELAY_3SEC);
        if (f_retval == ESP_FAIL) {
            // GOTO
            goto cleanup;
        }
    }

    // TX Command GPIO0 = LED
    f_retval = _cmd("sys set pindig GPIO0 0", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    // TX Command
    /*
     f_retval = _cmd("radio set wdt 15000", RTOS_DELAY_100MILLISEC);
     if (f_retval == ESP_FAIL) {
     // GOTO
     goto cleanup;
     }
     */

    // TX Command: SYS SLEEP [+-49 days]
    f_retval = _cmd("sys sleep 4294967295", RTOS_DELAY_100MILLISEC);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    /********************************************************************************
     * LABEL
     */
    cleanup: ;

    /********************************************************************************
     * Task Delete
     * @doc Passing NULL will end the current task
     */
    ESP_LOGI(TAG, "END OF %s()", __FUNCTION__);
    vTaskDelete(NULL);
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /**********
     * CREATE TASK:
     * @important For stability (RMT + Wifi etc.): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate()]
     */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task has been created, and is running right now");
    }

    /**********
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
