#include "demo.h"
#include "display.h"
#include "terminal.h"
#include <Arduino.h>
#include <avr/pgmspace.h>

#define PIN_BUTTON       PC5
#define LONG_PRESS_MS    1000
#define DEBOUNCE_MS      50
#define EFFECT_MIN_MS    5000
#define EFFECT_MAX_MS    10000
#define CELLS_PER_TICK   6

enum DemoEffect {
    EFFECT_TYPEWRITER,
    EFFECT_TOP_DOWN,
    EFFECT_BOTTOM_UP,
    EFFECT_LEFT_RIGHT,
    EFFECT_RIGHT_LEFT,
    EFFECT_DIAGONAL,
    EFFECT_RANDOM,
    EFFECT_COUNT
};

enum DeviceMode {
    MODE_TERMINAL,
    MODE_DEMO
};

enum DiagDir {
    DIAG_TL, DIAG_TR, DIAG_BL, DIAG_BR
};

static const char PROGMEM txt_vfd[] =
    "    ALPHANUMERIC  PLASMA     \n"
    "       DISPLAY  32x8         \n"
    "      VFD  TERMINAL  v2      \n"
    "  SPI  +  74HC595N  SHIFT    \n"
    "  ATMEGA328P  @  16 MHZ      \n"
    "  VT100  ESC  COMPATIBLE     \n"
    "  POWERED  BY  ARDUINO NANO  \n"
    "     DEMO  MODE  ACTIVE      ";

static const char PROGMEM txt_hacker[] =
    "  WE ARE THE CHILDREN OF THE \n"
    "       DIGITAL  AGE          \n"
    "  WE SPEAK IN CODE           \n"
    "  WE DREAM IN PIXELS         \n"
    "  WE BUILD THE FUTURE        \n"
    "  ONE BYTE AT A TIME         \n"
    "   <  MAKE  IT  GLOW  >      \n"
    "   *** HACKERS UNITE! ***    ";

static const char PROGMEM txt_retro[] =
    "VFD TERMINAL BOOT V2.0\n"
    "COPYRIGHT (C) 2025\n"
    "OKAYA LABS 325552-01\n"
    "\n"
    "CPU: ATMEGA328P 16MHZ\n"
    "TESTING MAIN MEMORY...\n"
    "   RAM: 1024K OK\n"
    "   RAM: 2048K OK\n"
    "MEMORY TEST COMPLETE\n"
    "\n"
    "INITIALIZING DISPLAY...\n"
    "   VFD: 32 COL X 8 ROW\n"
    "   BRIGHTNESS: 80%\n"
    "DISPLAY: OK\n"
    "\n"
    "INITIALIZING PERIPHERALS..\n"
    "   SPI: 74HC595 DETECTED\n"
    "   UART: 38400 8N1\n"
    "   TIMER: 16MHZ / 64\n"
    "PERIPHERALS: OK\n"
    "\n"
    "LOADING ESC/PARSER...\n"
    "   VT100 SUBSET V2.0\n"
    "   CURSOR CONTROL\n"
    "   ERASE FUNCTIONS\n"
    "ESC/PARSER: OK\n"
    "\n"
    "STARTING TERMINAL...\n"
    "   BUFFER: 32x8 (256B)\n"
    "   FLUSH: 33MS INTERVAL\n"
    "TERMINAL: OK\n"
    "\n"
    "CHECKING FILESYSTEM...\n"
    "   FLASH: 30720 BYTES\n"
    "   FREE: 23006 BYTES\n"
    "FILESYSTEM: OK\n"
    "\n"
    "INIT NETWORK STACK...\n"
    "   NO ETHERNET HARDWARE\n"
    "NETWORK: SKIP\n"
    "\n"
    "ALL SYSTEMS NOMINAL\n"
    "READY FOR INPUT\n"
    "\n"
    "C:\\>_";

