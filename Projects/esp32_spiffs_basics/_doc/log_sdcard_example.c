/* SD card and FAT filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

static const char *TAG = "example";

// This file stream will be used for logging
static FILE* log_file;

// This function will be called by log library every time ESP_LOG needs
// to be performed. This function writes log string to a file.
static int print_to_sd_card(const char *fmt, va_list list)
{
    if (log_file == NULL) {
        return -1;
    }
    int res = vfprintf(log_file, fmt, list);

    // Committing changes to the file on each write is slower,
    // but ensures that no data will be lost.
    //
    // You may have to figure out when to call fsync.
    // For example, only call fsync after every 50 log messages,
    // or after 100ms passed since last fsync, and so on.
    fsync(fileno(log_file));

    return res;
}

// This example can use SDMMC and SPI peripherals to communicate with SD card.
// By default, SDMMC peripheral is used.
// To enable SPI mode, uncomment the following line:

// #define USE_SPI_MODE

// When testing SD and SPI modes, keep in mind that once the card has been
// initialized in SPI mode, it can not be reinitialized in SD mode without
// toggling power to the card.

#ifdef USE_SPI_MODE
// Pin mapping when using SPI mode.
// With this mapping, SD card can be used both in SPI and 1-line SD mode.
// Note that a pull-up on CS line is required in SD mode.
#define PIN_NUM_MISO 2
#define PIN_NUM_MOSI 15
#define PIN_NUM_CLK  14
#define PIN_NUM_CS   13
#endif //USE_SPI_MODE

void app_main(void)
{
    ESP_LOGI(TAG, "Initializing SD card");

#ifndef USE_SPI_MODE
    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // To use 1-line SD mode, uncomment the following line:
    // host.flags = SDMMC_HOST_FLAG_1BIT;

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // GPIOs 15, 2, 4, 12, 13 should have external 10k pull-ups.
    // Internal pull-ups are not sufficient. However, enabling internal pull-ups
    // does make a difference some boards, so we do that here.
    gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
    gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
    gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

#else
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = PIN_NUM_MISO;
    slot_config.gpio_mosi = PIN_NUM_MOSI;
    slot_config.gpio_sck  = PIN_NUM_CLK;
    slot_config.gpio_cs   = PIN_NUM_CS;
    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
#endif //USE_SPI_MODE

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5
    };

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing
    // production applications.
    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%d). "
                "Make sure SD card lines have pull-up resistors in place.", ret);
        }
        return;
    }

    log_file = fopen("/sdcard/log.txt", "a");

    if (log_file == NULL) {
        ESP_LOGE(TAG, "Failed to open file for logging!");
    } else {
        ESP_LOGI(TAG, "Redirecting log output to SD card!");
        esp_log_set_vprintf(&print_to_sd_card);
        printf("Stdout is still sent to UART\n");
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    // Use POSIX and C standard library functions to work with files.
    // First create a file.
    ESP_LOGI(TAG, "Opening file");
    FILE* f = fopen("/sdcard/hello.txt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    // Check if destination file exists before renaming
    struct stat st;
    if (stat("/sdcard/foo.txt", &st) == 0) {
        // Delete it if it exists
        unlink("/sdcard/foo.txt");
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file");
    if (rename("/sdcard/hello.txt", "/sdcard/foo.txt") != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file");
    f = fopen("/sdcard/foo.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);



    printf("Now redirecting stdout to log file as well\n");
    // Save UART stdout stream
    FILE* uart_stdout = stdout;
    // Change stdout for THIS TASK ONLY
    stdout = log_file;
    // Change stdout for all new tasks which will be created
    _GLOBAL_REENT->_stdout = log_file;

    printf("This line will be written into the log file, not to UART\n");

    // Do more stuff
    // ...

    // Reset back to normal
    stdout = uart_stdout;
    _GLOBAL_REENT->_stdout = uart_stdout;
    esp_log_set_vprintf(&vprintf);

    fclose(log_file);
    log_file = NULL;

    printf("Stdout is back to UART\n");
    ESP_LOGI(TAG, "Log is also back to UART");

    // All done, unmount partition and disable SDMMC or SPI peripheral
    esp_vfs_fat_sdmmc_unmount();
    ESP_LOGI(TAG, "Card unmounted");
}
