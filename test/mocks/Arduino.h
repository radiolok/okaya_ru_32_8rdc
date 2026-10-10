// Host-side stand-in for the Arduino core, used by the native unit tests only.
#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#define INPUT        0x0
#define OUTPUT       0x1
#define INPUT_PULLUP 0x2
#define LOW          0x0
#define HIGH         0x1
#define A5           19
#define A7           21

unsigned long millis();
long random(long max);
long random(long min, long max);
void randomSeed(unsigned long seed);
int analogRead(uint8_t pin);
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
void delayMicroseconds(unsigned int us);

class FakeSerial {
public:
    void begin(unsigned long) {}
    size_t write(uint8_t c);
    int available();
    int read();
};

extern FakeSerial Serial;

#endif
