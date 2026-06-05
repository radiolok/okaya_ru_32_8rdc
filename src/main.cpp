#include <Arduino.h>
#include "display.h"
#include "terminal.h"
#include "esc_parser.h"
#include "demo.h"

#define FLUSH_INTERVAL_MS  33

void setup() {
    display_init();
    terminal_init();
    esc_parser_reset();
    demo_setup();
    Serial.begin(9600);

#ifdef DEBUG_PATTERN
    terminal_set_cursor(0, 0); terminal_puts("ALPHANUMERIC PLASMA DISPLAY 32X8");
    terminal_set_cursor(0, 1); terminal_puts("SPI CONNECT VIA 74HC595N");
    terminal_set_cursor(3, 2); terminal_puts("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    terminal_set_cursor(2, 3); terminal_puts("@[\\]^_ !\"#$%&'()*+,-./:;<=>?");
    terminal_set_cursor(3, 4); terminal_puts("REGULAR TEXT ONLY");
    terminal_set_cursor(11, 5); terminal_puts("0123456789");
    terminal_set_cursor(0, 6); terminal_puts("WHEN STRING LENGTH MORE THAN 32,TEXT CONTINUES ON THE NEXT LINE");
    terminal_flush();
#endif
}

#ifdef DEBUG_PATTERN
void loop() {
    demo_tick();
}
#else
void loop() {
    demo_tick();

    if (demo_is_active()) return;

    while (Serial.available()) {
        char c = (char)Serial.read();
        esc_parser_feed(c);
    }

    static unsigned long last_flush = 0;
    unsigned long now = millis();
    if (now - last_flush >= FLUSH_INTERVAL_MS) {
        terminal_flush();
        last_flush = now;
    }
}
#endif
