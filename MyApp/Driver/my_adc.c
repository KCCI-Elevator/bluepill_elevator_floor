#include "my_adc.h"

#include "adc.h"

#define MY_ADC_TIMEOUT_MS 10U

typedef struct {
    ADC_HandleTypeDef *h_adc;
    uint32_t channel;
    uint32_t rank;
    uint32_t sampling_time;
} my_adc_tbl_t;

// inner
static const my_adc_tbl_t adc_tbl[MY_ADC_CH_MAX] = {
    [MY_ADC_CH_TS0224] = {
        .h_adc = &hadc1,
        .channel = ADC_CHANNEL_1,   // AD1_IN1
        .rank = ADC_REGULAR_RANK_1,
        .sampling_time = ADC_SAMPLETIME_55CYCLES_5,
    },
};

static bool adcConfigChannel(my_adc_ch_t ch) {
    ADC_ChannelConfTypeDef s_config = {0};

    if (ch >= MY_ADC_CH_MAX) return false;

    s_config.Channel = adc_tbl[ch].channel;
    s_config.Rank = adc_tbl[ch].rank;
    s_config.SamplingTime = adc_tbl[ch].sampling_time;

    if (HAL_ADC_ConfigChannel(adc_tbl[ch].h_adc, &s_config) != HAL_OK) return false;

    return true;
}

// function
bool adcInit(void) {
    /*
     * STM32F1 ADC는 사용 전 calibration 권장.
     * MX_ADC1_Init() 이후에 호출되어야 합니다.
     */
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK) return false;

    return true;
}

bool adcRead(my_adc_ch_t ch, uint16_t *p_data) {
    ADC_HandleTypeDef *h_adc;

    if (p_data == NULL) return false;
    if (ch >= MY_ADC_CH_MAX) return false;

    h_adc = adc_tbl[ch].h_adc;

    if (adcConfigChannel(ch) != true) return false;

    if (HAL_ADC_Start(h_adc) != HAL_OK) {
        HAL_ADC_Stop(h_adc);
        return false;
    }

    if (HAL_ADC_PollForConversion(h_adc, MY_ADC_TIMEOUT_MS) != HAL_OK) {
        HAL_ADC_Stop(h_adc);
        return false;
    }

    *p_data = (uint16_t)HAL_ADC_GetValue(h_adc);

    HAL_ADC_Stop(h_adc);

    return true;
}

uint16_t adcReadRaw(my_adc_ch_t ch) {
    uint16_t adc_value = 0;

    adcRead(ch, &adc_value);

    return adc_value;
}