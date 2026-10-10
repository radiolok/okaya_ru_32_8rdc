#include <unity.h>
#include <limits.h>
#include <stdio.h>
#include <string>
#include "fakes.h"
#include "terminal.h"
#include "esc_parser.h"
#include "demo.h"

// demo.cpp effect order
enum { FX_TYPEWRITER, FX_TOP_DOWN, FX_BOTTOM_UP, FX_LEFT_RIGHT, FX_RIGHT_LEFT, FX_DIAGONAL, FX_RANDOM };

static std::string pad(const char *s) {
    std::string r(s);
    r.resize(DISP_COLS, ' ');
    return r;
}

static void feed(const char *s) {
    while (*s) esc_parser_feed(*s++);
}

static void feed_flush(const char *s) {
    feed(s);
    terminal_flush();
}

static void assert_row(uint8_t row, const char *expected) {
    TEST_ASSERT_EQUAL_STRING(pad(expected).c_str(), fake_row(row).c_str());
}

static void assert_cursor(uint8_t col, uint8_t row) {
    TEST_ASSERT_EQUAL_UINT8(col, terminal_get_cursor_col());
    TEST_ASSERT_EQUAL_UINT8(row, terminal_get_cursor_row());
}

void setUp() {
    fake_reset();
    if (demo_is_active()) demo_exit();
    terminal_set_scroll_enabled(true);
    terminal_set_echo_enabled(false);
    terminal_init();
    terminal_flush();
    esc_parser_reset();
    fake_reset();
}

void tearDown() {}

// --- terminal --------------------------------------------------------------

void test_print_and_flush() {
    feed_flush("HELLO");
    assert_row(0, "HELLO");
    assert_cursor(5, 0);
}

void test_flush_writes_only_dirty_cells() {
    feed_flush("AB");
    TEST_ASSERT_EQUAL_UINT32(2, fake_total_writes);
    terminal_flush();
    TEST_ASSERT_EQUAL_UINT32(2, fake_total_writes);
}

void test_wraps_at_end_of_line() {
    feed_flush("0123456789ABCDEFGHIJKLMNOPQRSTUVWX");
    assert_row(0, "0123456789ABCDEFGHIJKLMNOPQRSTUV");
    assert_row(1, "WX");
    assert_cursor(2, 1);
}

void test_scrolls_when_full() {
    feed_flush("L0\nL1\nL2\nL3\nL4\nL5\nL6\nL7\nL8");
    assert_row(0, "L1");
    assert_row(6, "L7");
    assert_row(7, "L8");
}

void test_no_scroll_overwrites_last_row() {
    feed("\x1b[?4l");
    feed_flush("L0\nL1\nL2\nL3\nL4\nL5\nL6\nL7\nL8");
    assert_row(0, "L0");
    assert_row(7, "L8");
}

void test_crlf_and_lfcr_are_one_newline() {
    feed_flush("A\r\nB\n\rC");
    assert_row(0, "A");
    assert_row(1, "B");
    assert_row(2, "C");
}

void test_double_lf_makes_blank_line() {
    feed_flush("A\n\nB");
    assert_row(0, "A");
    assert_row(1, "");
    assert_row(2, "B");
}

void test_double_crlf_makes_blank_line() {
    feed_flush("A\r\n\r\nB");
    assert_row(1, "");
    assert_row(2, "B");
}

void test_backspace_moves_back_across_lines() {
    feed("\x1b[2;1H\b");
    assert_cursor(31, 0);
    feed("\b");
    assert_cursor(30, 0);
}

void test_tab_to_next_multiple_of_four() {
    feed_flush("A\tB");
    assert_row(0, "A   B");
    assert_cursor(5, 0);
}

void test_control_chars_are_ignored() {
    feed_flush("A\x07" "B");
    assert_row(0, "AB");
}

void test_insert_mode_shifts_right() {
    feed("ABC\x1b[1;1H\x1b[4h");
    feed_flush("X");
    assert_row(0, "XABC");
    feed("\x1b[4l\x1b[1;1H");
    feed_flush("Y");
    assert_row(0, "YABC");
}

void test_cursor_move_clamps() {
    terminal_set_cursor(5, 7);
    terminal_move_cursor_down(250);
    assert_cursor(5, 7);
    terminal_set_cursor(31, 3);
    terminal_move_cursor_right(230);
    assert_cursor(31, 3);
    terminal_move_cursor_up(200);
    terminal_move_cursor_left(200);
    assert_cursor(0, 0);
    terminal_move_cursor_down(0);
    assert_cursor(0, 1);
}

