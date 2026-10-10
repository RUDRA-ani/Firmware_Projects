idf_component_register(
    SRCS "keypad.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_gpio freertos
)