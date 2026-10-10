#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "temperature.h"
#include "keypad.h"
#include "burner_timer.h"


#define MAX_DURATION_SECONDS 99999U

static const char *TAG = "STOVE";

static int selected_burner = -1;
static uint32_t input_seconds = 0;

static void handle_key(char key)
{
    if (key == 'A' || key == 'B') {
        selected_burner = (key == 'A') ? 0 : 1;
        input_seconds = 0;

        ESP_LOGI(TAG, "Selected Burner %d",
                 selected_burner + 1);
        return;
    }

    if (key >= '0' && key <= '9') {
        if (selected_burner == -1) {
            ESP_LOGW(TAG, "Select A or B first");
            return;
        }

        uint32_t digit = (uint32_t)(key - '0');

        if (input_seconds <=
            (MAX_DURATION_SECONDS - digit) / 10U) {

            input_seconds = input_seconds * 10U + digit;

            ESP_LOGI(TAG, "Entered duration: %lu seconds",
                     (unsigned long)input_seconds);
        } else {
            ESP_LOGW(TAG, "Maximum duration exceeded");
        }

        return;
    }

    if (key == '#') {
        if (selected_burner == -1) {
            ESP_LOGW(TAG, "Select a burner first");
            return;
        }

        if (input_seconds == 0) {
            ESP_LOGW(TAG, "Enter a duration greater than zero");
            return;
        }

        esp_err_t err = burner_timer_start(
            selected_burner, input_seconds);

        if (err == ESP_OK) {
            input_seconds = 0;
        } else {
            ESP_LOGE(TAG, "Could not start timer: %s",
                     esp_err_to_name(err));
        }

        return;
    }

    if (key == '*') {
        input_seconds = 0;

        if (selected_burner != -1) {
            burner_timer_cancel(selected_burner);
        }

        if (burner_timer_alarm_is_active()) {
            burner_timer_acknowledge_alarm();
        }

        ESP_LOGI(TAG, "Input cleared / cancel requested");
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(temperature_init());
    ESP_ERROR_CHECK(keypad_init());
    ESP_ERROR_CHECK(burner_timer_init());

    ESP_LOGI(TAG, "Dual-burner monitor ready");
    ESP_LOGI(TAG, "A/B select | Digits enter seconds | # start | * cancel");

    TickType_t last_report = xTaskGetTickCount();

    while (1) {
        char key = keypad_get_key();

        if (key != '\0') {
            ESP_LOGI(TAG, "Key pressed: %c", key);
            handle_key(key);
        }

        burner_timer_update();

        TickType_t now = xTaskGetTickCount();

        if ((now - last_report) >= pdMS_TO_TICKS(1000)) {
            float burner1_temp;
            float burner2_temp;

            esp_err_t err = temperature_read(
                &burner1_temp, &burner2_temp);


                if (err == ESP_OK) {
                    if (temperature_is_valid(0)) {
                        ESP_LOGI(TAG, "Burner 1 temperature: %.2f C",
                                burner1_temp);
                    } else {
                        ESP_LOGE(TAG, "Burner 1 sensor fault!");
                    }

                    if (temperature_is_valid(1)) {
                        ESP_LOGI(TAG, "Burner 2 temperature: %.2f C",
                                burner2_temp);
                    } else {
                        ESP_LOGE(TAG, "Burner 2 sensor fault!");
                    }
                } else {
                    ESP_LOGE(TAG, "ADC read failed: %s",
                            esp_err_to_name(err));
                }


            for (int i = 0; i < 2; i++) {
                if (burner_timer_is_active(i)) {
                    ESP_LOGI(TAG,
                             "Burner %d remaining: %lu seconds",
                             i + 1,
                             (unsigned long)
                                 burner_timer_remaining(i));
                }
            }

            last_report = now;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}