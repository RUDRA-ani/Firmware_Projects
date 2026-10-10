#ifndef KEYPAD_H
#define KEYPAD_H

#include "esp_err.h"

esp_err_t keypad_init(void);

/*
 * Returns a key once per press.
 * Returns '\0' when there is no new key press.
 */
char keypad_get_key(void);

#endif