// Host-side stand-in for <avr/pgmspace.h>: flash is ordinary memory.
#ifndef MOCK_AVR_PGMSPACE_H
#define MOCK_AVR_PGMSPACE_H

#include <stdint.h>

#define PROGMEM
#define pgm_read_byte(p) (*(const uint8_t *)(p))
#define pgm_read_word(p) (*(const uint16_t *)(p))
#define pgm_read_ptr(p)  (*(const void * const *)(p))

#endif
