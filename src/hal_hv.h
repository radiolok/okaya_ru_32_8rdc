#ifndef HAL_HV_H
#define HAL_HV_H

#include <stdint.h>
#include <stdbool.h>

/** @brief Target ADC value for 160 V anode voltage.
 *
 * Divider: 470 kΩ / 10 kΩ → 3.33 V at 160 V.
 * ADC 10-bit, AVCC reference: 160 × 10 / 480 × 1024 / 5.0 ≈ 683.
 */
#define HV_TARGET_ADC  683

/** @brief Initial PWM duty during soft-start (0 = off). */
#define HV_START_DUTY  0

/** @brief Soft-start ramp duration in milliseconds. */
#define HV_SOFT_START_MS  1500

/** @brief Timeout for reaching target voltage in milliseconds. */
#define HV_TIMEOUT_MS  3000

/** @brief Proportional controller gain (scaled by 1/16 internally). */
#define HV_KP  2

/** @brief Initialize Timer1 Fast PWM and ADC in free-running mode.
 *
 * Timer1: mode 14 (Fast PWM, ICR1=TOP), prescaler 1, ~30 kHz, 533 steps.
 * Output on PB1 (OC1A), non-inverting.
 * ADC: free-running @ 9.6 kHz, interrupt-driven, AVCC reference, channel 3 (PC3).
 */
void hv_init();

/** @brief Execute soft-start ramp and transition to closed-loop regulation.
 *
 * Ramps PWM duty linearly from 0 to HV_PWM_MAX over @ref HV_SOFT_START_MS,
 * then enables the P-controller in the ADC ISR and waits up to
 * @ref HV_TIMEOUT_MS for the voltage to reach @ref HV_TARGET_ADC.
 * Blocks until either the target is reached or the timeout expires.
 */
void hv_start_soft();

/** @brief Perform a single ADC conversion on the HV feedback channel.
 *
 * Temporarily disables free-running mode to take a one-shot reading.
 *
 * @return Raw 10-bit ADC value (0–1023).
 */
uint16_t hv_read();

/** @brief Check whether the anode voltage has reached the target.
 *
 * @return true if HV regulation is stable at the target voltage.
 */
bool hv_is_ready();

#endif
