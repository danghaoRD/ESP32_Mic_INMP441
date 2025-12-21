file(GLOB SOURCE_FILES
    "*.c"
    "*.cpp"
)

idf_component_register(SRCS "esp32_mic.cpp" "${SOURCE_FILES}"
                    INCLUDE_DIRS ".")
