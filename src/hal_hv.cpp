#include "hal_hv.h"
#include "pins.h"
#include <Arduino.h>

void hv_enable() {
    DDRB |= (1 << PIN_HV_EN);
    PORTB &= ~(1 << PIN_HV_EN);
}
