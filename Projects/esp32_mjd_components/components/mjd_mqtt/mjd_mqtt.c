/*
 * Component: MQTT
 *
 * - HARDWARE SETUP the MJD components:
 *  *NONE
 *
 * - TODO Apr7,2018+: bugfix for esp-mqtt (cannot do more than 1 cycle [start publish stop] else critical error "LWIP yield -4".
 *
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_mqtt.h"

/**********
 * Logging
 */
static const char TAG[] = "mjd_mqtt";

/*
 * MAIN
 */
static bool is_mqtt_started = false;

static uint32_t total_nbr_of_fatal_mqtt_publish_errors = 0;
static uint32_t total_nbr_of_mqtt_publish_errors = 0;

/**********
 * Callback and Event Group Handler
 * @doc Use IRAM_ATTR to reduce the penalty associated with loading the code from flash. Cases when parts of application should or may be placed into IRAM:
 *          - Interrupt handlers (SHOULD).
 *          - Some timing critical code (MAY).
 */
static EventGroupHandle_t mqtt_event_group;
static const int MQTT_CONNECTED_BIT = BIT0;

static IRAM_ATTR void mqtt_status_callback(esp_mqtt_status_t status) {
    switch (status) {
    case ESP_MQTT_STATUS_CONNECTED:
        xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
        break;
    case ESP_MQTT_STATUS_DISCONNECTED: // @important This bitflag is not set when stopping mqtt (only when an active netconn is ABORTED)...
        xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
        break;
    }
}

static IRAM_ATTR void mqtt_message_callback(const char *topic, uint8_t *payload, size_t len) {
    ESP_LOGI(TAG, "(not used yet) subscr msg rcvd: topic=%s => payload=%s (%d)", topic, payload, (int ) len);
}

/**********
 * PUBLIC IFACE
 */
esp_err_t mjd_mqtt_init(size_t buffer_size, int command_timeout) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    mqtt_event_group = xEventGroupCreate();
    esp_mqtt_init(mqtt_status_callback, mqtt_message_callback, buffer_size, command_timeout);

    return f_retval;
}

esp_err_t mjd_mqtt_start(const char *host, const char *port, const char *client_id, const char *username, const char *password) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // @important Can only start ONCE (limitation of the lib esp_mqtt)
    if (is_mqtt_started == true) {
        ESP_LOGW(TAG, "mjd_mqtt_start() already started. Ignore this start request.");
        // GOTO
        goto cleanup;
    }

    EventBits_t uxBits;
    esp_mqtt_start(host, port, client_id, username, password);
    uxBits = xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT, pdFALSE, pdTRUE, RTOS_DELAY_5SEC);
    if ((uxBits & MQTT_CONNECTED_BIT) == 0) {
        ESP_LOGE(TAG, "ABORT. esp_mqtt_start() failed");
        is_mqtt_started = false;
        // EXIT
        f_retval = ESP_FAIL;
    }

    is_mqtt_started = true;

    // LABEL
    cleanup:;

    return f_retval;
}

esp_err_t mjd_mqtt_stop() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // @important Avoid double stops
    if (is_mqtt_started == false) {
        ESP_LOGW(TAG, "mjd_mqtt_stop() already stopped. Ignore this stop request.");
        // GOTO
        goto cleanup;
    }

    esp_mqtt_stop();
    xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT); // @important You have to do this MANUALLY!

    is_mqtt_started = false;

    ESP_LOGI(TAG, "  @stats total_nbr_of_mqtt_publish_errors (retried): %u", total_nbr_of_mqtt_publish_errors);
    ESP_LOGI(TAG, "  @stats total_nbr_of_fatal_mqtt_publish_errors:     %u", total_nbr_of_fatal_mqtt_publish_errors);

    // LABEL
    cleanup:;

    return f_retval;
}

esp_err_t mjd_mqtt_publish(const char *topic, uint8_t *payload, size_t len, int qos, bool retained) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    uint32_t mqtt_publish_attempt_nr = 0;

    while (++mqtt_publish_attempt_nr <= MJD_MQTT_MAX_PUBLISH_ATTEMPTS) {
        // MQTT Publish.
        if (MJD_MQTT_LOG_MQTT_PUBLISH == true) {
            ESP_LOGI(TAG, "mjd_mqtt_publish(): topic=%s => payload=%s", topic, payload);
        }
        if (esp_mqtt_publish(topic, payload, len, qos, retained) == true) {
            // @important Delay MINIMUM 10MILLISEC => 1. Avoid LWIP Error -4 (timeout); 2. Avoid ESP-IDF watchdog triggered when publishing a lot of MQTT messages.
            vTaskDelay(RTOS_DELAY_10MILLISEC);
            // BREAK (OK)
            break;
        }
        mjd_log_memory_statistics();

        ESP_LOGE(TAG, "mjd_mqtt_publish(): FAILED. retrying...");
        ++total_nbr_of_mqtt_publish_errors;

        // @important Delay MINIMUM 5SEC (hoping the network error condition is resolved by then. Try to fix LWIP network errors.)
        vTaskDelay(RTOS_DELAY_5SEC);
    }
    if (mqtt_publish_attempt_nr > MJD_MQTT_MAX_PUBLISH_ATTEMPTS) {
        ++total_nbr_of_fatal_mqtt_publish_errors;
        ESP_LOGE(TAG, "MQTT: esp_mqtt_publish() failed after max %u attempts! Aborting...", MJD_MQTT_MAX_PUBLISH_ATTEMPTS);
        f_retval = ESP_FAIL;
    }

    /////mjd_log_memory_statistics();

    return f_retval;
}
