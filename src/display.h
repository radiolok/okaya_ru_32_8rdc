#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include "pins.h"

#define DISP_COLS 32
#define DISP_ROWS 8
#define DISP_SIZE (DISP_COLS * DISP_ROWS)

#define DISP_STROBE_DELAY_US 1
#define DISP_RESET_DELAY_US  100

void strobe_nAS();
void strobe_nAD();
void strobe_NS8();
void strobe_nWR();
void strobe_nRESET();
void set_blank(bool blank);

void display_init();
void display_write(uint8_t col, uint8_t row, uint8_t ch);
void display_write_raw(uint8_t addr, uint8_t data);
void display_clear();
void display_set_brightness(uint8_t level);
bool display_is_busy();

#endif
