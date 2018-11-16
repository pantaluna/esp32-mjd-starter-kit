/*
 * Goto the README.md for instructions
 *
 */

/*
 * Includes: system, own
 */
#include "mjd.h"
#include "mjd_lorabee.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_lorabee";

/*
 * UART settings
 *  @rule baud speed MAX 9600 for the Ublox NEO-M8N GPS board. Other (USB) UART boards might support higher speeds.
 *  @rule ring buffer size > 128
 *  @rule ring buffer size = at least 2x the RX buffer size
 */
static QueueHandle_t _uart_driver_queue = NULL;
static QueueHandle_t _uart_rx_data_queue = NULL;

#define MJD_LORABEE_UART_BAUD_SPEED              (57600)
#define MJD_LORABEE_UART_RX_BUFFER_SIZE          (512)
#define MJD_LORABEE_UART_RX_BUFFER_SIZE_PLUS_ONE (513)
#define MJD_LORABEE_UART_RX_RINGBUFFER_SIZE       (512 * 2)

#define MJD_LORABEE_UART_DRIVER_QUEUE_SIZE      (20)
#define MJD_LORABEE_UART_RX_DATA_QUEUE_SIZE     (20)

/*
 * MUTEX
 * @doc For future use.
 */
static SemaphoreHandle_t _lorabee_service_semaphore = NULL;
#define MJD_LORABEE_SERVICE_LOCK()     xSemaphoreTake(_lorabee_service_semaphore, portMAX_DELAY)
#define MJD_LORABEE_SERVICE_UNLOCK()   xSemaphoreGive(_lorabee_service_semaphore)

/*
 * Requests & Responses
 *  MJD_LORABEE_TX_COMMAND_MAX_SIZE @unit=bytes The max command length + max Lora TX payload (in HEX chars) + \r\n
 *
 */
#define MJD_LORABEE_TX_COMMAND_MAX_SIZE (10 + (MJD_LORABEE_LORA_TX_PAYLOAD_MAX_BYTES*2))
#define MJD_LORABEE_REQUEST_PREFIX_RADIO_TX "radio tx"" "
#define MJD_LORABEE_RESPONSE_PREFIX_RADIO_RX "radio_rx"" "" "

/***
 * Responses and RELATED Error Codes and Messages
 *   @doc Special feature of the C Preprocessor to convert a defined NAME to a string representation of that name (handy!).
 *
 */
#define MJD_LORABEE_RESPONSE_OK "ok"
#define MJD_LORABEE_RESPONSE_BUSY "busy"
#define MJD_LORABEE_RESPONSE_CANNOT_MAC_PAUSE "0"
#define MJD_LORABEE_RESPONSE_INVALID_PARAM "invalid_param"
#define MJD_LORABEE_RESPONSE_RADIO_ERROR "radio_err"
#define MJD_LORABEE_RESPONSE_RADIO_TX_OK "radio_tx_ok"

typedef enum {
    MJD_LORABEE_OK = 0, // first
    MJD_LORABEE_ERROR_BUSY,
    MJD_LORABEE_ERROR_CANNOT_MAC_PAUSE,
    MJD_LORABEE_ERROR_INVALID_PARAM,
    MJD_LORABEE_RADIO_ERROR, // 1. TX: if transmission was unsuccessful (interrupted by radio Watchdog Timer time-out) 2. RX: if reception was not successful, reception time-out occurred
    MJD_LORABEE_RADIO_TX_OK,
    MJD_LORABEE_ERROR_MAX, // last
} mjd_lorabee_error_t;

#define MJD_LORABEE_ADD_ERROR_ITEM(err)  {err, #err}

typedef struct {
    int32_t code;
    const char *msg;
} mjd_lorabee_err_msg_t;

static const char _err_unknown_msg[] = "UNKNOWN ERROR MSG";

static const mjd_lorabee_err_msg_t _err_msg_table[] =
    {
    MJD_LORABEE_ADD_ERROR_ITEM(MJD_LORABEE_OK),
MJD_LORABEE_ADD_ERROR_ITEM(MJD_LORABEE_ERROR_BUSY),
MJD_LORABEE_ADD_ERROR_ITEM(MJD_LORABEE_ERROR_CANNOT_MAC_PAUSE),
MJD_LORABEE_ADD_ERROR_ITEM(MJD_LORABEE_ERROR_INVALID_PARAM),
MJD_LORABEE_ADD_ERROR_ITEM(MJD_LORABEE_RADIO_ERROR),
MJD_LORABEE_ADD_ERROR_ITEM(MJD_LORABEE_RADIO_TX_OK), };

const char *mjd_lorabee_err_to_name(int32_t code) {
    for (int i = 0; i < ARRAY_SIZE(_err_msg_table); ++i) {
        if (_err_msg_table[i].code == code) {
            return _err_msg_table[i].msg;
        }
    }
    return _err_unknown_msg;
}

/**************************************
 * UART EVENTS TASK
 *
 */
#define MY_LORABEE_TASK_UART_EVENTS_TASK_STACK_SIZE (8192)

static TaskHandle_t _uart_events_task_handle = NULL;

static void _task_delete_using_handle(TaskHandle_t* param_handle) {
    if (param_handle != NULL) {
        vTaskDelete(*param_handle);
        param_handle = NULL;
    }
}

static void _uart_events_task(void *pvParameters) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    static const char *EVENT_TASK_TAG = "UART EVENT_TASK";

    uart_event_t event;
    uart_event_t dq_event;
    for (;;) {
        // Wait for UART event.
        if (xQueueReceive(_uart_driver_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            switch (event.type) {
            case UART_DATA:
                dq_event.type = event.type;
                dq_event.size = event.size;
                if (xQueueSend(_uart_rx_data_queue, &dq_event, RTOS_DELAY_0) != pdTRUE) {
                    ESP_LOGE(TAG, "%s(). xQueueSend() failed", __FUNCTION__);
                }
                ESP_LOGD(EVENT_TASK_TAG, "[event: UART rx data] size=%d", event.size);
                break;
            case UART_BREAK:
                ESP_LOGD(EVENT_TASK_TAG, "[EVENT: event RX break]");
                break;
            case UART_DATA_BREAK:
                ESP_LOGD(EVENT_TASK_TAG, "[EVENT: event TX data_break]");
                break;
            case UART_BUFFER_FULL:
                ESP_LOGW(EVENT_TASK_TAG, "[EVENT: error event RX ring buffer full]");
                xQueueReset(_uart_driver_queue);
                break;
            case UART_FIFO_OVF:
                // "If fifo overflow happened, you should consider adding flow control for your application."
                xQueueReset(_uart_driver_queue);
                ESP_LOGW(EVENT_TASK_TAG, "[EVENT: error hw fifo overflow]");
                break;
            case UART_FRAME_ERR:
                ESP_LOGW(EVENT_TASK_TAG, "[EVENT: error event RX frame error]");
                break;
            case UART_PARITY_ERR:
                ESP_LOGW(EVENT_TASK_TAG, "[EVENT: error event RX parity error]");
                break;
            case UART_PATTERN_DET:
                ESP_LOGD(EVENT_TASK_TAG, "[EVENT: RX pattern detected]");
                break;
            default:
                ESP_LOGD(EVENT_TASK_TAG, "[EVENT: unhandled event type %d]", event.type);
                break;
            }
        }
    }
}

