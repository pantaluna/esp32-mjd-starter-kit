#include "mjd.h"
#include "mjd_list.h"   // Linux Kernel linked list implementation

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
#define MYAPP_RTOS_TASK_STACK_SIZE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * TASK
 */
void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    ESP_LOGI(TAG,
            "\n\nTHE GOOD EXAMPLES HAVE BEEN MOVED TO PRJ MY_MJD_COMPONENTS - THIS PRJ IS FOR testing NEW linked list stuff\n\n");

    /********************************************************************************
     * Reuseable variables
     */

    /********************************************************************************
     * MY STANDARD Init
     */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer, let sensors become active)");
    vTaskDelay(RTOS_DELAY_1MILLISEC);

    /********************************************************************************
     * MAIN: LINKED LIST
     */
    mjd_log_memory_statistics();

    struct fox {
        uint32_t code;
        float weight_kg;
        bool is_loyal;
        struct mjd_list_head list;
    };

    static MJD_LIST_HEAD(fox_list);

    struct fox *one_fox, *next_fox;
    uint32_t mycounter;

    mjd_log_memory_statistics();

    ESP_LOGI(TAG, "ADD 5000 ITEMS (limited FREE HEAP might be only +-200K)");
    ESP_LOGI(TAG, "  sizeof(*one_fox): %u", sizeof(*one_fox));
    for (uint32_t j=0; j < 5000; ++j) {
        //printf("%u ", j); fflush(stdout);
        one_fox = malloc(sizeof(*one_fox));
        one_fox->code = j;
        one_fox->weight_kg = j * 25.25;
        one_fox->is_loyal = false;
        mjd_list_add_tail(&one_fox->list, &fox_list);
    }

    mjd_log_memory_statistics();

    ESP_LOGI(TAG, "mjd_list_empty() 0=no 1=yes");
    ESP_LOGI(TAG, "   %u", mjd_list_empty(&fox_list));

    ESP_LOGI(TAG, "MJD_LIST_COUNT");
    mjd_list_count(&fox_list, &mycounter);
    ESP_LOGI(TAG, "  mycounter: %u", mycounter);

    ESP_LOGI(TAG, "LIST");
    mjd_list_for_each_entry(one_fox, &fox_list, list)
    {
        printf("%u ", one_fox->code); fflush(stdout);
    }

    mjd_log_memory_statistics();


    /********************************************************************************
     * Task Delete
     * @doc Passing NULL will end the current task
     */
    vTaskDelete(NULL);
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /**********
     * TASK:
     * @important For stability (RMT + Wifi): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate()]
     */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE, NULL,
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
