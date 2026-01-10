/**
 * @file AI_audio.cpp
 * @author Hao Dang
 * @date 24 Dec 2025
 * @brief Audio processing and AI inference module
 */

#include "AI_audio.h"
#include "INMP441.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2s_std.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_err.h"

#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "model-parameters/model_metadata.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"

#include "button.h"
#include "my_config.h"
#define MAX2(a,b) ((a) > (b) ? (a) : (b))
static const char *TAG = "AI_audio";
typedef struct {
    int16_t i2s_readbuffer[EI_CLASSIFIER_SLICE_SIZE];
    int16_t buffers[2][EI_CLASSIFIER_SLICE_SIZE];
    uint8_t buf_select;
    uint8_t buf_ready;
    uint32_t buf_count;
    uint32_t n_samples;
} audio_interface_t;

static audio_interface_t audio_interface;

static void ai_audio_interface_callback(uint32_t num_samples);
static void audio_capture_task(void *arg);
static void run_classifier_task(void *arg);
static void send_audio_frame(const int16_t *audio_frame, size_t frame_size);
static void send_audio_task(void *arg);

#if 0 // Test with dummy data
    static float dummy_data[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE]= {0};
#endif

static int get_data(size_t offset, size_t length, float *out_ptr)
{
    // memcpy(out_ptr, &dummy_data[offset], length * sizeof(float));
    // return 0;


        // Safety check (rất nên có)
    if ((offset + length) > EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        return -1;
    }

    // memcpy(out_ptr,
    //        dummy_data + offset,
    //        length * sizeof(float));

    return 0;
}

