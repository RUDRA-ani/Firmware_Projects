// #include <stdio.h>
// #include <stdint.h>
// #include <stdbool.h>
// #include <math.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_log.h"
// #include "esp_err.h"
// #include "esp_adc/adc_oneshot.h"
// #include "driver/gpio.h"

// // ---------------- ADC CONFIGURATION ----------------

// #define BURNER_1_ADC_CHANNEL ADC_CHANNEL_6  // GPIO34
// #define BURNER_2_ADC_CHANNEL ADC_CHANNEL_7  // GPIO35

// #define ADC_MAX_VALUE 4095.0
// #define NTC_BETA 3950.0
// #define NTC_NOMINAL_RESISTANCE 10000.0
// #define NTC_NOMINAL_TEMP 298.15
// #define FIXED_RESISTANCE 10000.0

// // ---------------- KEYPAD CONFIGURATION ----------------

// #define ROWS 4
// #define COLS 4

// static const gpio_num_t row_pins[ROWS] = {
//     GPIO_NUM_23, GPIO_NUM_22, GPIO_NUM_21, GPIO_NUM_19
// };

// static const gpio_num_t col_pins[COLS] = {
//     GPIO_NUM_18, GPIO_NUM_5, GPIO_NUM_17, GPIO_NUM_16
// };

// static const char keymap[ROWS][COLS] = {
//     {'1', '2', '3', 'A'},
//     {'4', '5', '6', 'B'},
//     {'7', '8', '9', 'C'},
//     {'*', '0', '#', 'D'}
// };

// // ---------------- OUTPUT CONFIGURATION ----------------

// #define ALARM_GPIO GPIO_NUM_27

// static const char *TAG = "STOVE";

// // ---------------- TIMER STATE ----------------

// typedef struct {
//     bool active;
//     TickType_t deadline;
// } burner_timer_t;

// static burner_timer_t timers[2] = {0};

// static int selected_burner = -1;
// static uint32_t input_seconds = 0;
// static bool alarm_active = false;

// // ---------------- TEMPERATURE CONVERSION ----------------

// static float adc_to_temperature(int adc_raw)
// {
//     if (adc_raw <= 0 || adc_raw >= 4095) {
//         return NAN;
//     }

//     float resistance =
//         FIXED_RESISTANCE *
//         ((float)adc_raw / (ADC_MAX_VALUE - adc_raw));

//     float temperature_kelvin =
//         1.0 / (
//             log(resistance / NTC_NOMINAL_RESISTANCE) / NTC_BETA
//             + 1.0 / NTC_NOMINAL_TEMP
//         );

//     return temperature_kelvin - 273.15;
// }

// // ---------------- KEYPAD ----------------

// static void keypad_init(void)
// {
//     for (int i = 0; i < ROWS; i++) {
//         ESP_ERROR_CHECK(gpio_reset_pin(row_pins[i]));
//         ESP_ERROR_CHECK(
//             gpio_set_direction(row_pins[i], GPIO_MODE_OUTPUT)
//         );
//         ESP_ERROR_CHECK(gpio_set_level(row_pins[i], 1));
//     }

//     for (int i = 0; i < COLS; i++) {
//         ESP_ERROR_CHECK(gpio_reset_pin(col_pins[i]));
//         ESP_ERROR_CHECK(
//             gpio_set_direction(col_pins[i], GPIO_MODE_INPUT)
//         );
//         ESP_ERROR_CHECK(
//             gpio_set_pull_mode(col_pins[i], GPIO_PULLUP_ONLY)
//         );
//     }

//     ESP_LOGI(TAG, "Keypad initialized");
// }

// static char keypad_scan(void)
// {
//     for (int row = 0; row < ROWS; row++) {
//         for (int i = 0; i < ROWS; i++) {
//             gpio_set_level(row_pins[i], 1);
//         }

//         gpio_set_level(row_pins[row], 0);
//         vTaskDelay(pdMS_TO_TICKS(2));

//         for (int col = 0; col < COLS; col++) {
//             if (gpio_get_level(col_pins[col]) == 0) {
//                 return keymap[row][col];
//             }
//         }
//     }

//     return '\0';
// }

// // ---------------- TIMER CONTROLS ----------------

// static void select_burner(int burner)
// {
//     selected_burner = burner;
//     input_seconds = 0;

//     ESP_LOGI(TAG, "Burner %d selected",
//              burner + 1);
// }

// static void handle_key(char key)
// {
//     if (key == 'A') {
//         select_burner(0);
//         return;
//     }

//     if (key == 'B') {
//         select_burner(1);
//         return;
//     }

//     // Enter timer duration in seconds.
//     if (key >= '0' && key <= '9') {
//         if (selected_burner == -1) {
//             ESP_LOGW(TAG, "Select burner A or B first");
//             return;
//         }

//         uint32_t digit = (uint32_t)(key - '0');

//         // Limit duration to 99999 seconds.
//         if (input_seconds <= (99999U - digit) / 10U) {
//             input_seconds = input_seconds * 10U + digit;

//             ESP_LOGI(TAG, "Timer input: %lu seconds",
//                      (unsigned long)input_seconds);
//         } else {
//             ESP_LOGW(TAG, "Maximum duration is 99999 seconds");
//         }

