/*
 * HARDWARE SETUP the MJD components:
 *  *NONE
 *
 */

/*
 * Includes: system, own
 */
#include "mjd.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_8K (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * Config: local
 *      PName NName: max 15c @values nvsbasics nvs
 *      Deep Sleep:  5 15*60 30*60
 */
static const char MY_NVS_PARTITION_DEFAULT[] = "nvs";
static const char MY_NVS_PARTITION_CUSTOM[] = "mynvsbasics";
static const char MY_NVS_NAMESPACE[] = "uploadqueue";
static const uint32_t MY_DEEP_SLEEP_TIME_SEC = 5;
static const uint32_t MY_NVS_PURGE_FREQUENCY = 5;

/*
 * HELPERS
 */
void do_partition_nvs(const char * param_ptr_partition_name, const char * param_ptr_namespace_name) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /********************************************************************************
     * Reuseable variables
     *
     */
    esp_err_t f_retval = ESP_OK;

    /********************************************************************************
     * NVS
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: NVS***");

    nvs_handle my_nvs_handle;
    char nvsKey[16] = "";
    nvs_stats_t nvs_stats;

    //
    ESP_LOGI(TAG, "exec nvs_flash_init_partition()");
    f_retval = nvs_flash_init_partition(param_ptr_partition_name);
    if (f_retval == ESP_ERR_NVS_NO_FREE_PAGES || f_retval == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "  nvs_flash_init_partition() err %i (%s)", f_retval, esp_err_to_name(f_retval));

        ESP_LOGI(TAG, "exec nvs_flash_erase_partition()");
        f_retval = nvs_flash_erase_partition(param_ptr_partition_name);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "  nvs_flash_erase_partition() err %i (%s)", f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup_nvs;
        }

        ESP_LOGI(TAG, "exec nvs_flash_init_partition()");
        f_retval = nvs_flash_init_partition(param_ptr_partition_name);
    }
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "  nvs_flash_init_partition() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup_nvs;
    }

    //
    ESP_LOGI(TAG, "exec nvs_open_from_partition()");
    f_retval = nvs_open_from_partition(param_ptr_partition_name, param_ptr_namespace_name, NVS_READWRITE, &my_nvs_handle);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "  nvs_open_from_partition() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup_nvs;
    }

    // STATS
    ESP_LOGI(TAG, "exec nvs_get_stats()");
    f_retval = nvs_get_stats(param_ptr_partition_name, &nvs_stats);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "  nvs_get_stats() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup_nvs;
    }
    ESP_LOGI(TAG, "  STATS %s :: UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d)\n", param_ptr_partition_name, nvs_stats.used_entries,
            nvs_stats.free_entries, nvs_stats.total_entries);

    mjd_log_memory_statistics();

    // Get boot-count
    uint32_t counter = mjd_get_mcu_boot_count(); // Func uses RTC variable (persistent after Deep Sleep)

    // COMPUTE nvsKey (MODULO!)
    uint32_t num_key_suffix = 0;
    uint32_t modulo = counter % MY_NVS_PURGE_FREQUENCY;
    if (modulo != 0) {
        num_key_suffix = modulo;
    } else {
        num_key_suffix = MY_NVS_PURGE_FREQUENCY;
    }
    sprintf(nvsKey, "cntr%05u", num_key_suffix);
    ESP_LOGI(TAG, "nvsKey = %s", nvsKey);

    // Write & Read
    ESP_LOGI(TAG, "exec nvs_set_u32()");
    f_retval = nvs_set_u32(my_nvs_handle, nvsKey, counter);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "  nvs_set_u32() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup_nvs;
    }

    ESP_LOGI(TAG, "exec nvs_get_u32()");
    f_retval = nvs_get_u32(my_nvs_handle, nvsKey, &counter);
    if (f_retval == ESP_OK) {
        ESP_LOGI(TAG, "NVS key: %s | val: %i", nvsKey, counter);
    } else if (f_retval == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(TAG, "  nvs_get_u32() WARNING err %i (%s)", f_retval, esp_err_to_name(f_retval));
    } else {
        ESP_LOGE(TAG, "  nvs_get_u32() ERROR err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup_nvs;
    }

    // STATS
    ESP_LOGI(TAG, "exec nvs_get_stats()");
    f_retval = nvs_get_stats(param_ptr_partition_name, &nvs_stats);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "  nvs_get_stats() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup_nvs;
    }
    ESP_LOGI(TAG, "  STATS %s :: UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d)\n", param_ptr_partition_name, nvs_stats.used_entries,
            nvs_stats.free_entries, nvs_stats.total_entries);

    // DUMP DATA every MY_NVS_PURGE_FREQUENCY & CLEANUP:
    if (counter % MY_NVS_PURGE_FREQUENCY == 0) {
        ESP_LOGI(TAG, "\nDUMP DATA every MY_NVS_PURGE_FREQUENCY & CLEANUP | bootcount %u MODULO %u is 0 => erase all keys in namespace %s\n", counter,
                MY_NVS_PURGE_FREQUENCY, param_ptr_namespace_name);

        for (uint32_t i = 1; i <= MY_NVS_PURGE_FREQUENCY; i++) {
            // COMPUTE nvsKey
            sprintf(nvsKey, "cntr%05u", i);

            ESP_LOGI(TAG, "exec nvs_get_u32() nvsKey = %s", nvsKey);
            f_retval = nvs_get_u32(my_nvs_handle, nvsKey, &counter);
            if (f_retval == ESP_ERR_NVS_NOT_FOUND) {
                ESP_LOGI(TAG, "  BREAK LOOP <== nvs_get_u32() WARNING err %i (%s)", f_retval, esp_err_to_name(f_retval));
                // BREAK
                break;
            } else if (f_retval != ESP_OK) {
                ESP_LOGE(TAG, "  nvs_get_u32() ERROR err %i (%s)", f_retval, esp_err_to_name(f_retval));
                // GOTO
                goto cleanup_nvs;
            }
            ESP_LOGI(TAG, "NVS key: %s | val: %i", nvsKey, counter);
        }

        ESP_LOGI(TAG, "exec nvs_erase_all()");
        f_retval = nvs_erase_all(my_nvs_handle);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "  nvs_erase_all() err %i (%s)", f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup_nvs;
        }

        ESP_LOGI(TAG, "\n\n");
    }

    // STATS
    ESP_LOGI(TAG, "exec nvs_get_stats()");
    f_retval = nvs_get_stats(param_ptr_partition_name, &nvs_stats);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "  nvs_get_stats() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup_nvs;
    }
    ESP_LOGI(TAG, "  STATS %s :: UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d)\n", param_ptr_partition_name, nvs_stats.used_entries,
            nvs_stats.free_entries, nvs_stats.total_entries);

    // Commit
    ESP_LOGI(TAG, "exec nvs_commit()");
    f_retval = nvs_commit(my_nvs_handle);
    if (f_retval != ESP_OK) {
        ESP_LOGI(TAG, "  nvs_commit() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup_nvs;
    }

    // LABEL
    cleanup_nvs: ;

    // Close
    nvs_close(my_nvs_handle); // void

}

/*
 * TASK
 */

