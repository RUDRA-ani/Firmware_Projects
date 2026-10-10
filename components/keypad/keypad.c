#include "keypad.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define ROWS 4
#define COLS 4
#define DEBOUNCE_MS 30

static const gpio_num_t row_pins[ROWS] = {
    GPIO_NUM_23,
    GPIO_NUM_22,
    GPIO_NUM_21,
    GPIO_NUM_19
};

static const gpio_num_t col_pins[COLS] = {
    GPIO_NUM_18,
    GPIO_NUM_5,
    GPIO_NUM_17,
    GPIO_NUM_16
};

static const char keymap[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

static char candidate_key = '\0';
static char stable_key = '\0';
static TickType_t candidate_since = 0;

static char scan_raw_key(void)
{
    for (int row = 0; row < ROWS; row++) {
        for (int i = 0; i < ROWS; i++) {
            gpio_set_level(row_pins[i], 1);
        }

        gpio_set_level(row_pins[row], 0);
        vTaskDelay(pdMS_TO_TICKS(2));

        for (int col = 0; col < COLS; col++) {
            if (gpio_get_level(col_pins[col]) == 0) {
                return keymap[row][col];
            }
        }
    }

    return '\0';
}

esp_err_t keypad_init(void)
{
    for (int i = 0; i < ROWS; i++) {
        esp_err_t err = gpio_reset_pin(row_pins[i]);
        if (err != ESP_OK) return err;

        err = gpio_set_direction(
            row_pins[i], GPIO_MODE_OUTPUT);
        if (err != ESP_OK) return err;

        err = gpio_set_level(row_pins[i], 1);
        if (err != ESP_OK) return err;
    }

    for (int i = 0; i < COLS; i++) {
        esp_err_t err = gpio_reset_pin(col_pins[i]);
        if (err != ESP_OK) return err;

        err = gpio_set_direction(
            col_pins[i], GPIO_MODE_INPUT);
        if (err != ESP_OK) return err;

        err = gpio_set_pull_mode(
            col_pins[i], GPIO_PULLUP_ONLY);
        if (err != ESP_OK) return err;
    }

    return ESP_OK;
}

char keypad_get_key(void)
{
    char raw_key = scan_raw_key();
    TickType_t now = xTaskGetTickCount();

    if (raw_key != candidate_key) {
        candidate_key = raw_key;
        candidate_since = now;
    }

    if ((now - candidate_since) >=
        pdMS_TO_TICKS(DEBOUNCE_MS)) {

        if (candidate_key != stable_key) {
            stable_key = candidate_key;

            // Report only a new press, not a release.
            if (stable_key != '\0') {
                return stable_key;
            }
        }
    }

    return '\0';
}