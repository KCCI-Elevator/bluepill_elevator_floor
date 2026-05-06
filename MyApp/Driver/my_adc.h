#ifndef __MAP_DRIVER__MY_ADC_H__
#define __MAP_DRIVER__MY_ADC_H__

#include "def.h"

typedef enum {
    MY_ADC_CH_TS0224 = 0,
    MY_ADC_CH_MAX
} my_adc_ch_t;

bool adcInit(void);
bool adcRead(my_adc_ch_t ch, uint16_t *p_data);
uint16_t adcReadRaw(my_adc_ch_t ch);

#endif //__MAP_DRIVER__MY_ADC_H__