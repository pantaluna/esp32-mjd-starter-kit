#include <sys/dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "esp_spiffs.h"

#include "mjd.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED,
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * LOGGING SPIFFS
 *  @doc The partition is not overwritten when writing a new application to flash (it is in another sector of the Flash EEPROM).
 */

// This file will be used for logging
static bool _log_initialized = false;
static FILE* _log_file;
static const char SPIFFS_PARTITION[] = "myspiffs";
static const char SPIFFS_BASE_PATH[] = "/spiffs";
static const char LOG_PATH[] = "/spiffs/log.txt";

// This function will be called by the ESP log library every time ESP_LOG needs to be performed.
//      @CRITICAL Do NOT use the ESP_LOG* macro's in this function ELSE recursive loop and stack overflow! So use here printf() instead for logging.
int my_log_vprintf(const char *fmt, va_list args) {
    static const uint32_t WRITE_CACHE_CYCLE = 5;
    static uint32_t counter = 0;
    int iresult;

    // #1 Write to SPIFFS
    if (_log_file == NULL) {
        printf("ABORT. file handle _log_file is NULL\n");
        return -1;
    }

    iresult = vfprintf(_log_file, fmt, args);
    if (iresult < 0) {
        printf("ABORT. failed vfprintf() err %i\n", iresult);
        return iresult;
    }

    // Smart commit after x writes
    counter++;
    if (counter % WRITE_CACHE_CYCLE == 0) {
        /////printf("fsync'ing log file on SPIFFS (WRITE_CACHE_PERIOD=%u)\n", WRITE_CACHE_CYCLE);
        fsync(fileno(_log_file));
    }

    // ALSO Write to stdout!
    return vprintf(fmt, args);
}

esp_err_t log_format_spiffs() {
    // @seq After esp_vfs_spiffs_register()
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    // Format explicitly
    ESP_LOGI(TAG, "%s", "@doc Do not use esp_spiffs_format() too often else the internal Flash Chip degrades too fast!");
    ESP_LOGI(TAG, "SPIFFS Formatting...");
    if (esp_spiffs_format(SPIFFS_PARTITION) != ESP_OK) {
        ESP_LOGE(TAG, "Format failed");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Format OK");

    return ESP_OK;
}

esp_err_t log_df() { // ~bash df (disk free)
    // @seq After esp_vfs_spiffs_register()
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t ret;
    size_t total_bytes = 0, used_bytes = 0;

    ret = esp_spiffs_info(NULL, &total_bytes, &used_bytes);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information");
        return ret;
    }
    ESP_LOGI(TAG, "Partition size of %s: total: %u, used: %u (%u%%)", SPIFFS_PARTITION, total_bytes, used_bytes, 100 * used_bytes / total_bytes);

    return ESP_OK;
}

int log_df_used_pct() { // ~bash df (disk free)
    // @seq After esp_vfs_spiffs_register()
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t ret;
    size_t total_bytes = 0, used_bytes = 0;

    ret = esp_spiffs_info(NULL, &total_bytes, &used_bytes);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information");
        return -1;
    }

    return 100 * used_bytes / total_bytes;
}

esp_err_t log_erase_log_file() {
    // @seq After esp_vfs_spiffs_register()
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t ret;

    // erase log file
    ret = remove(LOG_PATH);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Unable to delete the file (error %i)", ret);
        return ret;
    }

    return ESP_OK;
}