//         return;
//     }

//     // Start the selected burner's timer.
//     if (key == '#') {
//         if (selected_burner == -1) {
//             ESP_LOGW(TAG, "Select burner A or B first");
//             return;
//         }

//         if (input_seconds == 0) {
//             ESP_LOGW(TAG, "Enter a duration greater than zero");
//             return;
//         }

//         int burner = selected_burner;

//         timers[burner].deadline =
//             xTaskGetTickCount() +
//             pdMS_TO_TICKS(input_seconds * 1000U);

//         timers[burner].active = true;

//         ESP_LOGI(TAG, "Burner %d timer started: %lu seconds",
//                  burner + 1,
//                  (unsigned long)input_seconds);

//         input_seconds = 0;
//         return;
//     }

//     // Clear input, cancel the selected timer,
//     // or acknowledge an active alarm.
//     if (key == '*') {
//         if (input_seconds > 0) {
//             input_seconds = 0;
//             ESP_LOGI(TAG, "Timer input cleared");
//         } else if (selected_burner != -1 &&
//                    timers[selected_burner].active) {
//             timers[selected_burner].active = false;

//             ESP_LOGI(TAG, "Burner %d timer cancelled",
//                      selected_burner + 1);
//         }

//         if (alarm_active) {
//             alarm_active = false;
//             ESP_LOGI(TAG, "Alarm acknowledged");
//         }

//         return;
//     }
// }

// // ---------------- TIMER MONITORING ----------------

// static void update_timers(void)
// {
//     TickType_t now = xTaskGetTickCount();

//     for (int i = 0; i < 2; i++) {
//         if (timers[i].active &&
//             (int32_t)(now - timers[i].deadline) >= 0) {

//             timers[i].active = false;
//             alarm_active = true;

//             ESP_LOGW(TAG,
//                      "Burner %d timer expired! ALARM ACTIVE",
//                      i + 1);
//         }
//     }

//     gpio_set_level(ALARM_GPIO, alarm_active ? 1 : 0);
// }

// // ---------------- MAIN APPLICATION ----------------

// void app_main(void)
// {
//     // Initialize ADC.
//     adc_oneshot_unit_handle_t adc_handle;

//     adc_oneshot_unit_init_cfg_t init_config = {
//         .unit_id = ADC_UNIT_1,
//         .ulp_mode = ADC_ULP_MODE_DISABLE,
//     };

//     ESP_ERROR_CHECK(
//         adc_oneshot_new_unit(&init_config, &adc_handle)
//     );

//     adc_oneshot_chan_cfg_t channel_config = {
//         .atten = ADC_ATTEN_DB_12,
//         .bitwidth = ADC_BITWIDTH_DEFAULT,
//     };

//     ESP_ERROR_CHECK(adc_oneshot_config_channel(
//         adc_handle, BURNER_1_ADC_CHANNEL, &channel_config));

//     ESP_ERROR_CHECK(adc_oneshot_config_channel(
//         adc_handle, BURNER_2_ADC_CHANNEL, &channel_config));

//     // Initialize alarm output.
//     ESP_ERROR_CHECK(gpio_reset_pin(ALARM_GPIO));
//     ESP_ERROR_CHECK(
//         gpio_set_direction(ALARM_GPIO, GPIO_MODE_OUTPUT)
//     );
//     ESP_ERROR_CHECK(gpio_set_level(ALARM_GPIO, 0));

//     // Initialize keypad.
//     keypad_init();

//     ESP_LOGI(TAG, "Dual-burner timer system started");
//     ESP_LOGI(TAG, "A=Burner 1, B=Burner 2, #=Start, *=Cancel");

//     char last_key = '\0';
//     TickType_t last_temp_tick = xTaskGetTickCount();

//     while (1) {
//         // Scan keypad and process each new press.
//         char key = keypad_scan();

//         if (key != '\0' && key != last_key) {
//             handle_key(key);
//         }

//         if (key == '\0') {
//             last_key = '\0';
//         } else {
//             last_key = key;
//         }

//         // Check both timers without blocking.
//         update_timers();

//         // Read temperatures approximately once per second.
//         TickType_t now = xTaskGetTickCount();

//         if (now - last_temp_tick >= pdMS_TO_TICKS(1000)) {
//             int burner1_raw = 0;
//             int burner2_raw = 0;

//             esp_err_t err1 = adc_oneshot_read(
//                 adc_handle, BURNER_1_ADC_CHANNEL, &burner1_raw);

//             esp_err_t err2 = adc_oneshot_read(
//                 adc_handle, BURNER_2_ADC_CHANNEL, &burner2_raw);

//             if (err1 == ESP_OK && err2 == ESP_OK) {
//                 ESP_LOGI(TAG,
//                          "Burner 1: %.2f C | Burner 2: %.2f C",
//                          adc_to_temperature(burner1_raw),
//                          adc_to_temperature(burner2_raw));
//             } else {
//                 ESP_LOGE(TAG, "ADC read failed");
//             }

//             last_temp_tick = now;
//         }

//         vTaskDelay(pdMS_TO_TICKS(30));
//     }
// }