/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "esp_log.h"
#include "driver/i2s_std.h"
#include <string.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "INMP441.h"
#include "AI_audio.h"

#include "my_config.h"
#include "button.h"
#include "uart_driver.h"

static void mcu_intro(void);

uint8_t button_pressed = 0;
static int prev_button_state = 1; // assuming pull-up, not pressed
extern "C" int app_main(void)
{
    esp_log_level_set("*", ESP_LOG_NONE);
    mcu_intro();

    inmp441_init();
    AI_audio_init();

    button_init();
    uart_init();



    while (1)
    {
        int curr_button_state = gpio_get_level(BUTTON_GPIO);
      //  ESP_LOGI("TAG", "Button pressed first %d", curr_button_state);
        if(prev_button_state != curr_button_state)
        {
            if(curr_button_state == 1)
            {
                button_pressed = 1;

                ESP_LOGI("TAG", "Button pressed! %d", curr_button_state);

            }
        }
        prev_button_state = curr_button_state;
        //ESP_LOGI("TAG", "Running...");
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    



    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}


static void mcu_intro(void)
{
        printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    vTaskDelay(pdMS_TO_TICKS(1000));

    return;
}

