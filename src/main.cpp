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
}

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
