#ifndef __MAP_DRIVER__TS0224_H__
#define __MAP_DRIVER__TS0224_H__

#include "def.h"

#define TS0224_ADC_MAX_VALUE    4095U

typedef enum {
    TS0224_D0_ACTIVE_LOW = 0,
    TS0224_D0_ACTIVE_HIGH
} ts0224_d0_active_t;

bool ts0224Init(void);
bool ts0224Update(void);

bool ts0224Calibrate(uint8_t samples);

uint16_t ts0224GetAnalogRaw(void);
uint8_t  ts0224GetAnalogPercent(void);

uint16_t ts0224GetBaseline(void);
uint16_t ts0224GetDiff(void);

bool ts0224GetD0Raw(void);
bool ts0224IsDetectedDigital(void);
bool ts0224IsDetectedAnalog(void);

void ts0224SetD0Active(ts0224_d0_active_t active);
void ts0224SetAnalogThreshold(uint16_t threshold);

#endif //__MAP_DRIVER__TS0224_H__