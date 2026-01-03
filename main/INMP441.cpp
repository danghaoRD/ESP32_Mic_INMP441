
#include "INMP441.h"
#include <stdio.h>
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "esp_log.h"
// #include <string.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static const char *TAG = "INMP441";

#define I2S_SAMPLE_RATE   8000
#define I2S_BCK_IO        GPIO_NUM_14
#define I2S_WS_IO         GPIO_NUM_15
#define I2S_DATA_IN_IO    GPIO_NUM_13


i2s_chan_handle_t rx_handle = NULL;

void inmp441_init(void)
{
    // Initialization code for INMP441 microphone
    /* 1. Create I2S channel */
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(
        I2S_NUM_0,
        I2S_ROLE_MASTER
    );

    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_handle));

    /* 2. Standard I2S configuration */
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(I2S_SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO), // ⭐ INMP441 is mono AND USE PHILIPS FORMAT        
        .gpio_cfg = {
            .bclk = I2S_BCK_IO,
            .ws   = I2S_WS_IO,
            .dout = I2S_GPIO_UNUSED,
            .din  = I2S_DATA_IN_IO,
            .invert_flags = {
                .mclk_inv = 0,
                .bclk_inv = 0,
                .ws_inv   = 0,
            },
        },
    };
    
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_handle, &std_cfg));

    /* 3. Enable RX */
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle));

    ESP_LOGI(TAG, "I2S mic initialized");
}


void inmp441_deinit(void)
{
    ESP_ERROR_CHECK(i2s_channel_disable(rx_handle));
    ESP_ERROR_CHECK(i2s_del_channel(rx_handle));
    ESP_LOGI(TAG, "I2S mic deinitialized");
}

/**
 * @func    inm441_read
 * @brief   wait and copy data from i2s ringbuff to dest
 * @param   int16_t *dest: destination buffer pointer
 * @param   size_t len: length of data to read in bytes
 * @param   size_t *bytes_read: actual bytes read
 * @param   uint32_t timeout_ms: timeout of reading in milliseconds
 * @reval   -ret reading status
 */
esp_err_t inmp441_read(int16_t *dest, size_t len, size_t *bytes_read, uint32_t timeout_ms)
{
    esp_err_t ret =  i2s_channel_read(
        rx_handle,
        dest,
        len,
        bytes_read,
        timeout_ms
    );

    return ret;
}

