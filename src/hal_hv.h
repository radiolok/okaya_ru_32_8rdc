#ifndef HAL_HV_H
#define HAL_HV_H

/** @brief Enable the external HV converter module.
 *
 *  Drives PB1 (PIN_HV_EN) LOW — the converter is pulled HIGH externally
 *  via a pull-up resistor, so LOW = enabled.
 */
void hv_enable();

#endif
