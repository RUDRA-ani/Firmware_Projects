#ifndef BURNER_TIMER_H
#define BURNER_TIMER_H

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

esp_err_t burner_timer_init(void);

esp_err_t burner_timer_start(
    int burner,
    uint32_t duration_seconds
);

void burner_timer_cancel(int burner);
bool burner_timer_is_active(int burner);
uint32_t burner_timer_remaining(int burner);

void burner_timer_update(void);
void burner_timer_acknowledge_alarm(void);
bool burner_timer_alarm_is_active(void);

#endif