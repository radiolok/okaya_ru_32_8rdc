#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

/** @defgroup pins_spi SPI to 74HC595 chain */
/** @{ */
#define PIN_595_MOSI  PB3  /**< SPI MOSI → 595#1 DS */
#define PIN_595_SCK   PB5  /**< SPI SCK  → both 595 SHCP */
#define PIN_595_SS    PB2  /**< SPI SS   → forced OUTPUT (prevents slave mode) */
/** @} */

/** @defgroup pins_stcp RCLK (STCP) — common latch for both 595 */
/** @{ */
#define PIN_STCP      PD2  /**< RCLK pin */
#define PORT_STCP     PORTD
#define DDR_STCP      DDRD
#define MASK_STCP     (1 << PD2)  /**< RCLK bitmask */
/** @} */

/** @defgroup pins_strobes Display control signals */
/** @{ */
#define PIN_NS7       PD3  /**< NS7 CLOCK — character data latch (active HIGH pulse) */
#define PORT_NS7      PORTD
#define DDR_NS7       DDRD
#define MASK_NS7      (1 << PD3)

#define PIN_NS8       PD4  /**< NS8 CS — chip select (active LOW, held LOW during writes) */
#define PORT_NS8      PORTD
#define DDR_NS8       DDRD
#define MASK_NS8      (1 << PD4)

#define PIN_nAD       PD5  /**< ~AD UL — underline bit (active LOW: LOW = underline ON, HIGH = OFF) */
#define PORT_nAD      PORTD
#define DDR_nAD       DDRD
#define MASK_nAD      (1 << PD5)

#define PIN_nAS       PD6  /**< ~AS AW — address write strobe (active LOW pulse) */
#define PORT_nAS      PORTD
#define DDR_nAS       DDRD
#define MASK_nAS      (1 << PD6)
/** @} */

/** @defgroup pins_ns4 NS4 — READY input from display (active HIGH) */
/** @{ */
#define PIN_NS4       PC4  /**< NS4 READY — display ready (active HIGH) */
#define PORT_NS4      PORTC
#define DDR_NS4       DDRC
#define PIN_NS4_REG   PINC /**< Input register for reading NS4 */
#define MASK_NS4      (1 << PC4)
/** @} */

/** @defgroup pins_other Other display signals */
/** @{ */
#define PIN_nWR       PC0  /**< ~WR — unused, held HIGH */
#define PORT_nWR      PORTC
#define DDR_nWR       DDRC
#define MASK_nWR      (1 << PC0)

#define PIN_nBL       PC1  /**< ~BL blanking (active LOW: LOW = display blanked) */
#define PORT_nBL      PORTC
#define DDR_nBL       DDRC
#define MASK_nBL      (1 << PC1)

#define PIN_nRESET    PC2  /**< ~RESET display RAM reset (active LOW pulse) */
#define PORT_nRESET   PORTC
#define DDR_nRESET    DDRC
#define MASK_nRESET   (1 << PC2)
/** @} */

/** @defgroup pins_hv HV converter enable */
/** @{ */
#define PIN_HV_EN     PB1  /**< ~EN — converter enable (active LOW: 0 = ON, 1 = OFF). Pulled HIGH externally. */
/** @} */

/** @defgroup pins_uart UART */
/** @{ */
#define PIN_UART_RX   PD0  /**< UART RX */
#define PIN_UART_TX   PD1  /**< UART TX */
/** @} */

#endif
