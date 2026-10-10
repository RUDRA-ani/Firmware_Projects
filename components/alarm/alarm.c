
#include "alarm.h"
#include "driver/gpio.h"

#define ALARM_GPIO GPIO_NUM_27

static bool current_alarm_state = false;

esp_err_t alarm_init(void)
{
    esp_err_t err = gpio_reset_pin(ALARM_GPIO);
    if (err != ESP_OK) {
        return err;
    }

    err = gpio_set_direction(ALARM_GPIO, GPIO_MODE_OUTPUT);
    if (err != ESP_OK) {
        return err;
    }

    current_alarm_state = false;

    return gpio_set_level(ALARM_GPIO, 0);
}

void alarm_set(bool active)
{
    current_alarm_state = active;
    gpio_set_level(ALARM_GPIO, active ? 1 : 0);
}

bool alarm_is_active(void)
{
    return current_alarm_state;
}