void test_set_cursor_clamps() {
    terminal_set_cursor(200, 200);
    assert_cursor(31, 7);
}

void test_echo_mirrors_output() {
    feed("\x1b[?100h");
    feed("AB\r\nC");
    TEST_ASSERT_EQUAL_STRING("AB\r\nC", fake_serial_tx.c_str());
    feed("\x1b[?100l");
    feed("D");
    TEST_ASSERT_EQUAL_STRING("AB\r\nC", fake_serial_tx.c_str());
}

// --- ESC parser ------------------------------------------------------------

void test_csi_cursor_position() {
    feed("\x1b[3;5H");
    assert_cursor(4, 2);
    feed("\x1b[H");
    assert_cursor(0, 0);
    feed("\x1b[8;32f");
    assert_cursor(31, 7);
}

void test_csi_cursor_position_partial_params() {
    feed("\x1b[5;H");
    assert_cursor(0, 4);
    feed("\x1b[;7H");
    assert_cursor(6, 0);
    feed("\x1b[4H");
    assert_cursor(0, 3);
}

void test_csi_cursor_moves() {
    feed("\x1b[4;10H\x1b[A");
    assert_cursor(9, 2);
    feed("\x1b[2B");
    assert_cursor(9, 4);
    feed("\x1b[3C");
    assert_cursor(12, 4);
    feed("\x1b[5D");
    assert_cursor(7, 4);
}

void test_csi_large_counts_clamp() {
    feed("\x1b[256B");
    assert_cursor(0, 7);
    feed("\x1b[300C");
    assert_cursor(31, 7);
    feed("\x1b[9999999A");
    assert_cursor(31, 0);
}

void test_csi_erase_display() {
    feed_flush("AAAA\nBBBB\nCCCC");
    feed_flush("\x1b[2;3H\x1b[J");
    assert_row(0, "AAAA");
    assert_row(1, "BB");
    assert_row(2, "");

    feed_flush("\x1b[1;1H\x1b[2J");
    for (uint8_t r = 0; r < DISP_ROWS; r++) assert_row(r, "");
    assert_cursor(0, 0);
}

void test_csi_erase_display_to_cursor() {
    feed_flush("AAAA\nBBBB\nCCCC");
    feed_flush("\x1b[2;2H\x1b[1J");
    assert_row(0, "");
    assert_row(1, "  BB");
    assert_row(2, "CCCC");
}

void test_csi_erase_line() {
    feed_flush("ABCDEF\x1b[1;3H\x1b[K");
    assert_row(0, "AB");
    feed_flush("\x1b[1;1HABCDEF\x1b[1;3H\x1b[1K");
    assert_row(0, "   DEF");
    feed_flush("\x1b[2K");
    assert_row(0, "");
}

void test_esc_c_resets_terminal() {
    feed("HELLO\x1b[4h");
    feed("\x1b" "c");
    for (uint8_t r = 0; r < DISP_ROWS; r++) assert_row(r, "");
    assert_cursor(0, 0);
    feed_flush("AB\x1b[1;1HX");
    assert_row(0, "XB");  // insert mode was cleared
}

void test_unknown_escape_is_discarded() {
    feed_flush("A\x1b" "7B\x1b" "DC");
    assert_row(0, "ABC");
}

void test_esc_esc_restarts_sequence() {
    feed_flush("\x1b\x1b[2;2HX");
    assert_row(1, " X");
}

void test_invalid_csi_byte_aborts_sequence() {
    feed_flush("\x1b[2!AB");
    assert_row(0, "AB");
}

void test_param_buffer_overflow_is_safe() {
    // Digits beyond the 7-byte buffer are dropped; the row saturates at 255.
    feed("\x1b[12345678901234567890;5H");
    assert_cursor(0, 7);
    feed_flush("Z");
    assert_row(7, "Z");
}

void test_feed_return_value() {
    TEST_ASSERT_TRUE(esc_parser_feed('A'));
    TEST_ASSERT_FALSE(esc_parser_feed('\x1b'));
    TEST_ASSERT_FALSE(esc_parser_feed('['));
    TEST_ASSERT_FALSE(esc_parser_feed('H'));
}

// --- demo ------------------------------------------------------------------

