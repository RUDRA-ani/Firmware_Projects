
#ifndef ALARM_H
#define ALARM_H

#include "esp_err.h"
#include <stdbool.h>

esp_err_t alarm_init(void);

void alarm_set(bool active);

bool alarm_is_active(void);

#endif
