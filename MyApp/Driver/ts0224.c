#include "ts0224.h"
#include "hw_def.h"
#include "my_adc.h"
#include "main.h"

// PA1 연결을 위한 채널 정의 (my_adc.h에서 1로 정의되어 있어야 함)
#define TS0224_ADC_CH           MY_ADC_CH_TS0224
#define TS0224_DEFAULT_THRESHOLD 200U
#define TS0224_CALIBRATION_SAMPLES 16U

static uint16_t ts0224_adc_raw = 0;
static uint16_t ts0224_adc_baseline = 0;
static uint16_t ts0224_adc_diff = 0;
static uint16_t ts0224_adc_threshold = TS0224_DEFAULT_THRESHOLD;

static bool ts0224_d0_raw = false;
static bool ts0224_detected_digital = false;
static bool ts0224_detected_analog = false;

static ts0224_d0_active_t ts0224_d0_active = TS0224_D0_ACTIVE_LOW;

// 절댓값 차이 계산: $Diff = |a - b|$
static uint16_t ts0224AbsDiff(uint16_t a, uint16_t b) {
    return (a >= b) ? (a - b) : (b - a);
}

static bool ts0224ReadAnalog(uint16_t *p_data) {
    if (p_data == NULL) return false;
    return adcRead(TS0224_ADC_CH, p_data);
}

static bool ts0224ReadDigital(void) {
    // CubeMX에서 PA2를 TS0224_D0로 라벨링했을 경우
    return (HAL_GPIO_ReadPin(TS0224_D0_GPIO_Port, TS0224_D0_Pin) == GPIO_PIN_SET);
}

bool ts0224Init(void) {
    // 1. 자석이 없는 상태에서 기준값 설정
    if (ts0224Calibrate(TS0224_CALIBRATION_SAMPLES) != true) return false;

    // 2. 초기 임계값 설정 (환경에 따라 100~500 사이 튜닝)
    ts0224SetAnalogThreshold(TS0224_DEFAULT_THRESHOLD);

    return ts0224Update();
}

bool ts0224Update(void) {
    if (ts0224ReadAnalog(&ts0224_adc_raw) != true) return false;

    // 현재값과 기준값의 차이 계산
    ts0224_adc_diff = ts0224AbsDiff(ts0224_adc_raw, ts0224_adc_baseline);

    // 차이가 임계값보다 크면 자석 감지로 판단: $Diff \ge Threshold$
    ts0224_detected_analog = (ts0224_adc_diff >= ts0224_adc_threshold);

    // 디지털 신호 처리 (PA2)
    ts0224_d0_raw = ts0224ReadDigital();
    if (ts0224_d0_active == TS0224_D0_ACTIVE_HIGH) {
        ts0224_detected_digital = ts0224_d0_raw;
    } else {
        ts0224_detected_digital = !ts0224_d0_raw;
    }

    return true;
}

bool ts0224Calibrate(uint8_t samples) {
    uint32_t sum = 0;
    uint16_t adc_value = 0;

    if (samples == 0) return false;

    for (uint8_t i = 0; i < samples; i++) {
        if (ts0224ReadAnalog(&adc_value) != true) return false;
        sum += adc_value;
        HAL_Delay(1); // 샘플링 간격 확보 (노이즈 필터링 효과)
    }

    ts0224_adc_baseline = (uint16_t)(sum / samples);
    ts0224_adc_raw = ts0224_adc_baseline;
    ts0224_adc_diff = 0;
    ts0224_detected_analog = false;

    return true;
}

// --- Getter / Setter 생략 (기존 코드와 동일) ---
uint16_t ts0224GetAnalogRaw(void) { return ts0224_adc_raw; }
uint16_t ts0224GetBaseline(void) { return ts0224_adc_baseline; }
uint16_t ts0224GetDiff(void) { return ts0224_adc_diff; }
bool ts0224IsDetectedAnalog(void) { return ts0224_detected_analog; }
void ts0224SetAnalogThreshold(uint16_t threshold) { ts0224_adc_threshold = threshold; }