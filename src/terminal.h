#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stdbool.h>

#define TERM_COLS 32
#define TERM_ROWS 8
#define TERM_SIZE (TERM_COLS * TERM_ROWS)

void terminal_init();
void terminal_putchar(char c);
void terminal_flush();
void terminal_reset();
void terminal_set_cursor(uint8_t col, uint8_t row);
void terminal_move_cursor_up(uint8_t n);
void terminal_move_cursor_down(uint8_t n);
void terminal_move_cursor_right(uint8_t n);
void terminal_move_cursor_left(uint8_t n);
void terminal_erase_from_cursor_to_end();
void terminal_erase_from_begin_to_cursor();
void terminal_erase_all();
void terminal_erase_line_from_cursor_to_end();
void terminal_erase_line_from_begin_to_cursor();
void terminal_erase_line();
void terminal_set_insert_mode(bool insert);
uint8_t terminal_get_cursor_col();
uint8_t terminal_get_cursor_row();

#endif
