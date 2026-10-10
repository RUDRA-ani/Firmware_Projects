idf_component_register(
    SRCS "burner_timer.c"
    INCLUDE_DIRS "."
    REQUIRES alarm esp_driver_gpio
)