/**************************************
 * HELPERS
 *
 */

/*
 * @brief flush any old data in the UART RX buffer & reset the uart queue
 *
 */
static esp_err_t _uart_flush_queue_reset(mjd_lorabee_config_t* param_ptr_config) {
    uart_flush_input(param_ptr_config->uart_port_num);
    xQueueReset(_uart_driver_queue);
    return ESP_OK;
}

/*
 * @brief Read the next line ending with \r\n from an UART Port.
 *
 * @important A pointer to the function's static variable is returned to the caller.
 *
 * TODO Change retval to a receive ptr var (goal: separate error codes and returning string).
 *
 */
static char* _get_next_line_uart(uart_port_t param_uart_port_num) {
    // static => re-used!
    static char _line[MJD_LORABEE_UART_RX_BUFFER_SIZE] = "";
    char *_ptr_line = _line;

    static uint8_t _data_rx[MJD_LORABEE_UART_RX_BUFFER_SIZE_PLUS_ONE]; // @important +1 for the appended \0 character
    static uint8_t *_ptr_data_rx;
    static int _counter_data_rx = 0;    // @important 0 means that the data_rx buffer is empty

    uart_event_t dq_event;

    while (1) {
        if (_counter_data_rx <= 0) {
            // Wait for RXDATA event.
            if (xQueueReceive(_uart_rx_data_queue, (void * )&dq_event, RTOS_DELAY_30SEC) == pdTRUE) { // RTOS_DELAY_30SEC RTOS_DELAY_5MINUTES
                // Read data from UART1
                // @important No delay needed because we know via the _uart_rx_data_queue that data is waiting :)
                _counter_data_rx = uart_read_bytes(param_uart_port_num, _data_rx, dq_event.size, RTOS_DELAY_0);

                ESP_LOGD(TAG, "%s(): _counter_data_rx from uart_read_bytes(): %i", __FUNCTION__, _counter_data_rx);

                if (_counter_data_rx == ESP_FAIL) {
                    mjd_log_time();
                    ESP_LOGE(TAG, "    %s(). uart_read_bytes() err %i (%s)", __FUNCTION__, _counter_data_rx, esp_err_to_name(_counter_data_rx));
                    // ABORT
                    return NULL;
                }

                if (_counter_data_rx == 0) {
                    mjd_log_time();
                    ESP_LOGD(TAG, "    %s(). uart_read_bytes() returned 0 bytes -> read again from uart until some response comes back...",
                            __FUNCTION__);
                    // CONTINUE @important!
                    continue;
                }

                // DEVTEMP (verbose)
                ESP_LOGV(TAG, "    %s(): HEXDUMP data_rx (=original from uart_read_bytes())", __FUNCTION__);
                ESP_LOG_BUFFER_HEXDUMP(TAG, _data_rx, _counter_data_rx, ESP_LOG_VERBOSE);
                // DEVTEMP-END

                _ptr_data_rx = _data_rx; // reset ptr to the newly read data from uart
            } else {
                mjd_log_time();
                ESP_LOGW(TAG, "%s(): xQueueReceive() _uart_rx_data_queue time out (30 sec), continue", __FUNCTION__);
                // CONTINUE @important!
                continue;
            }
        }

        // Detect newline pattern \r\n (Detect end of new response from Microchip, and RETURN the accumulated data without \r\n)
        //   @doc Change 0xD 0xA => 0x00 0x00 (0xD \r is the return character)(0xA \n is the newline character)
        if (*_ptr_data_rx == '\n') {
            ESP_LOGD(TAG, "%s(). Removing \\r\\n from result", __FUNCTION__);
            *_ptr_line = '\0'; // put marker BEFORE resetting the _ptr_line
            if (*(_ptr_line - 1) == '\r') { // Remove the \r right before the \n as well, but only if it exists @important Handle case where \n is not prefixed with \r
                *(_ptr_line - 1) = '\0';
            }
            _ptr_line = _line;   // reset ptr to line[0] BEFORE return-ing
            ++_ptr_data_rx;     // advance ptr of data_rx BEFORE return-ing
            --_counter_data_rx; // DECREASE counter data_rx BEFORE return-ing
            // RETURN data
            return _line;
        }

        // Copy 1 byte & advance both pointers afterwards
        *_ptr_line++ = *_ptr_data_rx++;
        --_counter_data_rx; // DECREASE counter data_rx
    }
}

static int _response_text_to_code(const char *param_ptr_response) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    // @important This is the default code value for text values that are NOT CHECKED in the logic below. Especially for case 'radio_rx<space><space><data>'!
    int error_code = MJD_LORABEE_OK;

    if (strcmp(param_ptr_response, MJD_LORABEE_RESPONSE_OK) == 0) {
        error_code = MJD_LORABEE_OK;
    } else if (strcmp(param_ptr_response, MJD_LORABEE_RESPONSE_BUSY) == 0) {
        error_code = MJD_LORABEE_ERROR_BUSY;
    } else if (strcmp(param_ptr_response, MJD_LORABEE_RESPONSE_INVALID_PARAM) == 0) {
        error_code = MJD_LORABEE_ERROR_INVALID_PARAM;
    } else if (strcmp(param_ptr_response, MJD_LORABEE_RESPONSE_RADIO_TX_OK) == 0) {
        error_code = MJD_LORABEE_RADIO_TX_OK;
    } else if (strcmp(param_ptr_response, MJD_LORABEE_RESPONSE_RADIO_ERROR) == 0) {
        error_code = MJD_LORABEE_RADIO_ERROR;
    }

    return error_code;
}

/*
 * @brief helpder log config
 *
 */
