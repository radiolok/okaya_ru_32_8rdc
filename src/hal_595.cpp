#include "hal_595.h"
#include "pins.h"
#include <SPI.h>

void shift595_init() {
    DDR_STCP |= MASK_STCP;
    PORT_STCP &= ~MASK_STCP;

    DDRB |= (1 << PIN_595_SS);
    PORTB |= (1 << PIN_595_SS);

    SPI.begin();
}

void shift595_write(uint8_t addr, uint8_t data) {
    PORT_STCP &= ~MASK_STCP;
    SPI.transfer(data);
    SPI.transfer(addr);
    PORT_STCP |= MASK_STCP;
}
