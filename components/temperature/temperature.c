#include <math.h>

#include "temperature.h"
#include "esp_adc/adc_oneshot.h"

#define BURNER_1_ADC_CHANNEL ADC_CHANNEL_6  // GPIO34
#define BURNER_2_ADC_CHANNEL ADC_CHANNEL_7  // GPIO35

#define ADC_MAX_VALUE 4095.0f
#define NTC_BETA 3950.0f
#define NTC_NOMINAL_RESISTANCE 10000.0f
#define NTC_NOMINAL_TEMP 298.15f
#define FIXED_RESISTANCE 10000.0f

static adc_oneshot_unit_handle_t adc_handle = NULL;


#define MIN_VALID_TEMP_C (-20.0f)
#define MAX_VALID_TEMP_C (150.0f)

static bool sensor_valid[2] = { false, false };

static bool validate_temperature(int raw, float temp)
{
    if (raw <= 0 || raw >= 4095) {
        return false;
    }

    if (!isfinite(temp)) {
        return false;
    }

    if (temp < MIN_VALID_TEMP_C ||
        temp > MAX_VALID_TEMP_C) {
        return false;
    }

    return true;
}

bool temperature_is_valid(int burner)
{
    if (burner < 0 || burner >= 2) {
        return false;
    }

    return sensor_valid[burner];
}


static float adc_to_temperature(int adc_raw)
{
    if (adc_raw <= 0 || adc_raw >= 4095) {
        return NAN;
    }

    /*
     * Retains the divider equation that matched the
     * existing Wokwi tests. Verify accuracy across the
     * operating range before using physical hardware.
     */
    float resistance =
        FIXED_RESISTANCE *
        ((float)adc_raw / (ADC_MAX_VALUE - adc_raw));

    float inverse_temperature =
        (logf(resistance / NTC_NOMINAL_RESISTANCE) / NTC_BETA)
        + (1.0f / NTC_NOMINAL_TEMP);

    return (1.0f / inverse_temperature) - 273.15f;
}

esp_err_t temperature_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };

    esp_err_t err =
        adc_oneshot_new_unit(&init_config, &adc_handle);

    if (err != ESP_OK) {
        return err;
    }

    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    err = adc_oneshot_config_channel(
        adc_handle, BURNER_1_ADC_CHANNEL, &channel_config);

    if (err != ESP_OK) {
        return err;
    }

    return adc_oneshot_config_channel(
        adc_handle, BURNER_2_ADC_CHANNEL, &channel_config);
}


esp_err_t temperature_read(
    float *burner1_c,
    float *burner2_c)
{
    if (adc_handle == NULL ||
        burner1_c == NULL ||
        burner2_c == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    int raw1 = 0;
    int raw2 = 0;

    esp_err_t err = adc_oneshot_read(
        adc_handle, BURNER_1_ADC_CHANNEL, &raw1);

    if (err != ESP_OK) {
        sensor_valid[0] = false;
        return err;
    }

    err = adc_oneshot_read(
        adc_handle, BURNER_2_ADC_CHANNEL, &raw2);

    if (err != ESP_OK) {
        sensor_valid[1] = false;
        return err;
    }

    float temp1 = adc_to_temperature(raw1);
    float temp2 = adc_to_temperature(raw2);

    sensor_valid[0] = validate_temperature(raw1, temp1);
    sensor_valid[1] = validate_temperature(raw2, temp2);

    *burner1_c = temp1;
    *burner2_c = temp2;

    return ESP_OK;
}
