#include "hal_hv.h"
#include "pins.h"
#include <Arduino.h>
#include <avr/interrupt.h>

#define HV_PWM_MAX  24

static volatile uint8_t hv_duty = 0;
static volatile bool hv_ready = false;
static volatile bool hv_regulation_active = false;

ISR(ADC_vect) {
    uint16_t adc = ADC;

    if (!hv_regulation_active) {
        OCR1AH = 0;
        OCR1AL = hv_duty;
        return;
    }

    if (!hv_ready && hv_duty > 0 && adc >= HV_TARGET_ADC) {
        hv_ready = true;
    }

    if (hv_ready) {
        int16_t error = (int16_t)(HV_TARGET_ADC - adc);
        int16_t adj = (int16_t)((int32_t)error * HV_KP / 16);
        int16_t new_duty = (int16_t)hv_duty + adj;
        if (new_duty < 0) new_duty = 0;
        if (new_duty > HV_PWM_MAX) new_duty = HV_PWM_MAX;
        hv_duty = (uint8_t)new_duty;
    }

    OCR1AH = 0;
    OCR1AL = hv_duty;
}

void hv_init() {
    ADMUX = (1 << REFS0) | 3;
    ADCSRB = 0;
    ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (1 << ADIE)
           | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    DDRB |= (1 << PIN_HV_PWM);

    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1C = 0;
    TCNT1H = 0;
    TCNT1L = 0;
    ICR1H = 0;
    ICR1L = HV_PWM_MAX;
    OCR1AH = 0;
    OCR1AL = 0;
    TCCR1A = (1 << COM1A1) | (1 << WGM11);
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
}

uint16_t hv_read() {
    uint8_t old_adcsra = ADCSRA;
    ADCSRA &= ~((1 << ADATE) | (1 << ADIE));
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    uint16_t result = ADC;
    ADCSRA = old_adcsra;
    return result;
}

void hv_start_soft() {
    hv_duty = HV_START_DUTY;
    hv_ready = false;
    hv_regulation_active = false;

    uint32_t t0 = millis();
    while (millis() - t0 < HV_SOFT_START_MS) {
        uint32_t elapsed = millis() - t0;
        uint32_t duty = (uint32_t)HV_PWM_MAX * elapsed / HV_SOFT_START_MS;
        if (duty > HV_PWM_MAX) duty = HV_PWM_MAX;
        hv_duty = (uint8_t)duty;
        delay(10);
    }

    hv_duty = HV_PWM_MAX / 2;
    hv_regulation_active = true;

    t0 = millis();
    while (!hv_ready && (millis() - t0 < HV_TIMEOUT_MS)) {
        delay(20);
    }
}

bool hv_is_ready() {
    return hv_ready;
}