esp_err_t log_ls() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    char dashes[100 + 1] = "";
    memset(dashes, '-', 100);

    printf("%s %s\n", "ls", SPIFFS_BASE_PATH);
    printf("%10.10s  %20.20s  %50.50s\n", "Type", "Size", "Name");
    printf("%10.10s  %10.20s  %50.50s\n", dashes, dashes, dashes);

    struct stat st;
    DIR *dir;
    dir = opendir(SPIFFS_BASE_PATH);
    if (!dir) {
        printf("Error opendir(%s)", SPIFFS_BASE_PATH);
        return -1;
    }
    char uri[512] = "";
    struct dirent *direntry;
    char snippet[255];
    char line[255];
    while ((direntry = readdir(dir)) != NULL) {
        switch (direntry->d_type) {
        case DT_DIR:  // A directory.
            ESP_LOGI(TAG, "%10s  %20s  %50s\n", "DIR", "", direntry->d_name);
            break;
        case DT_REG:  // A regular file.
            strcpy(line, "");
            sprintf(snippet, "%10s  ", "FILE");
            strcat(line, snippet);
            sprintf(uri, "%s/%s", SPIFFS_BASE_PATH, direntry->d_name);
            if (stat(uri, &st) == 0) {
                sprintf(snippet, "%20ld  ", st.st_size);
            } else {
                sprintf(snippet, "%20s  ", "?");
            }
            strcat(line, snippet);
            sprintf(snippet, "%50.50s", direntry->d_name);
            strcat(line, snippet);
            ESP_LOGI(TAG, "%s", line);
            break;
        case DT_UNKNOWN:  // The type is unknown.
            ESP_LOGI(TAG, "%10sn", "UNKNOWN");
            break;
        default:
            ESP_LOGI(TAG, "%10s", "?????");
        }
    }
    closedir(dir);

    printf("\n");

    return ESP_OK;
}

esp_err_t log_init() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t ret;

    // Register the SPIFFS filesystem.
    //      @important It only formats the partition when it is not yet formatted (or already formatted but with a different partiton layout such as with the tool mkspiffs)
    ESP_LOGI(TAG, "VFS SPIFFS Register...");

    esp_vfs_spiffs_conf_t conf =
        { .base_path = SPIFFS_BASE_PATH, .partition_label = NULL, .max_files = 5, .format_if_mount_failed = true };
    ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "already mounted or partition is encrypted");
        } else if (ret == ESP_ERR_NO_MEM) {
            ESP_LOGE(TAG, "objects could not be allocated");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%d)", ret);
        }
        return ret;
    }

    /////log_format_spiffs();

    // erase log file if %used > X
    ESP_LOGI(TAG, "Checking free space on the SPIFFS partition");
    if (log_df_used_pct() > 2) {
        ESP_LOGI(TAG, "Yes, erasing the logfile because the Flash > 2%% used!");
        log_erase_log_file();
    }

    // Open log file
    _log_file = fopen(LOG_PATH, "at"); // a=append t=textmode
    if (_log_file == NULL) {
        ESP_LOGE(TAG, "Cannot open logfile");
        return ESP_FAIL;
    }

    // Redirect ESP_LOG
    ESP_LOGI(TAG, "\nRedirecting log output to SPIFFS log file (also keep sending logs to UART0)\n");
    esp_log_set_vprintf(&my_log_vprintf);

    // Put BEGIN marker in the logs
    ESP_LOGI(TAG, "##########BEGIN##########");
    mjd_log_time();

    // Mark init'd
    _log_initialized = true;

    // df
    log_df();

    // ls
    log_ls();

    return ESP_OK;
}

esp_err_t log_deinit() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t ret;

    // Put END marker in the logs
    mjd_log_time();
    ESP_LOGI(TAG, "##########END##########");

    // UNDO Redirect ESP_LOG
    ESP_LOGI(TAG, "Redirecting log output back to only UART0 (not to SPIFFS log file anymore)");
    esp_log_set_vprintf(&vprintf);

    // Close log file
    fclose(_log_file);
    _log_file = NULL;

    // df
    log_df();

    // ls
    log_ls();

    // SPIFFS unregister
    ESP_LOGI(TAG, "SPIFFS unregistering");
    ret = esp_vfs_spiffs_unregister(NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. esp_vfs_spiffs_unregister() failed");
        return ret;
    }

    // Mark uninit'd
    _log_initialized = false;

    return ESP_OK;
}

