#include "ts0224.h"
#include "hw_def.h"
#include "my_adc.h"
#include "main.h"

#define TS0224_ADC_CH MY_ADC_CH_TS0224
#define TS0224_DEFAULT_THRESHOLD 200U
#define TS0224_CALIBRATION_SAMPLES 16U

// inner
static uint16_t ts0224_adc_raw = 0;
static uint16_t ts0224_adc_baseline = 0;
static uint16_t ts0224_adc_diff = 0;
static uint16_t ts0224_adc_threshold = TS0224_DEFAULT_THRESHOLD;

static bool ts0224_d0_raw = false;
static bool ts0224_detected_digital = false;
static bool ts0224_detected_analog = false;

static ts0224_d0_active_t ts0224_d0_active = TS0224_D0_ACTIVE_LOW;

static uint16_t ts0224AbsDiff(uint16_t a, uint16_t b) {
    if (a >= b) {
        return a - b;
    }

    return b - a;
}

static bool ts0224ReadAnalog(uint16_t *p_data) {
    if (p_data == NULL) return false;

    return adcRead(TS0224_ADC_CH, p_data);
}

static bool ts0224ReadDigital(void) {
    GPIO_PinState pin_state;
    /*
     * CubeMX에서 PA2 핀 User Label을 TS0224_D0로 설정해야 합니다.
     *
     * main.h에 아래 매크로가 생성되어 있어야 합니다.
     * #define TS0224_D0_Pin GPIO_PIN_2
     * #define TS0224_D0_GPIO_Port GPIOA
     */
    pin_state = HAL_GPIO_ReadPin(TS0224_D0_GPIO_Port, TS0224_D0_Pin);

    if (pin_state == GPIO_PIN_SET) return true;

    return false;
}

// function
bool ts0224Init(void) {
    /*
     * myAdcInit()은 bspInit()에서 먼저 호출되는 구조를 권장합니다.
     *
     * 자석이 없는 상태에서 ts0224Init()이 호출되어야
     * 기준값 baseline이 정상적으로 잡힙니다.
     */
    if (ts0224Calibrate(TS0224_CALIBRATION_SAMPLES) != true) return false;
    if (ts0224Update() != true) return false;

    return true;
}

bool ts0224Update(void) {
    if (ts0224ReadAnalog(&ts0224_adc_raw) != true) return false;

    ts0224_adc_diff = ts0224AbsDiff(ts0224_adc_raw, ts0224_adc_baseline);

    if (ts0224_adc_diff >= ts0224_adc_threshold) ts0224_detected_analog = true;
    else                                         ts0224_detected_analog = false;

    ts0224_d0_raw = ts0224ReadDigital();

    if (ts0224_d0_active == TS0224_D0_ACTIVE_HIGH) ts0224_detected_digital = ts0224_d0_raw;
    else                                           ts0224_detected_digital = !ts0224_d0_raw;

    return true;
}

bool ts0224Calibrate(uint8_t samples) {
    uint32_t sum = 0;
    uint16_t adc_value = 0;

    if (samples == 0) return false;

    for (uint8_t i = 0; i < samples; i++) {
        if (ts0224ReadAnalog(&adc_value) != true) return false;

        sum += adc_value;
    }

    ts0224_adc_baseline = (uint16_t)(sum / samples);
    ts0224_adc_raw = ts0224_adc_baseline;
    ts0224_adc_diff = 0;
    ts0224_detected_analog = false;

    return true;
}

uint16_t ts0224GetAnalogRaw(void) {
    return ts0224_adc_raw;
}

uint8_t ts0224GetAnalogPercent(void) {
    return (uint8_t)((ts0224_adc_raw * 100U) / TS0224_ADC_MAX_VALUE);
}

uint16_t ts0224GetBaseline(void) {
    return ts0224_adc_baseline;
}

uint16_t ts0224GetDiff(void) {
    return ts0224_adc_diff;
}

bool ts0224GetD0Raw(void) {
    return ts0224_d0_raw;
}

bool ts0224IsDetectedDigital(void) {
    return ts0224_detected_digital;
}

bool ts0224IsDetectedAnalog(void) {
    return ts0224_detected_analog;
}

void ts0224SetD0Active(ts0224_d0_active_t active) {
    ts0224_d0_active = active;
}

void ts0224SetAnalogThreshold(uint16_t threshold) {
    ts0224_adc_threshold = threshold;
}