static const char PROGMEM txt_ascii[] =
    " ### #   #  ### #   #  ### \n"
    "#   ##  # #   #  # # #   #\n"
    "#   ####  #   #   #  #   #\n"
    "#   ##  ######    #  #####\n"
    "#   ##   ##   #   #  #   #\n"
    "#   ##   ##   #   #  #   #\n"
    " ### #   ##   #   #  #   #\n"
    "       OKAYA VFD TERMINAL       ";

static const char PROGMEM txt_specs[] =
    "  === TECHNICAL SPECS ===\n"
    "DISPLAY: VFD 32x8 CHARS\n"
    "BRIGHTNESS: 700 cd/m^2\n"
    "IFACE: SPI VIA 74HC595\n"
    "MCU: ATMEGA328P 16MHZ\n"
    "BAUD: 38400 8N1 SERIAL\n"
    "PROTO: VT100 SUBSET\n"
    "BUILD: OKAYA LABS 2025";

static const char PROGMEM txt_motd[] =
    "       * MOTD *\n"
    "ALL YOUR BASE ARE BELONG\n"
    "       TO  US\n"
    "  [ SYSTEM  READY ]\n"
    "THERE IS NO SPOON\n"
    "  FOLLOW THE WHITE RABBIT\n"
    "> WAKE UP, NEO...\n"
    "  THE MATRIX HAS YOU...";

static const char PROGMEM txt_hello[] =
    "        HELLO !\n"
    "I AM  VFD  TERMINAL\n"
    "32x8  PLASMA  DISPLAY\n"
    "I CAN  SHOW  ANY  TEXT\n"
    "ASCII  ART  IMAGES\n"
    "AND  EVEN  ANIMATIONS\n"
    "  ASK  ME  ANYTHING\n"
    "      [ PRESS  BTN ]";

static const char* const text_table[] PROGMEM = {
    txt_vfd, txt_hacker, txt_retro, txt_ascii,
    txt_specs, txt_motd, txt_hello
};
#define TEXT_COUNT ((uint8_t)(sizeof(text_table) / sizeof(text_table[0])))

static uint8_t screen_buf[DISP_SIZE];
static DeviceMode mode = MODE_TERMINAL;

static bool btn_pressed = false;
static bool btn_long_done = false;
static unsigned long btn_time = 0;

static DemoEffect effect = EFFECT_TYPEWRITER;
static uint8_t text_idx = 0;
static unsigned long effect_start = 0;
static unsigned long next_switch = 0;

static uint16_t reveal_count = 0;
static uint8_t diag_dist = 0;
static uint8_t diag_cell = 0;
static uint8_t diag_dir = DIAG_TL;
static uint8_t rand_visited[32];

static uint16_t tw_offset = 0;
static bool tw_done = false;
static unsigned long tw_last_ms = 0;
static uint16_t tw_delay_ms = 0;

static void demo_load_text(uint8_t idx);
void demo_enter();
void demo_exit();
static void demo_next_effect();
static void demo_switch_text_and_effect();
static void handle_button();

void demo_setup() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    randomSeed(analogRead(A7));
}

bool demo_is_active() {
    return mode == MODE_DEMO;
}

void demo_enter() {
    terminal_erase_all();
    terminal_flush();
    display_clear();
    mode = MODE_DEMO;
    text_idx = random(0, TEXT_COUNT);
    effect = (DemoEffect)random(0, EFFECT_COUNT);
    effect_start = millis();
    next_switch = millis() + random(EFFECT_MIN_MS, EFFECT_MAX_MS);
    reveal_count = 0;
    diag_dist = 0;
    diag_cell = 0;
    diag_dir = (uint8_t)random(0, 4);
    tw_offset = 0;
    tw_done = false;
    tw_last_ms = 0;
    tw_delay_ms = 0;
    memset(rand_visited, 0, sizeof(rand_visited));
    demo_load_text(text_idx);
}

void demo_exit() {
    mode = MODE_TERMINAL;
    terminal_reset();
}

