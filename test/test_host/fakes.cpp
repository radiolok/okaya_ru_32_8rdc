#include "fakes.h"
#include <Arduino.h>
#include <deque>

volatile uint8_t PINB, PORTB, DDRB;
volatile uint8_t PINC, PORTC, DDRC;
volatile uint8_t PIND, PORTD, DDRD;

uint8_t fake_screen[DISP_SIZE];
uint16_t fake_write_count[DISP_SIZE];
uint32_t fake_total_writes;
uint32_t fake_clear_count;
unsigned long fake_millis;
std::string fake_serial_tx;
std::string fake_serial_rx;

static std::deque<long> random_queue;
static unsigned long rng_state = 1;

FakeSerial Serial;

void fake_reset_write_counts() {
    memset(fake_write_count, 0, sizeof(fake_write_count));
    fake_total_writes = 0;
}

void fake_reset() {
    memset(fake_screen, ' ', sizeof(fake_screen));
    fake_reset_write_counts();
    fake_clear_count = 0;
    fake_millis = 1000;
    fake_serial_tx.clear();
    fake_serial_rx.clear();
    random_queue.clear();
    rng_state = 1;
    PINB = PINC = PIND = 0xFF;
    PORTB = PORTC = PORTD = 0;
    DDRB = DDRC = DDRD = 0;
}

void fake_random_push(long value) { random_queue.push_back(value); }
void fake_random_clear() { random_queue.clear(); }

std::string fake_row(uint8_t row) {
    return std::string((const char *)&fake_screen[row * DISP_COLS], DISP_COLS);
}

// --- Arduino core ---------------------------------------------------------

unsigned long millis() { return fake_millis; }

long random(long min, long max) {
    if (max <= min) return min;
    long v;
    if (!random_queue.empty()) {
        v = random_queue.front();
        random_queue.pop_front();
        if (v < min) v = min;
        if (v >= max) v = max - 1;
        return v;
    }
    rng_state = rng_state * 1103515245UL + 12345UL;
    return min + (long)((rng_state >> 8) % (unsigned long)(max - min));
}

long random(long max) { return random(0, max); }
void randomSeed(unsigned long seed) { rng_state = seed ? seed : 1; }
int analogRead(uint8_t) { return 0; }
void pinMode(uint8_t, uint8_t) {}
void digitalWrite(uint8_t, uint8_t) {}
void delayMicroseconds(unsigned int) {}

size_t FakeSerial::write(uint8_t c) {
    fake_serial_tx.push_back((char)c);
    return 1;
}

int FakeSerial::available() { return (int)fake_serial_rx.size(); }

int FakeSerial::read() {
    if (fake_serial_rx.empty()) return -1;
    uint8_t c = (uint8_t)fake_serial_rx[0];
    fake_serial_rx.erase(0, 1);
    return c;
}

// --- Display driver -------------------------------------------------------

void display_write_raw(uint8_t addr, uint8_t data, bool) {
    fake_screen[addr] = data;
    fake_write_count[addr]++;
    fake_total_writes++;
}

void display_write(uint8_t col, uint8_t row, uint8_t ch) {
    if (col >= DISP_COLS || row >= DISP_ROWS) return;
    display_write_raw((uint8_t)(col + row * DISP_COLS), ch);
}

void display_clear() {
    memset(fake_screen, ' ', sizeof(fake_screen));
    fake_reset_write_counts();
    fake_clear_count++;
}
