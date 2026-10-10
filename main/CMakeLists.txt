idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES temperature keypad burner_timer alarm
)