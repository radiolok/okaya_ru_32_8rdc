#include "display.h"
#include "hal_595.h"
#include "hal_hv.h"
#include "pins.h"
#include <Arduino.h>

#define STROBE_DELAY_CYCLES ((F_CPU / 1000000UL) * DISP_STROBE_DELAY_US)

static inline void strobe_delay() {
    __builtin_avr_delay_cycles(STROBE_DELAY_CYCLES);
}

void strobe_aw() {
    PORT_nAS &= ~MASK_nAS;
    strobe_delay();
    PORT_nAS |= MASK_nAS;
    strobe_delay();
}

void strobe_clock() {
    PORT_NS7 |= MASK_NS7;
    strobe_delay();
    PORT_NS7 &= ~MASK_NS7;
    strobe_delay();
}

void set_cs(bool active) {
    if (active) {
        PORT_NS8 &= ~MASK_NS8;
    } else {
        PORT_NS8 |= MASK_NS8;
    }
}

void set_underline(bool on) {
    if (on) {
        PORT_nAD &= ~MASK_nAD;
    } else {
        PORT_nAD |= MASK_nAD;
    }
}

bool wait_ready() {
    uint16_t timeout = 2000;
    while (timeout--) {
        if (PIN_NS4_REG & MASK_NS4) return true;
        __builtin_avr_delay_cycles(F_CPU / 100000UL);
    }
    return false;
}

void strobe_nRESET() {
    PORT_nRESET &= ~MASK_nRESET;
    delayMicroseconds(DISP_RESET_DELAY_US);
    PORT_nRESET |= MASK_nRESET;
    delayMicroseconds(DISP_RESET_DELAY_US);
}

void set_blank(bool blank) {
    if (blank) {
        PORT_nBL &= ~MASK_nBL;
    } else {
        PORT_nBL |= MASK_nBL;
    }
}

static void strobes_init() {
    DDR_NS7  |= MASK_NS7;   PORT_NS7  &= ~MASK_NS7;
    DDR_NS8  |= MASK_NS8;   PORT_NS8  &= ~MASK_NS8;
    DDR_nAD  |= MASK_nAD;   PORT_nAD  |= MASK_nAD;
    DDR_nAS  |= MASK_nAS;   PORT_nAS  |= MASK_nAS;
    DDR_nWR  |= MASK_nWR;   PORT_nWR  |= MASK_nWR;
    DDR_nBL  |= MASK_nBL;   PORT_nBL  |= MASK_nBL;
    DDR_nRESET |= MASK_nRESET; PORT_nRESET |= MASK_nRESET;
    DDR_NS4  &= ~MASK_NS4;  PORT_NS4  &= ~MASK_NS4;
}

void display_init() {
    strobes_init();
    shift595_init();
    hv_enable();

    set_blank(true);

    strobe_nRESET();
    delayMicroseconds(500);
    strobe_nRESET();
    delayMicroseconds(500);

    display_clear();
    set_blank(false);
}

void display_write_raw(uint8_t addr, uint8_t data, bool underline) {
    shift595_write(addr, data);
    strobe_delay();
    strobe_delay();

    wait_ready();
    strobe_aw();
    strobe_delay();
    strobe_delay();
    set_underline(underline);
    strobe_clock();
}

void display_write_batch(uint8_t addr, const uint8_t *data, uint16_t len) {
    for (uint16_t i = 0; i < len; i++) {
        shift595_write((uint8_t)(addr + i), data[i]);
        strobe_delay();
        strobe_delay();

        wait_ready();
        strobe_aw();
        strobe_delay();
        strobe_delay();
        set_underline(false);
        strobe_clock();
    }
}

void display_write(uint8_t col, uint8_t row, uint8_t ch) {
    if (col >= DISP_COLS || row >= DISP_ROWS) return;
    uint8_t addr = col + row * DISP_COLS;
    display_write_raw(addr, ch);
}

void display_clear() {
    set_blank(true);
    for (uint16_t addr = 0; addr < DISP_SIZE; addr++) {
        display_write_raw((uint8_t)addr, 0x20);
    }
    set_blank(false);
}

void display_set_brightness(uint8_t level) {
    set_blank(level < 128);
}
