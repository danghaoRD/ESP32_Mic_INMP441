#include "uart_driver.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_err.h"

#include "my_config.h"
static const char *TAG = "UART";

void uart_init()
{
  
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    // Configure UART
    uart_param_config(UART_NUM_0, &uart_config);
    
    // Set pins (UART0 default: TXD=GPIO1, RXD=GPIO3)
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, 
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
    // Install driver
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, 1024, 1024, 0, NULL, 0));
}


