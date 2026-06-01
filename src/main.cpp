#include <Arduino.h>
#include "display.h"
#include "terminal.h"
#include "esc_parser.h"

#define FLUSH_INTERVAL_MS  33

void setup() {
    display_init();
    terminal_init();
    esc_parser_reset();
    Serial.begin(115200);

    terminal_set_cursor(0, 0); terminal_puts("ALPHANUMERIC PLASMA DISPLAY 32X8");
    terminal_set_cursor(0, 1); terminal_puts("SPI CONNECT VIA 74HC595N");
    terminal_set_cursor(3, 2); terminal_puts("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    terminal_set_cursor(2, 3); terminal_puts("@[\\]^_ !\"#$%&'()*+,-./:;<=>?");
    terminal_set_cursor(3, 4); terminal_puts("REGULAR TEXT ONLY");
    terminal_set_cursor(11, 5); terminal_puts("0123456789");
    terminal_set_cursor(0, 6); terminal_puts("When string length more than 32,text continues on the next line.");
    terminal_flush();
}

#ifdef DEBUG_PATTERN
void loop() {
    static uint8_t addr = 0;
    uint8_t data = (addr & 1) ? 0x55 : 0xAA;
    display_write_raw(addr, data);
    addr++;
}
#else
void loop() {
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
