#define LOG_TAG "MAIN"
#include "log.h"
#include "platform.h"
#include "espi_host.h"

int main(void)
{
    static const uint16_t poll_addrs[] = ESPI_POLL_ADDRS;
    const unsigned naddr = (unsigned)(sizeof(poll_addrs) / sizeof(poll_addrs[0]));
    unsigned i;

    sys_init_none();
    espi_host_init();
    LOG_I("eSPI host GPIO bit-bang ready, poll %u ms", (unsigned)ESPI_POLL_PERIOD_MS);

    while (1)
    {
        for (i = 0; i < naddr; i++)
        {
            espi_get_cfg_result_t result;
            espi_get_configuration(poll_addrs[i], &result);
            espi_log_get_cfg(poll_addrs[i], &result);
        }
        DELAY_MS(ESPI_POLL_PERIOD_MS);
    }
}