static const char *const VFD_ROWS[DISP_ROWS] = {
    "    ALPHANUMERIC  PLASMA     ",
    "       DISPLAY  32x8         ",
    "      VFD  TERMINAL  v2      ",
    "  SPI  +  74HC595N  SHIFT    ",
    "  ATMEGA328P  @  16 MHZ      ",
    "  VT100  ESC  COMPATIBLE     ",
    "  POWERED  BY  ARDUINO NANO  ",
    "     DEMO  MODE  ACTIVE      ",
};

static void enter_demo(long text, long effect, long diag_dir = 0) {
    fake_random_clear();
    fake_random_push(text);
    fake_random_push(effect);
    fake_random_push(9999);  // effect duration (ms)
    fake_random_push(diag_dir);
    demo_enter();
}

static void tick(unsigned n, unsigned long step_ms = 0) {
    for (unsigned i = 0; i < n; i++) {
        fake_millis += step_ms;
        demo_tick();
    }
}

static void assert_vfd_screen() {
    for (uint8_t r = 0; r < DISP_ROWS; r++) assert_row(r, VFD_ROWS[r]);
}

static void assert_each_cell_written_once() {
    for (uint16_t i = 0; i < DISP_SIZE; i++) {
        char msg[32];
        snprintf(msg, sizeof(msg), "addr %u", (unsigned)i);
        TEST_ASSERT_EQUAL_UINT16_MESSAGE(1, fake_write_count[i], msg);
    }
}

static void check_reveal_effect(long effect, long diag_dir = 0) {
    setUp();
    enter_demo(0, effect, diag_dir);
    TEST_ASSERT_TRUE(demo_is_active());
    tick(200);
    assert_vfd_screen();
    assert_each_cell_written_once();
    TEST_ASSERT_EQUAL_UINT32(1, fake_clear_count);  // no premature switch
}

void test_demo_top_down() { check_reveal_effect(FX_TOP_DOWN); }
void test_demo_bottom_up() { check_reveal_effect(FX_BOTTOM_UP); }
void test_demo_left_right() { check_reveal_effect(FX_LEFT_RIGHT); }
void test_demo_right_left() { check_reveal_effect(FX_RIGHT_LEFT); }
void test_demo_random() { check_reveal_effect(FX_RANDOM); }

void test_demo_diagonal_all_corners() {
    for (long dir = 0; dir < 4; dir++) check_reveal_effect(FX_DIAGONAL, dir);
}

void test_demo_typewriter() {
    enter_demo(0, FX_TYPEWRITER);
    tick(20, 401);
    assert_vfd_screen();
}

void test_demo_switches_after_timeout() {
    enter_demo(0, FX_TOP_DOWN);
    tick(100);
    TEST_ASSERT_EQUAL_UINT32(1, fake_clear_count);
    fake_millis += 10000;
    tick(1);
    TEST_ASSERT_EQUAL_UINT32(2, fake_clear_count);
}

void test_demo_switch_survives_millis_wrap() {
    fake_millis = ULONG_MAX - 2000;
    enter_demo(0, FX_TOP_DOWN);  // next_switch wraps past zero
    tick(100);
    TEST_ASSERT_EQUAL_UINT32(1, fake_clear_count);
    fake_millis += 10000;
    tick(1);
    TEST_ASSERT_EQUAL_UINT32(2, fake_clear_count);
}

void test_demo_setup_configures_button_pin_only() {
    DDRD = 0xFF;
    PORTD = 0x00;
    DDRC = 0xFF;
    demo_setup();
    TEST_ASSERT_EQUAL_HEX8(0xFF, DDRD);
    TEST_ASSERT_EQUAL_HEX8(0x00, PORTD);
    TEST_ASSERT_EQUAL_HEX8(0xFF & ~(1 << PC5), DDRC);
    TEST_ASSERT_TRUE(PORTC & (1 << PC5));
}

void test_button_long_press_toggles_mode() {
    PINC &= ~(1 << PC5);
    tick(1);
    tick(1, 500);
    TEST_ASSERT_FALSE(demo_is_active());
    tick(1, 600);
    TEST_ASSERT_TRUE(demo_is_active());
    tick(5, 600);  // holding longer does not toggle again
    TEST_ASSERT_TRUE(demo_is_active());
    PINC |= (1 << PC5);
    tick(1);
    TEST_ASSERT_TRUE(demo_is_active());

    PINC &= ~(1 << PC5);
    tick(1);
    tick(1, 1100);
    TEST_ASSERT_FALSE(demo_is_active());
    PINC |= (1 << PC5);
    tick(1);
}

