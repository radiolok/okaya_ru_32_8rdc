#include "esc_parser.h"
#include "terminal.h"
#include "demo.h"

enum EscState {
    STATE_NORMAL,
    STATE_ESC,
    STATE_CSI_PARAM
};

static EscState state = STATE_NORMAL;
static char param_buf[8];
static uint8_t param_idx = 0;
static int16_t params[2] = {0, 0};
static uint8_t num_params = 0;
static bool question_mark = false;

void esc_parser_reset() {
    state = STATE_NORMAL;
    param_idx = 0;
    num_params = 0;
    params[0] = 0;
    params[1] = 0;
    question_mark = false;
}

static void parse_params() {
    param_buf[param_idx] = '\0';
    num_params = 0;
    params[0] = 0;
    params[1] = 0;

    if (param_idx == 0) return;

    // Values saturate at 255: larger counts/coordinates are clamped anyway.
    const char *p = param_buf;
    for (uint8_t i = 0; i < 2; i++) {
        while (*p >= '0' && *p <= '9') {
            params[i] = params[i] * 10 + (*p - '0');
            if (params[i] > 255) params[i] = 255;
            p++;
        }
        num_params = i + 1;
        if (*p != ';') break;
        p++;
    }
}

static void exec_csi(char final) {
    parse_params();

    // While the demo owns the display, only the demo-exit sequence is honoured.
    if (demo_is_active()) {
        if (question_mark && final == 'l' && params[0] == 101) demo_exit();
        return;
    }

    int16_t n = (num_params >= 1 && params[0] > 0) ? params[0] : 1;

    switch (final) {
        case 'H':
        case 'f':
            terminal_set_cursor(
                (params[1] > 0) ? (uint8_t)(params[1] - 1) : 0,
                (params[0] > 0) ? (uint8_t)(params[0] - 1) : 0
            );
            break;

        case 'A':
            terminal_move_cursor_up((uint8_t)n);
            break;

        case 'B':
            terminal_move_cursor_down((uint8_t)n);
            break;

        case 'C':
            terminal_move_cursor_right((uint8_t)n);
            break;

        case 'D':
            terminal_move_cursor_left((uint8_t)n);
            break;

        case 'J':
            if (!question_mark) {
                if (params[0] == 0 || num_params == 0) {
                    terminal_erase_from_cursor_to_end();
                } else if (params[0] == 1) {
                    terminal_erase_from_begin_to_cursor();
                } else if (params[0] == 2) {
                    terminal_erase_all();
                }
            }
            break;

        case 'K':
            if (!question_mark) {
                if (params[0] == 0 || num_params == 0) {
                    terminal_erase_line_from_cursor_to_end();
                } else if (params[0] == 1) {
                    terminal_erase_line_from_begin_to_cursor();
                } else if (params[0] == 2) {
                    terminal_erase_line();
                }
            }
            break;

        case 'h':
            if (question_mark) {
                if (params[0] == 4) terminal_set_scroll_enabled(true);
                else if (params[0] == 100) terminal_set_echo_enabled(true);
                else if (params[0] == 101) { if (!demo_is_active()) demo_enter(); }
            } else {
                if (params[0] == 4) terminal_set_insert_mode(true);
            }
            break;

        case 'l':
            if (question_mark) {
                if (params[0] == 4) terminal_set_scroll_enabled(false);
                else if (params[0] == 100) terminal_set_echo_enabled(false);
                else if (params[0] == 101) { if (demo_is_active()) demo_exit(); }
            } else {
                if (params[0] == 4) terminal_set_insert_mode(false);
            }
            break;

        default:
            break;
    }
}

bool esc_parser_feed(char c) {
    switch (state) {
        case STATE_NORMAL:
            if (c == 0x1B) {
                state = STATE_ESC;
                return false;
            }
            if (demo_is_active()) return false;
            terminal_putchar(c);
            return true;

        case STATE_ESC:
            if (c == '[') {
                state = STATE_CSI_PARAM;
                param_idx = 0;
                num_params = 0;
                params[0] = 0;
                params[1] = 0;
                question_mark = false;
                return false;
            }
            if (c == 0x1B) {
                return false;
            }
            if (c == 'c' && !demo_is_active()) {
                terminal_reset();
            }
            // Unsupported two-byte sequences (ESC 7, ESC D, ...) are discarded.
            state = STATE_NORMAL;
            return false;

        case STATE_CSI_PARAM:
            if (c >= '0' && c <= '9') {
                if (param_idx < sizeof(param_buf) - 1) {
                    param_buf[param_idx++] = c;
                }
                return false;
            }
            if (c == ';') {
                if (param_idx < sizeof(param_buf) - 1) {
                    param_buf[param_idx++] = c;
                }
                return false;
            }
            if (c == '?') {
                question_mark = true;
                return false;
            }
            if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
                exec_csi(c);
                state = STATE_NORMAL;
                return false;
            }
            state = STATE_NORMAL;
            return false;

        default:
            state = STATE_NORMAL;
            return false;
    }
}
