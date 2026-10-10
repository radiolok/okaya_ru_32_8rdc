// Controllable fakes for the Arduino core and the display driver.
#ifndef FAKES_H
#define FAKES_H

#include <stdint.h>
#include <string>
#include "display.h"

extern uint8_t fake_screen[DISP_SIZE];        // last byte written per address
extern uint16_t fake_write_count[DISP_SIZE];  // writes per address since last reset/clear
extern uint32_t fake_total_writes;
extern uint32_t fake_clear_count;
extern unsigned long fake_millis;
extern std::string fake_serial_tx;
extern std::string fake_serial_rx;

void fake_reset();
void fake_reset_write_counts();
// Values returned by the next random() calls (clamped to the requested range).
void fake_random_push(long value);
void fake_random_clear();
// Row of the fake screen as a string (no trailing-space trimming).
std::string fake_row(uint8_t row);

#endif
