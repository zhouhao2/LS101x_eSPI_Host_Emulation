#ifndef ESPI_HOST_H_
#define ESPI_HOST_H_

#include <stdint.h>
#include <stdbool.h>
#include "espi_host_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ESPI_OPCODE_GET_CONFIGURATION  0x21u

#define ESPI_RSP_ACCEPT           0x08u
#define ESPI_RSP_DEFER            0x01u
#define ESPI_RSP_NON_FATAL_ERROR  0x02u
#define ESPI_RSP_FATAL_ERROR      0x03u
#define ESPI_RSP_WAIT_STATE       0x0Fu
#define ESPI_RSP_NO_RESPONSE      0xFFu

typedef struct
{
    uint8_t  rsp;
    uint32_t data;
    uint16_t status;
    uint8_t  crc;
    bool     crc_ok;
    uint8_t  wait_states;
} espi_get_cfg_result_t;

void espi_host_init(void);
int espi_get_configuration(uint16_t addr, espi_get_cfg_result_t *out);
void espi_log_get_cfg(uint16_t addr, const espi_get_cfg_result_t *r);

#ifdef __cplusplus
}
#endif

#endif
