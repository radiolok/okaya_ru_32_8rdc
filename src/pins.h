#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

// SPI to 74HC595 chain
#define PIN_595_MOSI  PB3
#define PIN_595_SCK   PB5
#define PIN_595_SS    PB2

// RCLK (STCP) — common latch for both 595
#define PIN_STCP      PD2
#define PORT_STCP     PORTD
#define DDR_STCP      DDRD
#define MASK_STCP     (1 << PD2)

// Display control signals
#define PIN_NS7       PD3
#define PORT_NS7      PORTD
#define DDR_NS7       DDRD
#define MASK_NS7      (1 << PD3)

#define PIN_NS8       PD4
#define PORT_NS8      PORTD
#define DDR_NS8       DDRD
#define MASK_NS8      (1 << PD4)

#define PIN_nAD       PD5
#define PORT_nAD      PORTD
#define DDR_nAD       DDRD
#define MASK_nAD      (1 << PD5)

#define PIN_nAS       PD6
#define PORT_nAS      PORTD
#define DDR_nAS       DDRD
#define MASK_nAS      (1 << PD6)

// NS4 — input from display (D-trigger divider /4)
#define PIN_NS4       PC4
#define PORT_NS4      PORTC
#define DDR_NS4       DDRC
#define PIN_NS4_REG   PINC
#define MASK_NS4      (1 << PC4)

#define PIN_nWR       PC0
#define PORT_nWR      PORTC
#define DDR_nWR       DDRC
#define MASK_nWR      (1 << PC0)

#define PIN_nBL       PC1
#define PORT_nBL      PORTC
#define DDR_nBL       DDRC
#define MASK_nBL      (1 << PC1)

#define PIN_nRESET    PC2
#define PORT_nRESET   PORTC
#define DDR_nRESET    DDRC
#define MASK_nRESET   (1 << PC2)

// HV boost converter
#define PIN_HV_ADC    PC3
#define PIN_HV_PWM    PB1

#define PIN_UART_RX   PD0
#define PIN_UART_TX   PD1

#endif