void test_button_short_press_switches_effect() {
    enter_demo(0, FX_TOP_DOWN);
    uint32_t clears = fake_clear_count;
    PINC &= ~(1 << PC5);
    tick(1);
    tick(1, 100);
    PINC |= (1 << PC5);
    tick(1);
    TEST_ASSERT_EQUAL_UINT32(clears + 1, fake_clear_count);
}

void test_button_bounce_is_ignored() {
    enter_demo(0, FX_TOP_DOWN);
    uint32_t clears = fake_clear_count;
    PINC &= ~(1 << PC5);
    tick(1);
    tick(1, 10);
    PINC |= (1 << PC5);
    tick(1);
    TEST_ASSERT_EQUAL_UINT32(clears, fake_clear_count);
}

void test_remote_demo_enter_and_exit() {
    feed("\x1b[?101h");
    TEST_ASSERT_TRUE(demo_is_active());
    // Text and other sequences are ignored while the demo owns the display.
    feed("HELLO\x1b[5;5H\x1b[?101h");
    assert_cursor(0, 0);
    TEST_ASSERT_TRUE(demo_is_active());
    feed("\x1b[?101l");
    TEST_ASSERT_FALSE(demo_is_active());
    feed_flush("OK");
    assert_row(0, "OK");
}

// --- display helpers -------------------------------------------------------

void test_display_map_char_folds_lowercase() {
    TEST_ASSERT_EQUAL_UINT8('A', display_map_char('a'));
    TEST_ASSERT_EQUAL_UINT8('Z', display_map_char('z'));
    TEST_ASSERT_EQUAL_UINT8('X', display_map_char('x'));
    TEST_ASSERT_EQUAL_UINT8('A', display_map_char('A'));
    TEST_ASSERT_EQUAL_UINT8('0', display_map_char('0'));
    TEST_ASSERT_EQUAL_UINT8('`', display_map_char('`'));
    TEST_ASSERT_EQUAL_UINT8('{', display_map_char('{'));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_print_and_flush);
    RUN_TEST(test_flush_writes_only_dirty_cells);
    RUN_TEST(test_wraps_at_end_of_line);
    RUN_TEST(test_scrolls_when_full);
    RUN_TEST(test_no_scroll_overwrites_last_row);
    RUN_TEST(test_crlf_and_lfcr_are_one_newline);
    RUN_TEST(test_double_lf_makes_blank_line);
    RUN_TEST(test_double_crlf_makes_blank_line);
    RUN_TEST(test_backspace_moves_back_across_lines);
    RUN_TEST(test_tab_to_next_multiple_of_four);
    RUN_TEST(test_control_chars_are_ignored);
    RUN_TEST(test_insert_mode_shifts_right);
    RUN_TEST(test_cursor_move_clamps);
    RUN_TEST(test_set_cursor_clamps);
    RUN_TEST(test_echo_mirrors_output);

    RUN_TEST(test_csi_cursor_position);
    RUN_TEST(test_csi_cursor_position_partial_params);
    RUN_TEST(test_csi_cursor_moves);
    RUN_TEST(test_csi_large_counts_clamp);
    RUN_TEST(test_csi_erase_display);
    RUN_TEST(test_csi_erase_display_to_cursor);
    RUN_TEST(test_csi_erase_line);
    RUN_TEST(test_esc_c_resets_terminal);
    RUN_TEST(test_unknown_escape_is_discarded);
    RUN_TEST(test_esc_esc_restarts_sequence);
    RUN_TEST(test_invalid_csi_byte_aborts_sequence);
    RUN_TEST(test_param_buffer_overflow_is_safe);
    RUN_TEST(test_feed_return_value);

    RUN_TEST(test_demo_top_down);
    RUN_TEST(test_demo_bottom_up);
    RUN_TEST(test_demo_left_right);
    RUN_TEST(test_demo_right_left);
    RUN_TEST(test_demo_random);
    RUN_TEST(test_demo_diagonal_all_corners);
    RUN_TEST(test_demo_typewriter);
    RUN_TEST(test_demo_switches_after_timeout);
    RUN_TEST(test_demo_switch_survives_millis_wrap);
    RUN_TEST(test_demo_setup_configures_button_pin_only);
    RUN_TEST(test_button_long_press_toggles_mode);
    RUN_TEST(test_button_short_press_switches_effect);
    RUN_TEST(test_button_bounce_is_ignored);
    RUN_TEST(test_remote_demo_enter_and_exit);

    RUN_TEST(test_display_map_char_folds_lowercase);
    return UNITY_END();
}
