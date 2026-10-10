#include "burner_timer.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "alarm.h"
#define ALARM_GPIO GPIO_NUM_27
#define MAX_DURATION_SECONDS 99999U

static const char *TAG = "BURNER_TIMER";

typedef struct {
    bool active;
    TickType_t deadline;
} burner_timer_state_t;

static burner_timer_state_t timers[2] = {0};
static bool alarm_active = false;

static bool valid_burner(int burner)
{
    return burner >= 0 && burner < 2;
}

esp_err_t burner_timer_init(void)
{
    esp_err_t err = gpio_reset_pin(ALARM_GPIO);
    if (err != ESP_OK) return err;

    err = gpio_set_direction(
        ALARM_GPIO, GPIO_MODE_OUTPUT);
    if (err != ESP_OK) return err;

    return gpio_set_level(ALARM_GPIO, 0);
}

esp_err_t burner_timer_start(
    int burner,
    uint32_t duration_seconds)
{
    if (!valid_burner(burner) ||
        duration_seconds == 0 ||
        duration_seconds > MAX_DURATION_SECONDS) {
        return ESP_ERR_INVALID_ARG;
    }

    TickType_t duration_ticks =
        pdMS_TO_TICKS(duration_seconds * 1000U);

    timers[burner].deadline =
        xTaskGetTickCount() + duration_ticks;

    timers[burner].active = true;

    ESP_LOGI(TAG, "Burner %d started: %lu seconds",
             burner + 1,
             (unsigned long)duration_seconds);

    return ESP_OK;
}

void burner_timer_cancel(int burner)
{
    if (!valid_burner(burner)) return;

    timers[burner].active = false;

    ESP_LOGI(TAG, "Burner %d timer cancelled",
             burner + 1);
}

bool burner_timer_is_active(int burner)
{
    return valid_burner(burner) && timers[burner].active;
}

uint32_t burner_timer_remaining(int burner)
{
    if (!valid_burner(burner) ||
        !timers[burner].active) {
        return 0;
    }

    TickType_t now = xTaskGetTickCount();
    int32_t ticks_left =
        (int32_t)(timers[burner].deadline - now);

    if (ticks_left <= 0) return 0;

    // Round up so a partial second displays as one second.
    return ((uint32_t)ticks_left +
            configTICK_RATE_HZ - 1U) /
           configTICK_RATE_HZ;
}

void burner_timer_update(void)
{
    TickType_t now = xTaskGetTickCount();

    for (int i = 0; i < 2; i++) {
        if (timers[i].active &&
            (int32_t)(now - timers[i].deadline) >= 0) {

            timers[i].active = false;
            alarm_active = true;

            ESP_LOGW(TAG,
                     "Burner %d timer expired!",
                     i + 1);
        }
    }

alarm_set(alarm_active);
}

void burner_timer_acknowledge_alarm(void)
{
alarm_active = false;
alarm_set(false);

    ESP_LOGI(TAG, "Alarm acknowledged");
}

bool burner_timer_alarm_is_active(void)
{
    return alarm_active;
}