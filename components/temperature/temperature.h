
#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <stdbool.h>
#include "esp_err.h"

esp_err_t temperature_init(void);

esp_err_t temperature_read(
    float *burner1_c,
    float *burner2_c
);

bool temperature_is_valid(int burner);

#endif
