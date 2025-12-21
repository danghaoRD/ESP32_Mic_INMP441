
#include "INMP441.h"
#include <stdio.h>
#include "driver/i2s_std.h"
#include "esp_log.h"
// #include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static const char *TAG = "INMP441";

#define I2S_SAMPLE_RATE   16000
#define I2S_BCK_IO        GPIO_NUM_14
#define I2S_WS_IO         GPIO_NUM_15
#define I2S_DATA_IN_IO    GPIO_NUM_13


static i2s_chan_handle_t rx_handle = NULL;
static void i2s_mic_init(void);
void mic_read_task(void *arg)
{
    int16_t rx_buf[256];
    size_t bytes_read = 0;

    while (1) {
        esp_err_t ret = i2s_channel_read(
            rx_handle,
            rx_buf,
            sizeof(rx_buf),
            &bytes_read,
            portMAX_DELAY
        );

        if (ret == ESP_OK && bytes_read > 0) {
            int samples = bytes_read / sizeof(int16_t);

            /* Debug nhanh: xem biên độ */
            int16_t peak = 0;
            for (int i = 0; i < samples; i++) {
                int16_t v = rx_buf[i];
                if (v < 0) v = -v;
                if (v > peak) peak = v;
            }

            ESP_LOGI(TAG, "samples=%d peak=%d", samples, peak);
        }
    }
}

void INMP441_init(void)
{
    // Initialization code for INMP441 microphone
    i2s_mic_init();
    xTaskCreate(mic_read_task, "mic_read_task", 4096, NULL, 5, NULL);
    ESP_LOGI(TAG, "Microphone init done.");
}


static void i2s_mic_init(void)
{
    /* 1. Create I2S channel */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(
        I2S_NUM_0,
        I2S_ROLE_MASTER
    );

    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_handle));

    /* 2. Standard I2S configuration */
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        // .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(
        //     I2S_DATA_BIT_WIDTH_16BIT,
        //     I2S_SLOT_MODE_MONO
        // ),
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_32BIT, // ⭐ RẤT QUAN TRỌNG
            .slot_mode = I2S_SLOT_MODE_MONO,
            .slot_mask = I2S_STD_SLOT_LEFT,             // ⭐ L/R = GND
            .ws_width = 32,
            .ws_pol = false,
            .bit_shift = true,                          // ⭐ INMP441 cần
        },
        .gpio_cfg = {
            .bclk = I2S_BCK_IO,
            .ws   = I2S_WS_IO,
            .dout = I2S_GPIO_UNUSED,
            .din  = I2S_DATA_IN_IO,
        },
    };

    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));

    /* 3. Enable RX */
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));

    ESP_LOGI(TAG, "I2S mic initialized");
}