esp_err_t mjd_lorabee_log_config(mjd_lorabee_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGI(TAG, "Log mjd_lorabee_config_t* param_ptr_config:");
    ESP_LOGI(TAG, "  %32s = %u", "bool is_init", param_ptr_config->is_init);
    ESP_LOGI(TAG, "  %32s = %i", "uart_port_t uart_port_num", param_ptr_config->uart_port_num);
    ESP_LOGI(TAG, "  %32s = %i", "gpio_num_t uart_tx_gpio_num", param_ptr_config->uart_tx_gpio_num);
    ESP_LOGI(TAG, "  %32s = %i", "gpio_num_t uart_rx_gpio_num", param_ptr_config->uart_rx_gpio_num);
    ESP_LOGI(TAG, "  %32s = %i", "gpio_num_t reset_gpio_num", param_ptr_config->reset_gpio_num);

    ESP_LOGI(TAG, "  %32s = %s", "radio_mode", param_ptr_config->radio_mode);
    ESP_LOGI(TAG, "  %32s = %i", "radio_power", param_ptr_config->radio_power);
    ESP_LOGI(TAG, "  %32s = %u", "radio_frequency", param_ptr_config->radio_frequency);
    ESP_LOGI(TAG, "  %32s = %s", "radio_spreading_factor", param_ptr_config->radio_spreading_factor);
    ESP_LOGI(TAG, "  %32s = %u", "radio_bandwidth", param_ptr_config->radio_bandwidth);
    ESP_LOGI(TAG, "  %32s = %u millisec", "radio_watchdog_timeout", param_ptr_config->radio_watchdog_timeout);

    ESP_LOGI(TAG, "  %32s = %u", "uint8_t max_executions_radio_tx", param_ptr_config->max_executions_radio_tx);

    ESP_LOGI(TAG, "  %32s = %u", "uint32_t nbr_of_errors", param_ptr_config->nbr_of_errors);

    return f_retval;
}

/*
 * @brief Execute command and read the RX Response#1 (not #2!)
 *
 */
esp_err_t mjd_lorabee_cmd(mjd_lorabee_config_t* param_ptr_config, const char* param_ptr_command, mjd_lorabee_response_t* param_ptr_response) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    _uart_flush_queue_reset(param_ptr_config);

    // UART Send command
    // @size At least = max command byte length + max Lora TX payload + \r\n
    char command[MJD_LORABEE_TX_COMMAND_MAX_SIZE] = "";
    strcpy(command, param_ptr_command);
    ESP_LOGD(TAG, "command (without CarriageReturn & NewLine: len=%zu val=%s", strlen(command), command);
    strcat(command, "\r\n");

    f_retval = uart_write_bytes(param_ptr_config->uart_port_num, command, strlen(command));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). uart_write_bytes() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        ++param_ptr_config->nbr_of_errors;
        // GOTO
        goto cleanup;
    }

    // Mark ESP_OK (uart_write_bytes() returns ESP_FAIL or #bytes written...)
    f_retval = ESP_OK;

    // RX response#1
    ESP_LOGD(TAG, "RX response#1");
    char *line_uart = "";
    int len_line_uart = 0;

    // ONLY relevant for `sys sleep` which gives no uart output at all (no RX response#1)
    if (mjd_string_starts_with(command, "sys sleep") == true) {
        // GOTO
        goto cleanup;
    }

    line_uart = _get_next_line_uart(param_ptr_config->uart_port_num);
    if (line_uart == NULL) {
        ESP_LOGE(TAG, "    %s(): RX response#1 line_uart == NULL (means ESP_FAIL, goto cleanup)", __FUNCTION__);
        // Mark ERROR
        f_retval = ESP_FAIL;
        ++param_ptr_config->nbr_of_errors;
        // GOTO
        goto cleanup;
    }
    len_line_uart = 1 + strlen(line_uart); // +1 to show the \0 as well
    ESP_LOGD(TAG, "    %s(). HEXDUMP line_uart (len %i)", __FUNCTION__, len_line_uart);
    ESP_LOG_BUFFER_HEXDUMP(TAG, line_uart, len_line_uart, ESP_LOG_DEBUG);

    // Save RX response#1
    param_ptr_response->data_received = true;
    strcpy(param_ptr_response->response_1, line_uart);

    // LABEL
    cleanup: ;

    return f_retval;
}

static esp_err_t _mjd_lorabee_get_key_returning_string_value(mjd_lorabee_config_t* param_ptr_config, char * param_ptr_category, char * param_ptr_key,
                                                             char * param_ptr_result) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    char command[MJD_LORABEE_TX_COMMAND_MAX_SIZE];
    sprintf(command, "%s get %s", param_ptr_category, param_ptr_key);

    mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
    f_retval = mjd_lorabee_cmd(param_ptr_config, command, &response);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    strcpy(param_ptr_result, response.response_1);

    // LABEL
    cleanup: ;

    return f_retval;
}

static esp_err_t _mjd_lorabee_get_key_returning_uint32_value(mjd_lorabee_config_t* param_ptr_config, char * param_ptr_category, char * param_ptr_key,
                                                             uint32_t * param_ptr_result) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    char command[MJD_LORABEE_TX_COMMAND_MAX_SIZE];
    sprintf(command, "%s get %s", param_ptr_category, param_ptr_key);

    mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
    f_retval = mjd_lorabee_cmd(param_ptr_config, command, &response);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // convert string to uint32_t
    // @doc By placing indirection operator (*) before a pointer variable we can access data of the variable whose address is stored in the pointer variable.
    *param_ptr_result = (uint32_t) strtoul(response.response_1, NULL, 10);

    // LABEL
    cleanup: ;

    return f_retval;
}

static esp_err_t _mjd_lorabee_get_key_returning_int32_value(mjd_lorabee_config_t* param_ptr_config, char * param_ptr_category, char * param_ptr_key,
                                                            int32_t * param_ptr_result) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    char command[MJD_LORABEE_TX_COMMAND_MAX_SIZE];
    sprintf(command, "%s get %s", param_ptr_category, param_ptr_key);

    mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
    f_retval = mjd_lorabee_cmd(param_ptr_config, command, &response);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // convert string to int32_t signed value
    // @doc By placing indirection operator (*) before a pointer variable we can access data of the variable whose address is stored in the pointer variable.
    *param_ptr_result = (int32_t) strtol(response.response_1, NULL, 10);

    // LABEL
    cleanup: ;

    return f_retval;
}

static esp_err_t _mjd_lorabee_set_key_with_string_value(mjd_lorabee_config_t* param_ptr_config, char * param_ptr_category, char * param_ptr_key,
                                                        char * param_value) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    char command[MJD_LORABEE_TX_COMMAND_MAX_SIZE];
    sprintf(command, "%s set %s %s", param_ptr_category, param_ptr_key, param_value);

    mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
    f_retval = mjd_lorabee_cmd(param_ptr_config, command, &response);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = _response_text_to_code(response.response_1);
    if (f_retval != MJD_LORABEE_OK) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, mjd_lorabee_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