void AI_audio_init(void)
{
    vTaskDelay(pdMS_TO_TICKS(100)); // cho heap + psram ổn định
    
    printf("Free heap: %d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    printf("Free PSRAM: %d\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    xTaskCreate(audio_capture_task, "audio_capture_task", 4096, NULL, 5, NULL);

    #if(EXAMPLE_BUILD == EXAMPLE_RECORD)
        xTaskCreate(send_audio_task, "send_audio_task", 4096, NULL, 5, NULL);
    #elif(EXAMPLE_BUILD == EXAMPLE_AI_CLASSIFIER)
        //run_classifier_init();
        //xTaskCreate(run_classifier_task, "run_classifier_task", 8192, NULL, 6, NULL);
        xTaskCreatePinnedToCore(run_classifier_task, "run_classifier_task", 8192*2, NULL, 24, NULL, 1);
    #endif


    #if 0
    for (int i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) {
       // dummy_data[i] = 0.0f;   // hoặc sin, ramp, random
    }
    ei::signal_t signal;
    signal.total_length = EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
    signal.get_data = get_data;

    static ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR err =  run_classifier(&signal, &result, false);

    printf("run_classifier() err = %d\n", err);
    printf("Free heap: %d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    printf("Free PSRAM: %d\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    if (err != EI_IMPULSE_OK) {
        printf("run_classifier failed (%d)\n", err);
        return;
    }

    for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        printf("%s: %.3f\n",
            result.classification[i].label,
            result.classification[i].value);
    }
    #endif
    // Initialization code for AI audio processing
}

extern uint8_t button_pressed;
static void audio_capture_task(void *arg)
{
    size_t bytes_read = 0;
    // Audio capture and processing loop
    while (1) {
        #if(EXAMPLE_BUILD == EXAMPLE_RECORD)
            if(button_pressed)
            { 
                led_set_brigh_percen(10);
                button_pressed = 0;

                memset(audio_interface.i2s_readbuffer, 0, sizeof(audio_interface.i2s_readbuffer));
                
                esp_err_t ret = inmp441_read_oneTime(audio_interface.i2s_readbuffer, EI_CLASSIFIER_SLICE_SIZE * sizeof(int16_t),
                                            &bytes_read, portMAX_DELAY);
                uint16_t samples_read = bytes_read / sizeof(int16_t);
                if(ret == ESP_OK && samples_read > 0) 
                {
                    ai_audio_interface_callback(samples_read);
                }

                led_set_brigh_percen(0);

            }
        #elif(EXAMPLE_BUILD == EXAMPLE_AI_CLASSIFIER)
            memset(audio_interface.i2s_readbuffer, 0, sizeof(audio_interface.i2s_readbuffer));
            
            esp_err_t ret = inmp441_read(audio_interface.i2s_readbuffer, EI_CLASSIFIER_SLICE_SIZE * sizeof(int16_t),
                                        &bytes_read, portMAX_DELAY);
            
            uint16_t samples_read = bytes_read / sizeof(int16_t);

            if(ret == ESP_OK && samples_read > 0) 
            {
                ai_audio_interface_callback(samples_read);
            }
    #endif
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void ai_audio_interface_callback(uint32_t num_samples)
{
    int16_t peak = 0;
    int16_t v = 0;
    for (uint32_t i = 0; i < num_samples; i++)
    {
        int32_t s_gain = audio_interface.i2s_readbuffer[i] * 10L;
        if(s_gain > INT16_MAX) s_gain = INT16_MAX;
        if(s_gain < INT16_MIN) s_gain = INT16_MIN;
        audio_interface.buffers[audio_interface.buf_select][audio_interface.n_samples++] = (int16_t)s_gain;

        if(audio_interface.n_samples >= EI_CLASSIFIER_SLICE_SIZE)
        {
            ESP_LOGI(TAG, "data ready");
            audio_interface.buf_select ^= 1; // Switch buffer
            audio_interface.n_samples = 0;
            audio_interface.buf_ready = true;
        }

         v = audio_interface.buffers[audio_interface.buf_select][audio_interface.n_samples - 1];
        if (v < 0) v = -v;
        if (v > peak) peak = v;

    }
    ESP_LOGI(TAG, "samples=%d peak=%d", num_samples, peak);
}
   
/**
 * Get raw audio signal data
 */
int ei_microphone_inference_get_data(size_t offset, size_t length, float *out_ptr)
{
    return ei::numpy::int16_to_float(&audio_interface.buffers[audio_interface.buf_select ^ 1][offset], out_ptr, length);
}

static void run_classifier_task(void *arg)
{
    run_classifier_init();
    static uint8_t print_result = 0;
    static uint8_t led_state = 0;
    while (1)
    {
        if(audio_interface.buf_ready)
        {
           printf("Classifying audio... \n");
            audio_interface.buf_ready = false;
            signal_t signal;
            signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
            signal.get_data = ei_microphone_inference_get_data;

            ei_impulse_result_t result = { 0 };
            size_t start_time = ei_read_timer_us();
            EI_IMPULSE_ERROR err = run_classifier_continuous(&signal, &result, false);
            size_t end_time = ei_read_timer_us();
            printf("Inference time: %d us\n", (int)(end_time - start_time));
            if (err != EI_IMPULSE_OK) {
                printf("run_classifier failed (%d)\n", err);
                continue;
            }
            if(result.classification[0].value > 0.3f)
            {
                display_results(&ei_default_impulse, &result);
                if(result.classification[0].value > 0.5f)
                {
                    ESP_LOGI(TAG, "LED changed");
                    led_state ^= 1;
                }

                if(led_state) led_set_brigh_percen(10);
                else          led_set_brigh_percen(0);
                    
            }

            // if(++print_result >= (EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW/1))
            // {
            //     print_result = 0;
            //     display_results(&ei_default_impulse, &result);
            // }

        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void send_audio_task(void *arg)
{
    while(1)
    {
        if(audio_interface.buf_ready)
        {
            audio_interface.buf_ready = false;
            send_audio_frame(
                audio_interface.buffers[audio_interface.buf_select ^ 1],
                EI_CLASSIFIER_SLICE_SIZE
            );
            // send audio_interface.buffers[audio_interface.buf_select ^ 1] via bluetooth or wifi
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void send_audio_frame(const int16_t *audio_frame, size_t frame_size)
{
    uint32_t size = frame_size * sizeof(int16_t);

    // Header
    uart_write_bytes(UART_NUM_0, "AUDIO", 5);
    uart_write_bytes(UART_NUM_0, (const char*)&size, 4);
    uart_write_bytes(UART_NUM_0, (const char*)audio_frame, size);
    

}