#include "button.h"
#include "driver/gpio.h"
#include "my_config.h"
#include "esp_log.h"

static const char *TAG = "BUTTON";

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