static void demo_next_effect() {
    effect = (DemoEffect)random(0, EFFECT_COUNT);
    display_clear();
    reveal_count = 0;
    diag_dist = 0;
    diag_cell = 0;
    diag_dir = (uint8_t)random(0, 4);
    tw_offset = 0;
    tw_done = false;
    tw_last_ms = 0;
    tw_delay_ms = 0;
    memset(rand_visited, 0, sizeof(rand_visited));
    effect_start = millis();
    next_switch = millis() + random(EFFECT_MIN_MS, EFFECT_MAX_MS);
    demo_load_text(text_idx);
}

static void demo_switch_text_and_effect() {
    text_idx = random(0, TEXT_COUNT);
    demo_next_effect();
}

static void demo_load_text(uint8_t idx) {
    memset(screen_buf, ' ', DISP_SIZE);
    const char* p = (const char*)pgm_read_word(&text_table[idx]);
    uint8_t row = 0;
    uint8_t col = 0;
    char c;
    while ((c = pgm_read_byte(p++)) != 0) {
        if (c == '\n') {
            row++;
            col = 0;
            if (row >= DISP_ROWS) break;
            continue;
        }
        if (c == '\r') continue;
        if (c < 0x20) continue;
        if (col < DISP_COLS) {
            screen_buf[col + row * DISP_COLS] = c;
            col++;
        }
    }
    terminal_erase_all();
}

static void handle_button() {
    bool pressed = !(PINC & (1 << PIN_BUTTON));

    if (pressed) {
        if (!btn_pressed) {
            btn_time = millis();
            btn_pressed = true;
            btn_long_done = false;
        } else if (!btn_long_done && (millis() - btn_time) >= LONG_PRESS_MS) {
            btn_long_done = true;
            if (mode == MODE_TERMINAL) {
                demo_enter();
            } else {
                demo_exit();
            }
        }
    } else {
        if (btn_pressed) {
            if (!btn_long_done && (millis() - btn_time) >= DEBOUNCE_MS) {
                if (mode == MODE_DEMO) {
                    demo_switch_text_and_effect();
                }
            }
            btn_pressed = false;
        }
    }
}

static bool tw_tick() {
    if (tw_done) return false;

    unsigned long now = millis();

    if (tw_last_ms == 0) {
        tw_last_ms = now;
        tw_delay_ms = random(100, 401);
        return true;
    }

    if ((now - tw_last_ms) < tw_delay_ms) return true;

    uint16_t p = pgm_read_word(&text_table[text_idx]);

    while (true) {
        char c = pgm_read_byte(p + tw_offset);
        if (c == 0) {
            tw_done = true;
            terminal_flush();
            return false;
        }
        tw_offset++;
        if (c == '\r') continue;
        terminal_putchar(c);
        if (c == '\n') break;
    }

    terminal_flush();
    tw_last_ms = millis();
    tw_delay_ms = random(100, 401);
    return true;
}

static bool td_tick() {
    uint8_t written = 0;
    while (written < CELLS_PER_TICK && reveal_count < DISP_SIZE) {
        display_write_raw((uint8_t)reveal_count, screen_buf[reveal_count]);
        reveal_count++;
        written++;
    }
    return reveal_count < DISP_SIZE;
}

static bool bu_tick() {
    uint8_t written = 0;
    while (written < CELLS_PER_TICK && reveal_count < DISP_SIZE) {
        uint16_t addr = (uint16_t)(DISP_SIZE - 1 - reveal_count);
        display_write_raw((uint8_t)addr, screen_buf[addr]);
        reveal_count++;
        written++;
    }
    return reveal_count < DISP_SIZE;
}

static bool lr_tick() {
    uint8_t written = 0;
    while (written < CELLS_PER_TICK && reveal_count < DISP_SIZE) {
        uint8_t col = reveal_count / DISP_ROWS;
        uint8_t row = reveal_count % DISP_ROWS;
        display_write_raw(col + row * DISP_COLS, screen_buf[col + row * DISP_COLS]);
        reveal_count++;
        written++;
    }
    return reveal_count < DISP_SIZE;
}

