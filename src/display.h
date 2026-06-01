#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include "pins.h"

/** @brief Display width in characters. */
#define DISP_COLS 32
/** @brief Display height in characters. */
#define DISP_ROWS 8
/** @brief Total character positions (256). */
#define DISP_SIZE (DISP_COLS * DISP_ROWS)

/** @brief Strobe pulse width in microseconds for control signals. */
#define DISP_STROBE_DELAY_US 2
/** @brief Reset pulse width in microseconds. */
#define DISP_RESET_DELAY_US  100

/** @brief Pulse AW (Address Write): ~AS LOW → delay → HIGH. */
void strobe_aw();
/** @brief Pulse CLOCK (character data latch): NS7 HIGH → delay → LOW. */
void strobe_clock();
/** @brief Set CS (chip select): NS8 LOW = active, HIGH = inactive. */
void set_cs(bool active);
/** @brief Set Underline bit: ~AD LOW = underline ON, HIGH = OFF.
 *  Call BEFORE strobe_clock() to latch the bit with character data. */
void set_underline(bool on);
/** @brief Poll NS4 (READY) until HIGH or timeout.
 *  @return true if ready, false on timeout. */
bool wait_ready();
/** @brief Pulse ~RESET (display RAM reset, active low). */
void strobe_nRESET();
/** @brief Control the blanking signal (~BL).
 *  @param blank true = blank display, false = normal operation.
 */
void set_blank(bool blank);

/** @brief Full display initialization sequence.
 *
 * Order: configure strobe pins → init 595 chain → init HV converter →
 * blank display → reset display RAM → soft-start HV → clear screen → unblank.
 * Blocks during HV soft-start (~1.5–4.5 s).
 */
void display_init();

/** @brief Write a character at the given column and row.
 *
 * Converts (col, row) to a linear address and calls display_write_raw().
 *
 * @param col Column index (0–31).
 * @param row Row index (0–7).
 * @param ch  Character code to write.
 */
void display_write(uint8_t col, uint8_t row, uint8_t ch);

/** @brief Write a character at a raw linear address with the full strobe sequence.
 *
 * Sequence: shift595_write(addr, data) → strobe_delay() → wait_ready() →
 * strobe_aw() → set_underline() → strobe_clock().
 * CS (NS8) is held LOW from display_init(), WR (~WR) is held HIGH.
 *
 * @param addr      Linear display address (0–255).
 * @param data      Character code.
 * @param underline If true, turns on the underline attribute for this character.
 */
void display_write_raw(uint8_t addr, uint8_t data, bool underline = false);

/** @brief Batch write contiguous characters with auto-increment.
 *
 * Writes first character with full sequence (AW + CLOCK),
 * subsequent characters with only CLOCK (address auto-increments).
 *
 * @param addr Starting linear address (0–255).
 * @param data Pointer to character codes.
 * @param len  Number of characters to write.
 */
void display_write_batch(uint8_t addr, const uint8_t *data, uint16_t len);

/** @brief Clear the entire display (fill with spaces).
 *
 * Blanks the display during the fill to prevent flicker.
 * Writes 0x20 (space) to all 256 positions.
 */
void display_clear();

/** @brief Set display brightness via blanking toggle.
 *
 * @param level Brightness level: < 128 blanks the display, >= 128 shows it.
 *              Currently binary only (no PWM).
 */
void display_set_brightness(uint8_t level);

/** @brief Check if the display is busy (stub, always returns false). */
bool display_is_busy();

#endif
