#ifndef HAL_595_H
#define HAL_595_H

#include <stdint.h>

/** @brief Initialize the 74HC595 shift register chain over hardware SPI.
 *
 * Configures PB3 (MOSI), PB5 (SCK) as SPI outputs, PB2 (SS) as OUTPUT
 * to prevent SPI from switching to slave mode, and PD2 as RCLK (STCP).
 * SPI runs at F_CPU/2 (8 MHz @ 16 MHz).
 */
void shift595_init();

/** @brief Shift out a 16-bit frame and latch both 595 registers.
 *
 * Data byte is shifted first (ends up in the second 595 — data bus),
 * address byte second (stays in the first 595 — address bus).
 *
 * @param addr Display address (0–255), routed to the first 595.
 * @param data Character code, routed to the second 595.
 */
void shift595_write(uint8_t addr, uint8_t data);

#endif
