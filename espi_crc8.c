#include "espi_crc8.h"

uint8_t espi_crc8_update(uint8_t crc, uint8_t data)
{
    crc ^= data;
    for (int i = 0; i < 8; i++)
    {
        if (crc & 0x80u)
        {
            crc = (uint8_t)((crc << 1) ^ 0x07u);
        }
        else
        {
            crc = (uint8_t)(crc << 1);
        }
    }
    return crc;
}

uint8_t espi_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0;
    size_t i;

    for (i = 0; i < len; i++)
    {
        crc = espi_crc8_update(crc, data[i]);
    }
    return crc;
}
