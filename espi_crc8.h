#ifndef ESPI_CRC8_H_
#define ESPI_CRC8_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Intel eSPI CRC-8: poly x^8+x^2+x+1 (0x07), seed 0x00, MSB first. */
uint8_t espi_crc8_update(uint8_t crc, uint8_t data);
uint8_t espi_crc8(const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif
