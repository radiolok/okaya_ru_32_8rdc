#ifndef ESC_PARSER_H
#define ESC_PARSER_H

#include <stdint.h>

/** @brief Reset the ESC sequence parser to the initial state.
 *
 * Clears the internal state machine (NORMAL), parameter buffer,
 * and any pending parameters.
 */
void esc_parser_reset();

/** @brief Feed a single byte into the ESC sequence state machine.
 *
 * In NORMAL state, printable characters are forwarded directly to
 * terminal_putchar(). The ESC character (0x1B) begins a sequence.
 * Supports VT100 subset: cursor positioning, movement, screen/line
 * erase, insert/replace mode, and terminal reset.
 *
 * @param c Incoming byte from the serial stream.
 * @return true if the byte was forwarded to the terminal buffer,
 *         false if it was consumed by the parser.
 */
bool esc_parser_feed(char c);

#endif
