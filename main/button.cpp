#include "button.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "my_config.h"
#include "esp_log.h"

static const char *TAG = "BUTTON";
static const int LEDC_RESOLUTION_BITS = 13;
static const int LEDC_MAX_DUTY = (1 << LEDC_RESOLUTION_BITS) - 1;

void button_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Button GPIO config failed: %d", err);
    }

    gpio_config_t io_conf_led = {
        .pin_bit_mask = (1ULL << LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    err = gpio_config(&io_conf_led);
    if (err != ESP_OK) {  
        ESP_LOGE(TAG, "LED GPIO config failed: %d", err);
    }
}

void led_init(void)
{
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = (ledc_timer_bit_t)LEDC_RESOLUTION_BITS,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 5000,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    esp_err_t err = ledc_timer_config(&ledc_timer);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LED timer config failed: %d", err);
    }

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = LED_GPIO,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,
        .hpoint         = 0
    };
    err = ledc_channel_config(&ledc_channel);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LED channel config failed: %d", err);
    }
}

void led_set_brigh_percen(uint8_t percent)
{
    if (percent > 100) {
        percent = 100;
    }
    uint32_t duty = (LEDC_MAX_DUTY * percent) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}    
    