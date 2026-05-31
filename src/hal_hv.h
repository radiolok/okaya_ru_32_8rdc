#ifndef HAL_HV_H
#define HAL_HV_H

#include <stdint.h>
#include <stdbool.h>

// Divider: 470k / 10k, HV=160V → ADC=3.33V → 10-bit: 160*10/480*1024/5.0 ≈ 683
#define HV_TARGET_ADC  683
#define HV_START_DUTY  0
#define HV_SOFT_START_MS  1500
#define HV_TIMEOUT_MS  3000
#define HV_KP  2

void hv_init();
void hv_start_soft();
uint16_t hv_read();
bool hv_is_ready();

#endif
