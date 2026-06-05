#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdint.h>
#include <stdbool.h>

/** @brief Terminal buffer width (columns). */
#define TERM_COLS 32
/** @brief Terminal buffer height (rows). */
#define TERM_ROWS 8
/** @brief Total buffer size in bytes. */
#define TERM_SIZE (TERM_COLS * TERM_ROWS)

/** @brief Initialize the terminal buffer.
 *
 * Clears the 32×8 character buffer (fills with spaces),
 * resets cursor to (0,0), and sets replace mode.
 */
void terminal_init();

/** @brief Process a single character through the terminal state machine.
 *
 * Handles printable characters, control codes (CR, LF, BS, TAB),
 * and maintains the cursor position. Inserts character into the
 * internal buffer at the current cursor position and advances.
 * Scrolls the buffer up when the cursor overflows the last row.
 *
 * @param c Character to process (printable or control).
 */
void terminal_putchar(char c);

/** @brief Write a null-terminated string into the terminal buffer.
 *
 * Calls terminal_putchar() for each character in the string.
 * Does NOT flush — call terminal_flush() afterwards.
 *
 * @param s Null-terminated C string.
 */
void terminal_puts(const char *s);

/** @brief Flush all dirty buffer cells to the display.
 *
 * Iterates over all 256 positions and writes only those marked dirty
 * via display_write_raw(). Clears dirty flags after each write.
 */
void terminal_flush();

/** @brief Full terminal reset: clear buffer, reset cursor, flush display. */
void terminal_reset();

/** @brief Set the cursor to a specific column and row.
 *  @param col Column (0–31), clamped if out of range.
 *  @param row Row (0–7), clamped if out of range.
 */
void terminal_set_cursor(uint8_t col, uint8_t row);

/** @brief Move cursor up by @p n rows (clamped to top).
 *  @param n Number of rows (0 treated as 1).
 */
void terminal_move_cursor_up(uint8_t n);

/** @brief Move cursor down by @p n rows (clamped to bottom).
 *  @param n Number of rows (0 treated as 1).
 */
void terminal_move_cursor_down(uint8_t n);

/** @brief Move cursor right by @p n columns (clamped to right edge).
 *  @param n Number of columns (0 treated as 1).
 */
void terminal_move_cursor_right(uint8_t n);

/** @brief Move cursor left by @p n columns (clamped to left edge).
 *  @param n Number of columns (0 treated as 1).
 */
void terminal_move_cursor_left(uint8_t n);

/** @brief Erase from the current cursor position to the end of the screen. */
void terminal_erase_from_cursor_to_end();

/** @brief Erase from the beginning of the screen to the current cursor position. */
void terminal_erase_from_begin_to_cursor();

/** @brief Erase the entire screen and home the cursor. */
void terminal_erase_all();

/** @brief Erase from the cursor position to the end of the current line. */
void terminal_erase_line_from_cursor_to_end();

/** @brief Erase from the beginning of the current line to the cursor position. */
void terminal_erase_line_from_begin_to_cursor();

/** @brief Erase the entire current line. */
void terminal_erase_line();

/** @brief Set insert mode (characters shift right) or replace mode (overwrite).
 *  @param insert true for insert mode, false for replace mode.
 */
void terminal_set_insert_mode(bool insert);

/** @brief Get current cursor column.
 *  @return Column index (0–31).
 */
uint8_t terminal_get_cursor_col();

/** @brief Get current cursor row.
 *  @return Row index (0–7).
 */
uint8_t terminal_get_cursor_row();

void terminal_set_scroll_enabled(bool scroll_on);

#endif