static bool rl_tick() {
    uint8_t written = 0;
    while (written < CELLS_PER_TICK && reveal_count < DISP_SIZE) {
        uint8_t col = (uint8_t)(DISP_COLS - 1 - reveal_count / DISP_ROWS);
        uint8_t row = reveal_count % DISP_ROWS;
        display_write_raw(col + row * DISP_COLS, screen_buf[col + row * DISP_COLS]);
        reveal_count++;
        written++;
    }
    return reveal_count < DISP_SIZE;
}

static bool diag_tick() {
    uint8_t written = 0;
    uint8_t max_dist = DISP_ROWS + DISP_COLS - 2;

    while (written < CELLS_PER_TICK && reveal_count < DISP_SIZE) {
        if (diag_dist > max_dist) return false;

        bool found = false;
        while (diag_cell < DISP_SIZE) {
            uint8_t row = diag_cell / DISP_COLS;
            uint8_t col = diag_cell % DISP_COLS;

            uint8_t dist;
            switch (diag_dir) {
                default:
                case DIAG_TL: dist = row + col; break;
                case DIAG_TR: dist = row + (uint8_t)(DISP_COLS - 1 - col); break;
                case DIAG_BL: dist = (uint8_t)(DISP_ROWS - 1 - row) + col; break;
                case DIAG_BR: dist = (uint8_t)(DISP_ROWS - 1 - row) + (uint8_t)(DISP_COLS - 1 - col); break;
            }

            diag_cell++;

            if (dist == diag_dist) {
                uint16_t addr = col + row * DISP_COLS;
                display_write_raw((uint8_t)addr, screen_buf[addr]);
                reveal_count++;
                written++;
                found = true;
                break;
            }
        }

        if (!found) {
            diag_dist++;
            diag_cell = 0;
        }
    }
    return reveal_count < DISP_SIZE;
}

static bool rnd_tick() {
    uint8_t written = 0;
    uint8_t attempts = 0;
    while (written < CELLS_PER_TICK && reveal_count < DISP_SIZE) {
        uint8_t idx = (uint8_t)random(0, DISP_SIZE);
        uint8_t byte_idx = idx >> 3;
        uint8_t bit_idx = idx & 0x07;
        if (!(rand_visited[byte_idx] & (1 << bit_idx))) {
            rand_visited[byte_idx] |= (1 << bit_idx);
            display_write_raw(idx, screen_buf[idx]);
            reveal_count++;
            written++;
            attempts = 0;
        } else {
            attempts++;
            if (attempts > 50) {
                for (uint16_t i = 0; i < DISP_SIZE; i++) {
                    uint8_t bi = (uint8_t)(i >> 3);
                    uint8_t bb = i & 0x07;
                    if (!(rand_visited[bi] & (1 << bb))) {
                        rand_visited[bi] |= (1 << bb);
                        display_write_raw((uint8_t)i, screen_buf[i]);
                        reveal_count++;
                        written++;
                        break;
                    }
                }
                attempts = 0;
            }
        }
    }
    return reveal_count < DISP_SIZE;
}

static bool effect_is_done() {
    if (effect == EFFECT_TYPEWRITER) return tw_done;
    return reveal_count >= DISP_SIZE;
}

static void effect_tick() {
    unsigned long now = millis();

    if (effect_is_done() && now >= next_switch) {
        demo_switch_text_and_effect();
        return;
    }

    bool in_progress;
    switch (effect) {
        case EFFECT_TYPEWRITER: in_progress = tw_tick(); break;
        case EFFECT_TOP_DOWN:   in_progress = td_tick(); break;
        case EFFECT_BOTTOM_UP:  in_progress = bu_tick(); break;
        case EFFECT_LEFT_RIGHT: in_progress = lr_tick(); break;
        case EFFECT_RIGHT_LEFT: in_progress = rl_tick(); break;
        case EFFECT_RANDOM:     in_progress = rnd_tick(); break;
        case EFFECT_DIAGONAL:   in_progress = diag_tick(); break;
        default:                in_progress = td_tick(); break;
    }

    if (!in_progress && !effect_is_done()) {
        reveal_count = DISP_SIZE;
    }
}

void demo_tick() {
    handle_button();
    if (mode == MODE_DEMO) {
        effect_tick();
    }
}