void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /********************************************************************************
     * NVS
     *
     */
    /////do_partition_nvs(MY_NVS_PARTITION_DEFAULT, MY_NVS_NAMESPACE);
    do_partition_nvs(MY_NVS_PARTITION_CUSTOM, MY_NVS_NAMESPACE);

    /********************************************************************************
     * DEEP SLEEP & RESTART TIMER
     * @sop Put this section in comments when testing other things afterwards (else the MCU restarts every time...)
     * @important In deep sleep mode, wireless peripherals are powered down. Before entering sleep mode, applications must disable WiFi and BT using appropriate calls ( esp_bluedroid_disable(), esp_bt_controller_disable(), esp_wifi_stop()).
     * @doc https://esp-idf.readthedocs.io/en/latest/api-reference/system/sleep_modes.html
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: DEEP SLEEP***");

    mjd_log_memory_statistics();

    esp_sleep_enable_timer_wakeup(mjd_seconds_to_microseconds(MY_DEEP_SLEEP_TIME_SEC));

    // @important Wait a bit so that the ESP_LOGI() can really dump to UART before the deep sleep kicks in!
    ESP_LOGI(TAG, "Entering deep sleep (the MCU should wake up %u seconds later)...\n\n", MY_DEEP_SLEEP_TIME_SEC);
    vTaskDelay(RTOS_DELAY_1SEC);
    esp_deep_sleep_start();

    /********************************************************************************
     * OK: NEVER GETS HERE
     *
     */
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /********************************************************************************
     * MY STANDARD Init
     *
     */
    mjd_log_memory_statistics();

    mjd_log_wakeup_details();
    mjd_increment_mcu_boot_count();
    mjd_log_mcu_boot_count();
    mjd_log_time();

    ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer, let sensors become active, ...)");
    vTaskDelay(RTOS_DELAY_10MILLISEC);

    /**********
     * TASK:
     * @important For stability (RMT + Wifi): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate()]
     */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_8K, NULL,
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
