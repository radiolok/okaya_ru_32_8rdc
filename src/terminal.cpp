#include "terminal.h"
#include "display.h"

static uint8_t buf[TERM_SIZE];
static bool dirty[TERM_SIZE];
static uint8_t cursor_col = 0;
static uint8_t cursor_row = 0;
static bool insert_mode = false;

void terminal_init() {
    for (uint16_t i = 0; i < TERM_SIZE; i++) {
        buf[i] = ' ';
        dirty[i] = true;
    }
    cursor_col = 0;
    cursor_row = 0;
    insert_mode = false;
}

void terminal_reset() {
    terminal_init();
    terminal_flush();
}

void terminal_set_cursor(uint8_t col, uint8_t row) {
    if (col >= TERM_COLS) col = TERM_COLS - 1;
    if (row >= TERM_ROWS) row = TERM_ROWS - 1;
    cursor_col = col;
    cursor_row = row;
}


static void scroll_up() {
    for (uint8_t row = 1; row < TERM_ROWS; row++) {
        for (uint8_t col = 0; col < TERM_COLS; col++) {
            uint16_t dst = col + (row - 1) * TERM_COLS;
            uint16_t src = col + row * TERM_COLS;
            buf[dst] = buf[src];
            dirty[dst] = true;
        }
    }
    for (uint8_t col = 0; col < TERM_COLS; col++) {
        uint16_t idx = col + (TERM_ROWS - 1) * TERM_COLS;
        buf[idx] = ' ';
        dirty[idx] = true;
    }
}

static void advance_cursor() {
    cursor_col++;
    if (cursor_col >= TERM_COLS) {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= TERM_ROWS) {
            cursor_row = TERM_ROWS - 1;
            scroll_up();
        }
    }
}

void terminal_putchar(char c) {
    if (c == '\r') {
        cursor_col = 0;
        return;
    }

    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= TERM_ROWS) {
            cursor_row = TERM_ROWS - 1;
            scroll_up();
        }
        return;
    }

    if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
        } else if (cursor_row > 0) {
            cursor_row--;
            cursor_col = TERM_COLS - 1;
        }
        return;
    }

    if (c == '\t') {
        uint8_t spaces = 4 - (cursor_col % 4);
        for (uint8_t i = 0; i < spaces; i++) {
            terminal_putchar(' ');
        }
        return;
    }

    if (c < 0x20) return;

    uint16_t idx = cursor_col + cursor_row * TERM_COLS;

    if (insert_mode) {
        for (uint8_t col = TERM_COLS - 1; col > cursor_col; col--) {
            uint16_t src_idx = col - 1 + cursor_row * TERM_COLS;
            uint16_t dst_idx = col + cursor_row * TERM_COLS;
            buf[dst_idx] = buf[src_idx];
            dirty[dst_idx] = true;
        }
    }

    buf[idx] = c;
    dirty[idx] = true;
    advance_cursor();
}

void terminal_puts(const char *s) {
    while (*s) terminal_putchar(*s++);
}

void terminal_flush() {
    for (uint16_t i = 0; i < TERM_SIZE; i++) {
        if (dirty[i]) {
            display_write_raw((uint8_t)i, buf[i]);
            dirty[i] = false;
        }
    }
}

void terminal_move_cursor_up(uint8_t n) {
    if (n == 0) n = 1;
    if (n > cursor_row) cursor_row = 0;
    else cursor_row -= n;
}

void terminal_move_cursor_down(uint8_t n) {
    if (n == 0) n = 1;
    cursor_row += n;
    if (cursor_row >= TERM_ROWS) cursor_row = TERM_ROWS - 1;
}

void terminal_move_cursor_right(uint8_t n) {
    if (n == 0) n = 1;
    cursor_col += n;
    if (cursor_col >= TERM_COLS) cursor_col = TERM_COLS - 1;
}

void terminal_move_cursor_left(uint8_t n) {
    if (n == 0) n = 1;
    if (n > cursor_col) cursor_col = 0;
    else cursor_col -= n;
}

void terminal_erase_from_cursor_to_end() {
    uint16_t start = cursor_col + cursor_row * TERM_COLS;
    for (uint16_t i = start; i < TERM_SIZE; i++) {
        buf[i] = ' ';
        dirty[i] = true;
    }
}

void terminal_erase_from_begin_to_cursor() {
    uint16_t end = cursor_col + cursor_row * TERM_COLS;
    for (uint16_t i = 0; i <= end; i++) {
        buf[i] = ' ';
        dirty[i] = true;
    }
}

void terminal_erase_all() {
    for (uint16_t i = 0; i < TERM_SIZE; i++) {
        buf[i] = ' ';
        dirty[i] = true;
    }
    cursor_col = 0;
    cursor_row = 0;
}

void terminal_erase_line_from_cursor_to_end() {
    uint16_t row_start = cursor_row * TERM_COLS;
    for (uint8_t col = cursor_col; col < TERM_COLS; col++) {
        uint16_t idx = row_start + col;
        buf[idx] = ' ';
        dirty[idx] = true;
    }
}

void terminal_erase_line_from_begin_to_cursor() {
    uint16_t row_start = cursor_row * TERM_COLS;
    for (uint8_t col = 0; col <= cursor_col; col++) {
        uint16_t idx = row_start + col;
        buf[idx] = ' ';
        dirty[idx] = true;
    }
}

void terminal_erase_line() {
    uint16_t row_start = cursor_row * TERM_COLS;
    for (uint8_t col = 0; col < TERM_COLS; col++) {
        uint16_t idx = row_start + col;
        buf[idx] = ' ';
        dirty[idx] = true;
    }
}

void terminal_set_insert_mode(bool insert) {
    insert_mode = insert;
}

uint8_t terminal_get_cursor_col() {
    return cursor_col;
}

uint8_t terminal_get_cursor_row() {
    return cursor_row;
}