esp_err_t log_dump() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t ret;
    static FILE* f;

    if (_log_initialized == true) {
        ESP_LOGE(TAG, "ABORT. Cannot dump it. the log is not yet de-init'd (closed)");
        return ESP_FAIL;
    }

    // Register the SPIFFS filesystem.
    //      @important It only formats the partition when it is not yet formatted (or already formatted but with a different partiton layout such as with the tool mkspiffs)
    ESP_LOGI(TAG, "VFS SPIFFS Register...");
    esp_vfs_spiffs_conf_t conf =
        { .base_path = SPIFFS_BASE_PATH, .partition_label = NULL, .max_files = 5, .format_if_mount_failed = true };
    ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "already mounted or partition is encrypted");
        } else if (ret == ESP_ERR_NO_MEM) {
            ESP_LOGE(TAG, "objects could not be allocated");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%d)", ret);
        }
        return ret;
    }

    // df
    log_df();

    // ls
    log_ls();

    // Dump data
    // @doc The fgets function reads characters from the stream stream up to, AND including, a newline character.
    ESP_LOGI(TAG, "\n\nDUMPING DATA\n\n...");

    f = fopen(LOG_PATH, "rt"); // r=read t=textmode
    if (f == NULL) {
        ESP_LOGE(TAG, "ABORT. Cannot open logfile %s", LOG_PATH);
        return ESP_FAIL;
    }

    char line[1024];
    uint32_t counter = 0;
    while (fgets(line, sizeof(line), f) != NULL) {
        counter++;
        // strip newline
        char* pos = NULL;
        pos = strchr(line, '\n');
        if (pos != NULL) {
            *pos = '\0';
        }
        ESP_LOGI(TAG, "%8u: '%s'", counter, line);
    }

    fclose(f);

    // SPIFFS unregister
    ESP_LOGI(TAG, "SPIFFS unregistering");
    ret = esp_vfs_spiffs_unregister(NULL);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. esp_vfs_spiffs_unregister() failed");
        return ret;
    }

    return ESP_OK;
}

/*
 * MAIN
 */
void app_spiffs_logging_task(void *pvParameter) {
    // Use POSIX and C standard library functions to work with files.
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    const uint32_t NBR_OF_LINES = 5;

    mjd_log_memory_statistics();

    //log_format_spiffs();

    log_init();

    char dashes[512 + 1] = ""; // >=768: sometimes stack overflow, if so increase task stack heap size...
    memset(dashes, '-', 512);
    uint32_t counter = 1;
    while (counter <= NBR_OF_LINES) {
        ESP_LOGI(TAG, "ESP_LOGI() %u %s", counter, dashes);
        log_df();
        counter++;
    }

    log_deinit();

    log_dump();

    //
    mjd_rtos_wait_forever();
}

void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /**********
     * SOC init
     */
    ESP_LOGI(TAG, "@important nvs_flash_init() = mandatory for Wifi to work later on");
    nvs_flash_init();

    /**********
     * MY STANDARD Init
     */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let sensors become active!)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /**********
     * LED
     */
    ESP_LOGI(TAG, "\n\n***SECTION: LED***");
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_GPIO_NUM:    %i", MY_LED_ON_DEVBOARD_GPIO_NUM);
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_WIRING_TYPE: %i", MY_LED_ON_DEVBOARD_WIRING_TYPE);

    mjd_led_config_t led_config =
        { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    /**********
     * TASK: Logging to SPIFFS
     * @important For stability (Wifi task @ PRO_CPU_NUM): for my task always use xTaskCreatePinnedToCore(APP_CPU_NUM)
     */
    mjd_log_memory_statistics();

    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&app_spiffs_logging_task, "app_spiffs_logging_task (name)",
    MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task app_spiffs_logging_task has been created, and is running right now");
    }

    /**********
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
