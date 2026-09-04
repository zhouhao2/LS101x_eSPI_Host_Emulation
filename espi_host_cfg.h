#ifndef ESPI_HOST_CFG_H_
#define ESPI_HOST_CFG_H_

#include "ls_soc_gpio.h"

/*
 * Host GPIO map (Master):
 *   PE15 CS#, PE13 CLK, PE12 IO0, PE14 IO1
 * Avoid native eSPI (PD9–PD15), SPIS (PE0–PE3), log UART (PH04/PH05),
 * EVB LEDs (PE09–PE11), and SWD (PH02/PH03).
 *
 * Do NOT call pinmux_espi_init(); this firmware bit-bangs a host and
 * must not claim the on-chip eSPI slave pins.
 */
#ifndef ESPI_CS_PIN
#define ESPI_CS_PIN   PE15
#endif
#ifndef ESPI_CLK_PIN
#define ESPI_CLK_PIN  PE13
#endif
#ifndef ESPI_IO0_PIN
#define ESPI_IO0_PIN  PE12
#endif
#ifndef ESPI_IO1_PIN
#define ESPI_IO1_PIN  PE14
#endif
#ifndef ESPI_RST_PIN
#define ESPI_RST_PIN  PE04
#endif

/* Half-period of SCK. 1 us high + 1 us low ≈ 500 kHz (well below 20 MHz tINIT-FREQ). */
#ifndef ESPI_CLK_HALF_PERIOD_US
#define ESPI_CLK_HALF_PERIOD_US  1u
#endif

#ifndef ESPI_RESET_HOLD_MS
#define ESPI_RESET_HOLD_MS  10u
#endif

/* tINIT after Reset# release is 1 us; use a small margin. */
#ifndef ESPI_TINIT_US
#define ESPI_TINIT_US  10u
#endif

#ifndef ESPI_POLL_PERIOD_MS
#define ESPI_POLL_PERIOD_MS  3000u
#endif

#ifndef ESPI_MAX_WAIT_STATES
#define ESPI_MAX_WAIT_STATES  32u
#endif

#define ESPI_CFG_ADDR_DEVICE_ID  0x0004u
#define ESPI_CFG_ADDR_GEN_CAP    0x0008u

/* Registers read each poll cycle (General Capabilities, then Device ID). */
#ifndef ESPI_POLL_ADDRS
#define ESPI_POLL_ADDRS { ESPI_CFG_ADDR_GEN_CAP, ESPI_CFG_ADDR_DEVICE_ID }
#endif

#endif
