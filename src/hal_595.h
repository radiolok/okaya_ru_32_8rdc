#ifndef HAL_595_H
#define HAL_595_H

#include <stdint.h>

void shift595_init();
void shift595_write(uint8_t addr, uint8_t data);

#endif