static esp_err_t _mjd_lorabee_set_key_with_uint32_value(mjd_lorabee_config_t* param_ptr_config, char * param_ptr_category, char * param_ptr_key,
                                                        uint32_t param_value) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    char command[MJD_LORABEE_TX_COMMAND_MAX_SIZE];
    sprintf(command, "%s set %s %u", param_ptr_category, param_ptr_key, param_value);

    mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
    f_retval = mjd_lorabee_cmd(param_ptr_config, command, &response);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = _response_text_to_code(response.response_1);
    if (f_retval != MJD_LORABEE_OK) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, mjd_lorabee_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

static esp_err_t _mjd_lorabee_set_key_with_int32_value(mjd_lorabee_config_t* param_ptr_config, char * param_ptr_category, char * param_ptr_key,
                                                       int32_t param_value) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    char command[MJD_LORABEE_TX_COMMAND_MAX_SIZE];
    sprintf(command, "%s set %s %i", param_ptr_category, param_ptr_key, param_value);

    mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
    f_retval = mjd_lorabee_cmd(param_ptr_config, command, &response);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = _response_text_to_code(response.response_1);
    if (f_retval != MJD_LORABEE_OK) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, mjd_lorabee_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

//
// SYS SET
esp_err_t mjd_lorabee_sys_set_nvm(mjd_lorabee_config_t* param_ptr_config, uint32_t param_hex_address, uint8_t param_value) {
    // @doc <address>: hexadecimal number representing user EEPROM address, from 300 to 3FF
    // @doc <data>: hexadecimal number representing data, from 00 to FF
    // @doc The EEPROM is only erased when doing a factoryRESET (not with `sys reset` or my pin-reset)
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    char command[MJD_LORABEE_TX_COMMAND_MAX_SIZE];
    sprintf(command, "sys set nvm %03X %02X", param_hex_address, param_value);

    mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
    f_retval = mjd_lorabee_cmd(param_ptr_config, command, &response);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = _response_text_to_code(response.response_1);
    if (f_retval != MJD_LORABEE_OK) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, mjd_lorabee_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_sys_set_pindig(mjd_lorabee_config_t* param_ptr_config, mjd_lorabee_gpio_num_t param_gpio_num,
                                     mjd_lorabee_gpio_level_t param_gpio_level) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    char command[MJD_LORABEE_TX_COMMAND_MAX_SIZE];
    sprintf(command, "sys set pindig GPIO%i %i", param_gpio_num, param_gpio_level);

    mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
    f_retval = mjd_lorabee_cmd(param_ptr_config, command, &response);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = _response_text_to_code(response.response_1);
    if (f_retval != MJD_LORABEE_OK) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, mjd_lorabee_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

//
// SYS GET
esp_err_t mjd_lorabee_sys_get_nvm(mjd_lorabee_config_t* param_ptr_config, uint32_t param_address, uint8_t * param_result) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    if (param_address < 0x300 || param_address > 0x3FF) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "%s(). Hex address out of bounds err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // MAIN
    char command[MJD_LORABEE_TX_COMMAND_MAX_SIZE];
    sprintf(command, "sys get nvm %03X", param_address);

    mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
    f_retval = mjd_lorabee_cmd(param_ptr_config, command, &response);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // convert string to uint32_t
    // @doc By placing indirection operator (*) before a pointer variable we can access data of the variable whose address is stored in the pointer variable.
    *param_result = (uint8_t) strtoul(response.response_1, NULL, 16);   // hexstring to byte value

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_sys_get_hweui(mjd_lorabee_config_t* param_ptr_config, char * param_ptr_result) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_get_key_returning_string_value(param_ptr_config, "sys", "hweui", param_ptr_result);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_sys_get_version(mjd_lorabee_config_t* param_ptr_config, mjd_lorabee_version_info_t * param_ptr_result) {
    // @doc %[^characters] Negated scanset. Any number of characters none of them specified as characters between the brackets.
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_get_key_returning_string_value(param_ptr_config, "sys", "ver", param_ptr_result->raw);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Compute derived values
    sscanf(param_ptr_result->raw, "%s %s %[^\n]s", param_ptr_result->model, param_ptr_result->firmware_version, param_ptr_result->firmware_date);

    ESP_LOGD(TAG, "mjd_lorabee_version_info_t * param_ptr_result:");
    ESP_LOGD(TAG, "    param_ptr_result->raw %s", param_ptr_result->raw);
    ESP_LOGD(TAG, "    param_ptr_result->model %s", param_ptr_result->model);
    ESP_LOGD(TAG, "    param_ptr_result->firmware_version %s", param_ptr_result->firmware_version);
    ESP_LOGD(TAG, "    param_ptr_result->firmware_date %s", param_ptr_result->firmware_date);

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_sys_get_vdd(mjd_lorabee_config_t* param_ptr_config, uint32_t * param_ptr_result) {
    // @doc Response: 0–3600 (decimal value from 0 to 3600)
    // @doc This command informs the RN2483 module to do an ADC conversion on the VDD. The measurement is converted and returned as a voltage (mV).
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_get_key_returning_uint32_value(param_ptr_config, "sys", "vdd", param_ptr_result);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

//
// RADIO GET
esp_err_t mjd_lorabee_radio_get_mode(mjd_lorabee_config_t* param_ptr_config, char * param_ptr_result) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_get_key_returning_string_value(param_ptr_config, "radio", "mod", param_ptr_result);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_radio_get_frequency(mjd_lorabee_config_t* param_ptr_config, uint32_t * param_ptr_result) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_get_key_returning_uint32_value(param_ptr_config, "radio", "freq", param_ptr_result);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_radio_get_power(mjd_lorabee_config_t* param_ptr_config, int32_t * param_ptr_result) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_get_key_returning_int32_value(param_ptr_config, "radio", "pwr", param_ptr_result);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_radio_get_spreading_factor(mjd_lorabee_config_t* param_ptr_config, char * param_ptr_result) {
    // TODO Maybe make return param an uint8_t?
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_get_key_returning_string_value(param_ptr_config, "radio", "sf", param_ptr_result);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_radio_get_bandwidth(mjd_lorabee_config_t* param_ptr_config, uint32_t * param_ptr_result) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_get_key_returning_uint32_value(param_ptr_config, "radio", "bw", param_ptr_result);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_radio_get_watchdog_timeout(mjd_lorabee_config_t* param_ptr_config, uint32_t * param_ptr_result) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_get_key_returning_uint32_value(param_ptr_config, "radio", "wdt", param_ptr_result);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_radio_get_signal_noise_ratio(mjd_lorabee_config_t* param_ptr_config, int32_t * param_ptr_result) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_get_key_returning_int32_value(param_ptr_config, "radio", "snr", param_ptr_result);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

//
// RADIO SET
esp_err_t mjd_lorabee_radio_set_mode(mjd_lorabee_config_t* param_ptr_config, char * param_ptr_value) {
    // @doc <mode>: string representing the modulation method, either lora or fsk.
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_set_key_with_string_value(param_ptr_config, "radio", "mod", param_ptr_value);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_radio_set_frequency(mjd_lorabee_config_t* param_ptr_config, uint32_t param_value) {
    // @doc <frequency>: decimal representing the frequency, from 433050000 to 434790000 or from 863000000 to 870000000, in Hz.
    //  @rule EU863-870 ISM Band channel frequencies: 868.10 868.30 868.50
    /*
     * LoTTN RaWAN Frequencies Overview:
     *   This is a list of frequency plan definitions used in The Things Network.
     *   These frequency plans are based on what is specified in the LoRaWAN regional parameters document. See the list of frequency plans by country list.
     *   # EU863-870 Uplink:
     *     868.1 - SF7BW125 to SF12BW125
     *     868.3 - SF7BW125 to SF12BW125 and SF7BW250
     *     868.5 - SF7BW125 to SF12BW125
     *     867.1 - SF7BW125 to SF12BW125
     *     867.3 - SF7BW125 to SF12BW125
     *     867.5 - SF7BW125 to SF12BW125
     *     867.7 - SF7BW125 to SF12BW125
     *     867.9 - SF7BW125 to SF12BW125
     *     868.8 - FSK
     *  # EU863-870 Downlink:
     *      Uplink channels 1-9 (RX1)
     *      869.525 - SF9BW125 (RX2 downlink only)
     *      @doc TTN uses the non-standard SF9BW125 data rate for RX2 in Europe. If your devices use OTAA, that is configured automatically when they join (not so for ABP).
     */
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_set_key_with_uint32_value(param_ptr_config, "radio", "freq", param_value);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_radio_set_power(mjd_lorabee_config_t* param_ptr_config, int32_t param_value) {
    // @doc <pwrOut>: signed decimal number representing the transceiver output power, from -3 to 15
    // @doc -3 works fine for short distances (same room)
    /* @doc <pwrOut>: signed decimal number representing the transceiver output power, from -3 to 15
     * @doc TABLE 2-5: OUTPUT POWER OF TX POWER SETTING for BAND 868Mhz @ Microchip data sheet
     *          TX:     dBm output power    mA supply current
     *          ------- ----------------    -----------------
     *          -3      -4.0                17.3
     *           0      -1.7                20.2
     *           7       5.8                28.8
     *          14      13.5                38
     */

    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_set_key_with_int32_value(param_ptr_config, "radio", "pwr", param_value);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_radio_set_spreading_factor(mjd_lorabee_config_t* param_ptr_config, char * param_ptr_value) {
    // @doc <spreadingFactor>: string representing the spreading factor. Parameter values can be: sf7, sf8, sf9, sf10, sf11 or sf12.
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_set_key_with_string_value(param_ptr_config, "radio", "sf", param_ptr_value);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_radio_set_bandwidth(mjd_lorabee_config_t* param_ptr_config, uint32_t param_value) {
    // @doc <bandWidth>: decimal representing the operating radio bandwidth, in kHz. Parameter values can be: 125, 250, 500.
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_set_key_with_uint32_value(param_ptr_config, "radio", "bw", param_value);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_radio_set_watchdog_timeout(mjd_lorabee_config_t* param_ptr_config, uint32_t param_value) {
    // @doc <watchDog>: decimal number representing the time-out length for the Watchdog Timer, from 0 to 4294967295 milliseconds. Set to ‘0’ to disable this functionality.
    // @default 15000 (=15 seconds)
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    f_retval = _mjd_lorabee_set_key_with_uint32_value(param_ptr_config, "radio", "wdt", param_value);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*
 * @brief RADIO TX
 *
 * @rule EU863-870 Maximum payload size ASCII 115 => HEXSTR 230
 * @dep mjd_lorabee_config_t->max_executions_radio_tx
 * TODO More research FIX TOO LONG DELAY 5SEC AFTER SENDING PACKET and waiting for response (duration varies with quality of Lora network/environment!)
 *
 * @techdoc
 *  Response after entering the command:
 *      # ok            – if parameter is valid and the transceiver is configured in Transmit mode ==> OK, CONTINUE
 *      # invalid_param – if parameter is not valid ==> ERROR
 *      # busy          – if the transceiver is currently busy ==> RETRY (TODO)
 *  Response after the effective transmission:
 *      # radio_tx_ok – if transmission was successful ==> OK
 *      # radio_err   – if transmission was unsuccessful (interrupted by radio Watchdog Timer time-out)  ==> RETRY
 *
 * @doc Also handles `radio tx`: response#1 'busy' as an error
 *
 */
esp_err_t mjd_lorabee_radio_tx(mjd_lorabee_config_t* param_ptr_config, uint8_t* param_ptr_payload, size_t param_len) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    if (param_len * 2 > MJD_LORABEE_LORA_TX_PAYLOAD_MAX_BYTES) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT %s(). Payload length %i exceeds maximum (SF7 %i bytes) | err %i (%s)", __FUNCTION__, param_len * 2,
                MJD_LORABEE_LORA_TX_PAYLOAD_MAX_BYTES, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    char command[MJD_LORABEE_TX_COMMAND_MAX_SIZE] = ""; // @important Also reserve bytes for the command prefix
    f_retval = mjd_uint8s_to_hexstring(param_ptr_payload, param_len, command);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "ABORT %s(). mjd_uint8s_to_hexstring() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        goto cleanup;
    }

    mjd_string_prepend(command, MJD_LORABEE_REQUEST_PREFIX_RADIO_TX);

    uint8_t seq_execution = 0;
    while (true) {
        seq_execution++;
        if (seq_execution > param_ptr_config->max_executions_radio_tx) {
            f_retval = ESP_FAIL;
            ESP_LOGE(TAG, "    %s(). Exceeded max nbr of executions (%u) | err %i (%s)", __FUNCTION__, param_ptr_config->max_executions_radio_tx,
                    f_retval, esp_err_to_name(f_retval));
            ESP_LOGE(TAG, "    %s(): goto cleanup (ABORT)", __FUNCTION__);
            goto cleanup;
        }

        // cmd() RX Response#1
        mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
        f_retval = mjd_lorabee_cmd(param_ptr_config, command, &response);
        if (f_retval == ESP_FAIL) {
            ESP_LOGE(TAG, "    %s(). cmd-retval err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            ESP_LOGE(TAG, "    %s(): continue (try again)", __FUNCTION__);
            continue;
        }

        f_retval = _response_text_to_code(response.response_1);
        if (f_retval == MJD_LORABEE_OK) { // BREAK when OKAY (success)
            ESP_LOGD(TAG, "    %s(). text2code Response OK", __FUNCTION__);

            ESP_LOGD(TAG, "Handle RX response#2");
            char *line_uart = "";
            int len_line_uart = 0;
            line_uart = _get_next_line_uart(param_ptr_config->uart_port_num);
            if (line_uart == NULL) {
                ESP_LOGE(TAG, "    %s(): RX response#2 line_uart == NULL (means ESP_FAIL, goto cleanup)", __FUNCTION__);
                ++param_ptr_config->nbr_of_errors;
                ESP_LOGE(TAG, "    %s(): continue (try again)", __FUNCTION__);
                continue;
            }

            ESP_LOGD(TAG, "Processing RX response#2");
            len_line_uart = 1 + strlen(line_uart); // +1 to show the \0 as well
            ESP_LOGD(TAG, "    %s(). HEXDUMP line_uart (len %i)", __FUNCTION__, len_line_uart);
            ESP_LOG_BUFFER_HEXDUMP(TAG, line_uart, len_line_uart, ESP_LOG_DEBUG);

            // Save response#2
            response.data_received = true;
            strcpy(response.response_2, line_uart);

            f_retval = _response_text_to_code(response.response_2);
            if (f_retval != MJD_LORABEE_RADIO_TX_OK) {
                ESP_LOGE(TAG, "    %s(). err %i (%s)", __FUNCTION__, f_retval, mjd_lorabee_err_to_name(f_retval));
                ++param_ptr_config->nbr_of_errors;
                ESP_LOGE(TAG, "    %s(): continue (try again)", __FUNCTION__);
                continue;
            }

            //
            // BREAK (EXIT LOOP OK)
            break;
            //
        } else {
            ESP_LOGE(TAG, "    %s(). text2code err %i (%s)", __FUNCTION__, f_retval, mjd_lorabee_err_to_name(f_retval));
            ++param_ptr_config->nbr_of_errors;
            ESP_LOGE(TAG, "    %s(): continue (try again)", __FUNCTION__);
            continue;
        }
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*
 * @brief RADIO RX START
 *
 * @dep mjd_lorabee_config_t->max_executions_radio_rx
 *
 * @techdoc
 *  Response after entering the command:
 *      # ok            – if parameter is valid and the transceiver is configured in Transmit mode ==> OK, CONTINUE
 *      # invalid_param – if parameter is not valid ==> ERROR
 *      # busy          – if the transceiver is currently busy ==> RETRY
 *  Response after the effective transmission:
 *      # radio_rx<space><space><data> – if reception was successful, <data>: hexadecimal value that was received ==> OK
 *      # radio_err       – if reception was not successful, reception time-out occurred ==> RETRY
 *
 * @doc This is an endless loop issuing 'radio rx 0' commands and reading a message when it arrives (a different approach than the one shot in the 'radio tx' function).
 *
 */
esp_err_t mjd_lorabee_radio_rx(mjd_lorabee_config_t* param_ptr_config, uint8_t* param_ptr_result, size_t* param_ptr_len) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // TODO Check params

    char command[] = "radio rx 0";

    while (true) {
        // TODO Implement a way to exit this endless loop (example: with a momentary button)
        if (false) {
            f_retval = ESP_FAIL; // @important This ensures that the result (none in this case) is not used by the caller.
            ESP_LOGE(TAG, "    %s(). Request to end the rx loop", __FUNCTION__);
            // GOTO
            goto cleanup;
        }

        // cmd() Response#1
        mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
        f_retval = mjd_lorabee_cmd(param_ptr_config, command, &response);
        if (f_retval == ESP_FAIL) {
            ESP_LOGE(TAG, "    %s(). cmd-retval err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            ESP_LOGE(TAG, "    %s(): continue (try again)", __FUNCTION__);
            // CONTINUE
            continue;
        }

        f_retval = _response_text_to_code(response.response_1);
        if (f_retval == MJD_LORABEE_OK) { // BREAK when OKAY (success)
            ESP_LOGD(TAG, "    %s(). text2code Response OK", __FUNCTION__);

            // Read Response#2
            ESP_LOGD(TAG, "RX response#2");
            char *line_uart = "";
            int len_line_uart = 0;
            line_uart = _get_next_line_uart(param_ptr_config->uart_port_num);
            if (line_uart == NULL) {
                ESP_LOGE(TAG, "    %s(): RX response#2 line_uart == NULL (means ESP_FAIL, goto cleanup)", __FUNCTION__);
                ++param_ptr_config->nbr_of_errors;
                ESP_LOGE(TAG, "    %s(): continue (try again)", __FUNCTION__);
                // CONTINUE
                continue;
            }

            len_line_uart = 1 + strlen(line_uart); // +1 to show the \0 as well
            ESP_LOGD(TAG, "    %s(). HEXDUMP line_uart (len %i)", __FUNCTION__, len_line_uart);
            ESP_LOG_BUFFER_HEXDUMP(TAG, line_uart, len_line_uart, ESP_LOG_DEBUG);

            // Response#2 Save
            response.data_received = true;
            strcpy(response.response_2, line_uart);

            // Response#2 Check
            f_retval = _response_text_to_code(response.response_2);
            if (f_retval == MJD_LORABEE_RADIO_ERROR) {
                ESP_LOGE(TAG, "    %s(). err %i (%s) Drop this response.", __FUNCTION__, f_retval, mjd_lorabee_err_to_name(f_retval));
                ++param_ptr_config->nbr_of_errors;
                ESP_LOGE(TAG, "    %s(): continue (try again)", __FUNCTION__);
                // CONTINUE
                continue;
            }
            if (mjd_string_starts_with(response.response_2, MJD_LORABEE_RESPONSE_PREFIX_RADIO_RX) == false) {
                ESP_LOGE(TAG, "    %s(). response2 does not start with '%s'. Drop this response.", __FUNCTION__, MJD_LORABEE_RESPONSE_PREFIX_RADIO_RX);
                ESP_LOGE(TAG, "    %s().   response2: %s", __FUNCTION__, response.response_2);
                ++param_ptr_config->nbr_of_errors;
                ESP_LOGE(TAG, "    %s(): continue (try again)", __FUNCTION__);
                // CONTINUE
                continue;
            }

            // Response#2 Extract received data (integers). Format: radio_rx<space><space><hexdata>
            // => uint8_t* param_ptr_result, size_t param_len
            //  @special Remove prefix  using memmove() https://stackoverflow.com/questions/4295754/how-to-remove-first-character-from-c-string
            memmove(response.response_2, response.response_2 + strlen(MJD_LORABEE_RESPONSE_PREFIX_RADIO_RX), strlen(response.response_2));
            if (mjd_hexstring_to_uint8s(response.response_2, strlen(response.response_2), param_ptr_result) != ESP_OK) {
                ++param_ptr_config->nbr_of_errors;
                ESP_LOGE(TAG, "    %s(): continue (try again)", __FUNCTION__);
                // CONTINUE
                continue;
            }
            *param_ptr_len = strlen(response.response_2) / 2;

            //
            // BREAK (EXIT LOOP OK)
            break;
            //

        } else {
            ESP_LOGE(TAG, "    %s(). text2code err %i (%s)", __FUNCTION__, f_retval, mjd_lorabee_err_to_name(f_retval));
            ++param_ptr_config->nbr_of_errors;
            ESP_LOGE(TAG, "    %s(): continue (try again)", __FUNCTION__);
            // CONTINUE
            continue;
        }
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

//
// MAC commands
esp_err_t mjd_lorabee_mac_pause(mjd_lorabee_config_t* param_ptr_config) {
    // @doc Disable LoraWAN (and implicitly enable Lora)
    // @doc '0' is returned when the LoRaWAN stack functionality cannot be paused => fail
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
    f_retval = mjd_lorabee_cmd(param_ptr_config, "mac pause", &response);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    if (strcmp(response.response_1, MJD_LORABEE_RESPONSE_CANNOT_MAC_PAUSE) == 0) {
        f_retval = MJD_LORABEE_ERROR_CANNOT_MAC_PAUSE;
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, mjd_lorabee_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_mac_resume(mjd_lorabee_config_t* param_ptr_config) {
    // @doc Enable LoraWAN (and implicitly enable Lora)
    // @doc `mac resume` returns "ok"
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // MAIN
    mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
    f_retval = mjd_lorabee_cmd(param_ptr_config, "mac resume", &response);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = _response_text_to_code(response.response_1);
    if (f_retval != MJD_LORABEE_OK) {
        ESP_LOGE(TAG, "%s(). err %i (%s)", __FUNCTION__, f_retval, mjd_lorabee_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/********************************************************************************
 * LoraBee: RESET COMMAND
 * @brief
 * @important
 * @param mjd_lorabee_config_t
 * @return
 *
 *   @bug It is mandatory to reset the LoraBee device at the start of the app, else the LoraBee does not emit UART responses!
 *   @important The device is available for use again no earlier than 250 millisec after the reset.
 *   @seq AFTER mjd_lorabee_init()
 *
 *   @doc I sacrificed an extra ESP32 pin for this feature. The SJ1 pads on the LoraBee board must be solder-joined.
 *   @logic GPIO Config + Set LOW for 1 millisec (should reset the device!) + Set HIGH again
 *   @pcb RN2483 PIN #32 "RESET" Input Active-low device Reset | LoraBee Xbee Board PIN#17 ("IO7") | Parallax Xbee Adapter Board PIN#IO3
 *
 *   @doc Strict timing is required in this code. Do not put the log() calls between the time-sensitive commands.
 *
 */
esp_err_t mjd_lorabee_reset(mjd_lorabee_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    gpio_config_t reset_io_conf =
        { 0 };

    ESP_LOGD(TAG, "  gpio_config();");
    ESP_LOGD(TAG, "  gpio_set_level(x, 0);");
    ESP_LOGD(TAG, "  delay 1 millisec (arbitrarily)");
    ESP_LOGD(TAG, "  gpio_set_level(x, 1);");
    ESP_LOGD(TAG, "  delay 250 millisec (the device is booting)");

    reset_io_conf.pin_bit_mask = (1ULL << param_ptr_config->reset_gpio_num);
    reset_io_conf.mode = GPIO_MODE_OUTPUT;
    reset_io_conf.pull_up_en = GPIO_PULLUP_ENABLE;    // UART standard = pullup
    reset_io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    reset_io_conf.intr_type = GPIO_INTR_DISABLE;
    f_retval = gpio_config(&reset_io_conf);

    f_retval = gpio_set_level(param_ptr_config->reset_gpio_num, 0);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). gpio_set_level() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    ets_delay_us(1 * 1000); // Pull-down for 1 millisec

    f_retval = gpio_set_level(param_ptr_config->reset_gpio_num, 1);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). gpio_set_level() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Wait to let the device come alive (and generate UART output with the boot message)
    vTaskDelay(RTOS_DELAY_250MILLISEC);

    // Consume the UART output aka flush it (else the next lorabee command will get that output and returns error 'unexpected response')
    //   @data "RN2483 1.0.3 Mar 22 2017 06:00:42"
    char *line_uart = "";
    int len_line_uart = 0;

    line_uart = _get_next_line_uart(param_ptr_config->uart_port_num);
    if (line_uart == NULL) {
        ESP_LOGW(TAG, "    %s: line_uart == NULL (OK continue)", __FUNCTION__);
        ++param_ptr_config->nbr_of_errors;
        // GOTO
        goto cleanup;
    }
    len_line_uart = 1 + strlen(line_uart); // +1 to show the \0 as well
    ESP_LOGD(TAG, "    %s(). HEXDUMP line_uart (len %i)", __FUNCTION__, len_line_uart);
    ESP_LOG_BUFFER_HEXDUMP(TAG, line_uart, len_line_uart, ESP_LOG_DEBUG);

    // LABEL
    cleanup: ;

    return f_retval;
}

/********************************************************************************
 * LoraBee: SLEEP COMMAND
 * @brief 4294967295 = +- 50 days. <length>: decimal number representing the number of milliseconds the system is put to Sleep, from 100 to 4294967296.
 * @important
 * @param mjd_lorabee_config_t
 * @return
 *
 */
esp_err_t mjd_lorabee_sleep(mjd_lorabee_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    mjd_lorabee_response_t response = MJD_LORABEE_RESPONSE_DEFAULT();
    f_retval = mjd_lorabee_cmd(param_ptr_config, "sys sleep 4294967295", &response);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/********************************************************************************
 * LoraBee: WAKEUP COMMAND
 * @brief
 * @important
 * @param mjd_lorabee_config_t
 * @return
 *
 *   @doc The host system needs to transmit to the MicroChip module a BREAK condition followed by one 0x55 character ('U') at the new baud rate
 *   @doc A break condition is signaled to the module by keeping the UART_TX pin low for longer than the time to transmit a complete character (1 char = 10 bits) at that baudrate.
 *   @logic A valid break condition for the default baud rate 57600 bps is keeping the UART_RX pin low for at least 938 microsec. Using 1250 microsec in the code.
 *   @problem Cannot use uart_write_bytes_with_break() because a) Sends the BREAK -after- the data! 2) The data/buf cannot be empty!
 */
esp_err_t mjd_lorabee_wakeup(mjd_lorabee_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // Simulate a BREAK condition: inverse the standard TX signal level from HIGH to LOW for a specific duration
    f_retval = uart_set_line_inverse(param_ptr_config->uart_port_num, UART_INVERSE_TXD);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). uart_set_line_inverse() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ets_delay_us(1250);
    f_retval = uart_set_line_inverse(param_ptr_config->uart_port_num, UART_INVERSE_DISABLE);
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). uart_set_line_inverse() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // TX 'U'
    char autobaud_char = 0x55;
    f_retval = uart_write_bytes(param_ptr_config->uart_port_num, &autobaud_char, sizeof(autobaud_char));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "%s(). uart_write_bytes() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // A standard delay of 100 milliseconds after a standard RN2843 command (except after transmitting/receiving data which takes much much longer to get a device response back!)
    vTaskDelay(RTOS_DELAY_100MILLISEC);

    // Mark OK
    f_retval = ESP_OK;

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    // LABEL
    cleanup: ;

    return f_retval;
}

/**************************************
 * PUBLIC.
 *
 */
esp_err_t mjd_lorabee_init(mjd_lorabee_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // TODO Check all params

    if (param_ptr_config->is_init == true) {
        f_retval = ESP_ERR_INVALID_STATE;
        ESP_LOGE(TAG, "%s(). The component has already been init'd | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // MUTEX (for future use)
    if (!_lorabee_service_semaphore) {
        _lorabee_service_semaphore = xSemaphoreCreateMutex();
        if (!_lorabee_service_semaphore) {
            f_retval = ESP_FAIL;
            ESP_LOGE(TAG, "%s(). xSemaphoreCreateMutex() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
    }

    // Configure the UART1 controller
    uart_config_t uart_config =
        { .baud_rate = MJD_LORABEE_UART_BAUD_SPEED, .data_bits = UART_DATA_8_BITS, .parity = UART_PARITY_DISABLE, .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl =
                UART_HW_FLOWCTRL_DISABLE };
    f_retval = uart_param_config(param_ptr_config->uart_port_num, &uart_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). uart_param_config() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = uart_set_pin(param_ptr_config->uart_port_num, param_ptr_config->uart_tx_gpio_num, param_ptr_config->uart_rx_gpio_num,
    UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). uart_set_pin() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // @doc Param UART TX ring buffer size. OK If set to zero Then the driver will not use the TX buffer; the TX function will block the task until all data has been sent out.
    f_retval = uart_driver_install(param_ptr_config->uart_port_num, MJD_LORABEE_UART_RX_RINGBUFFER_SIZE, 0, MJD_LORABEE_UART_DRIVER_QUEUE_SIZE, &_uart_driver_queue, 0);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). uart_driver_install() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    _uart_flush_queue_reset(param_ptr_config);

    /**
     * RX data queue
     */
    _uart_rx_data_queue = xQueueCreate(MJD_LORABEE_UART_RX_DATA_QUEUE_SIZE, sizeof(uart_event_t));
    if (_uart_rx_data_queue == NULL) {
        ESP_LOGE(TAG, "%s(). xQueueCreate() failed", __FUNCTION__);
        f_retval = ESP_FAIL;
        // goto
        goto cleanup;
    }

    /*
     * RTOS CREATE TASK: monitor UART events (more specifically UART_DATA & ERRORS).
     */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&_uart_events_task, "_uart_events_task (name)", MY_LORABEE_TASK_UART_EVENTS_TASK_STACK_SIZE, NULL,
    RTOS_TASK_PRIORITY_NORMAL, &_uart_events_task_handle, APP_CPU_NUM);
    if (xReturned != pdPASS) {
        ESP_LOGE(TAG, "%s(). xTaskCreatePinnedToCore(_uart_events_task) | err %i (%s)", __FUNCTION__, xReturned, "!=pdPASS");
        f_retval = ESP_FAIL;
        // GOTO
        goto cleanup;
    }

    /********************************************************************************
     * LoraBee Reset
     *     @important It is mandatory to reset the device at the start of the app (else the LoraBee does not give UART responses!)
     */
    f_retval = mjd_lorabee_reset(param_ptr_config);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    /*
     * MICROCHIP SYSTEM: COMMANDS
     * @doc Setup by default for LoRa P2P (not LoraWAN)
     * @doc My Lora P2P mode=lora power=-3 frequency=865.1 Khz spreadfactor=SF7 bandwidth=125Khz watchdogtimeout=20 seconds
     * @important It is required to 'mac pause' before starting Lora P2P comms
     *
     */
    f_retval = mjd_lorabee_mac_pause(param_ptr_config);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    f_retval = mjd_lorabee_radio_set_mode(param_ptr_config, param_ptr_config->radio_mode);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    f_retval = mjd_lorabee_radio_set_power(param_ptr_config, param_ptr_config->radio_power);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    f_retval = mjd_lorabee_radio_set_frequency(param_ptr_config, param_ptr_config->radio_frequency);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    f_retval = mjd_lorabee_radio_set_spreading_factor(param_ptr_config, param_ptr_config->radio_spreading_factor);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    f_retval = mjd_lorabee_radio_set_bandwidth(param_ptr_config, param_ptr_config->radio_bandwidth);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    f_retval = mjd_lorabee_radio_set_watchdog_timeout(param_ptr_config, param_ptr_config->radio_watchdog_timeout);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    // WAKEUP
    f_retval = mjd_lorabee_wakeup(param_ptr_config);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    // Mark init-yes
    param_ptr_config->is_init = true;

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_lorabee_init_lorawan(mjd_lorabee_config_t* param_ptr_config) {
    // TODO Is it really needed to have a specific init() for LoraWAN (versus Lora) in the future?
    return ESP_OK;
}

esp_err_t mjd_lorabee_deinit(mjd_lorabee_config_t* param_ptr_config) {
    // @important Do not erase my static data structure _lorabee_data so the app can still retrieve the last readings.

    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // Lora device Sleep
    mjd_lorabee_sleep(param_ptr_config);

    // Delete task & rx data queue
    _task_delete_using_handle(&_uart_events_task_handle);
    vQueueDelete(_uart_rx_data_queue);

    // DELETE UART driver
    f_retval = uart_driver_delete(param_ptr_config->uart_port_num);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s. uart_driver_delete() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Mark init'd false
    param_ptr_config->is_init = false;

    // LABEL
    cleanup: ;

    return f_retval;
}

