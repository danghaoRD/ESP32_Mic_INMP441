cmake_minimum_required(VERSION 3.13.1)

set(MODEL_FOLDER ..)
set(EI_SDK_FOLDER ../edge-impulse-sdk)


if(NOT CMAKE_BUILD_EARLY_EXPANSION)
add_definitions(-DEI_CLASSIFIER_TFLITE_ENABLE_ESP_NN=1) # enables ESP-NN optimizations by Espressif
add_definitions(-DEIDSP_USE_ESP_DSP=1) # enables ESP-DSP optimizations by Espressif
endif()

set(include_dirs
    ${MODEL_FOLDER}
    ${MODEL_FOLDER}/tflite-model
    ${MODEL_FOLDER}/model-parameters
    ${EI_SDK_FOLDER}
    ${EI_SDK_FOLDER}/classifier
    ${EI_SDK_FOLDER}/dsp
    ${EI_SDK_FOLDER}/porting
    ${EI_SDK_FOLDER}/porting/espressif/ESP-NN
    ${EI_SDK_FOLDER}/porting/espressif/ESP-NN/include
    ${EI_SDK_FOLDER}//porting/espressif/esp-dsp/modules/fft/include
    ${EI_SDK_FOLDER}//porting/espressif/esp-dsp/modules/common/include
    ${EI_SDK_FOLDER}/tensorflow
    ${EI_SDK_FOLDER}/third_party
    ${EI_SDK_FOLDER}/third_party/ruy
    ${EI_SDK_FOLDER}/third_party/gemmlowp
    ${EI_SDK_FOLDER}/third_party/flatbuffers/include
)


include(${EI_SDK_FOLDER}/cmake/utils.cmake)

RECURSIVE_FIND_FILE_EXCLUDE_DIR(SOURCE_FILES "../edge-impulse-sdk" "CMSIS" "*.cpp"
                                                "*.c"
                                                "*.cpp")
RECURSIVE_FIND_FILE_EXCLUDE_DIR(CC_FILES "../edge-impulse-sdk" "CMSIS" "*.cc")
RECURSIVE_FIND_FILE_EXCLUDE_DIR(C_FILES "../edge-impulse-sdk" "CMSIS" "*.c")

# Manually handle needed C only files b/c of build errors
list(APPEND C_FILES "../edge-impulse-sdk/CMSIS/DSP/Source/StatisticsFunctions/arm_rms_f32.c")

RECURSIVE_FIND_FILE(PORTING_FILES "../edge-impulse-sdk/porting/espressif" "*.cpp")

RECURSIVE_FIND_FILE(MODEL_FILES "../tflite-model" "*.cpp")


RECURSIVE_FIND_FILE_EXCLUDE_DIR(S_FILES_ESP_NN "${EI_SDK_FOLDER}/porting/espressif/ESP-NN" "CMSIS" "*.s")
RECURSIVE_FIND_FILE_EXCLUDE_DIR(S_FILES_ESP_DSP "${EI_SDK_FOLDER}/porting/espressif/esp-dsp/modules/fft" "CMSIS" "*.s")

list(APPEND SOURCE_FILES ${C_FILES})
list(APPEND SOURCE_FILES ${CC_FILES})

list(APPEND SOURCE_FILES ${PORTING_FILES})

list(APPEND SOURCE_FILES ${FIRMWARE_SDK_FILES})
list(APPEND SOURCE_FILES ${FIRMWARE_SDK_C_FILES})

list(APPEND SOURCE_FILES ${MODEL_FILES})

list(APPEND SOURCE_FILES ${PLATFORM_FILES})
list(APPEND SOURCE_FILES ${SENSORS_FILES})
list(APPEND SOURCE_FILES ${INGESTION_FILES})

list(APPEND SOURCE_FILES ${INFERENCE_FILES})
list(APPEND SOURCE_FILES ${MBEDTLS_FILES})

list(APPEND SOURCE_FILES ${S_FILES_ESP_NN})
list(APPEND SOURCE_FILES ${S_FILES_ESP_DSP})


# file(GLOB SOURCE_FILES
#     "*.c"
#     "*.cpp"
# )

idf_component_register( SRCS "esp32_mic.cpp"
                             "AI_audio.cpp"
                             "INMP441.cpp"
                            "${SOURCE_FILES}"
                       INCLUDE_DIRS "${include_dirs}" 
                                    "."
    PRIV_REQUIRES spi_flash nvs_flash
    PRIV_REQUIRES nvs_flash esp_psram
    PRIV_REQUIRES esp_http_server
    PRIV_REQUIRES esp_timer
    PRIV_REQUIRES esp_netif 
    PRIV_REQUIRES esp_wifi
    PRIV_REQUIRES esp_event
    PRIV_REQUIRES driver
    #PRIV_REQUIRES edge-impulse-sdk
    )

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++17)

# file(GLOB SOURCE_FILES
#     "*.c"
#     "*.cpp"
# )

# idf_component_register(SRCS "esp32_mic.cpp" "${SOURCE_FILES}"
#                     INCLUDE_DIRS ".")
