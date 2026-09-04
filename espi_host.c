#define LOG_TAG "ESPI"
#include "log.h"
#include "platform.h"
#include "ls_soc_gpio.h"
#include "espi_host.h"
#include "espi_crc8.h"

static void espi_half_period(void)
{
    DELAY_US(ESPI_CLK_HALF_PERIOD_US);
}

static void espi_clk_high(void)
{
    io_set_pin(ESPI_CLK_PIN);
    espi_half_period();
}

static void espi_clk_low(void)
{
    io_clr_pin(ESPI_CLK_PIN);
    espi_half_period();
}

static void espi_io0_output(void)
{
    io_cfg_output(ESPI_IO0_PIN);
    io_cfg_pushpull(ESPI_IO0_PIN);
}

static void espi_write_byte(uint8_t val)
{
    int i;

    /* Host sets data while CLK is low; Target samples on the rising edge. */
    for (i = 7; i >= 0; i--)
    {
        if (val & (1u << i))
        {
            io_set_pin(ESPI_IO0_PIN);
        }
        else
        {
            io_clr_pin(ESPI_IO0_PIN);
        }
        espi_clk_high();
        espi_clk_low();
    }
}

static uint8_t espi_read_byte(void)
{
    uint8_t val = 0;
    int i;

    /* Target updates on the falling edge; Host samples on the rising edge. */
    for (i = 7; i >= 0; i--)
    {
        espi_clk_high();
        if (io_get_input_val(ESPI_IO1_PIN))
        {
            val |= (uint8_t)(1u << i);
        }
        espi_clk_low();
    }
    return val;
}

static void espi_tar(void)
{
    io_set_pin(ESPI_IO0_PIN);
    espi_clk_high();
    espi_clk_low();

    io_cfg_input(ESPI_IO0_PIN);
    espi_clk_high();
    espi_clk_low();
}

static void espi_cs_assert(void)
{
    espi_io0_output();
    io_clr_pin(ESPI_CS_PIN);
    espi_half_period();
}

static void espi_cs_deassert(void)
{
    espi_half_period();
    io_set_pin(ESPI_CS_PIN);
    espi_io0_output();
    io_set_pin(ESPI_IO0_PIN);
    espi_half_period();
}

void espi_host_init(void)
{
    io_cfg_output(ESPI_CS_PIN);
    io_cfg_pushpull(ESPI_CS_PIN);
    io_pull_write(ESPI_CS_PIN, IO_PULL_UP);
    io_drive_capacity_write(ESPI_CS_PIN, IO_OUTPUT_MAX_DRIVER);
    io_set_pin(ESPI_CS_PIN);

    io_cfg_output(ESPI_CLK_PIN);
    io_cfg_pushpull(ESPI_CLK_PIN);
    io_pull_write(ESPI_CLK_PIN, IO_PULL_DOWN);
    io_drive_capacity_write(ESPI_CLK_PIN, IO_OUTPUT_MAX_DRIVER);
    io_clr_pin(ESPI_CLK_PIN);

    espi_io0_output();
    io_pull_write(ESPI_IO0_PIN, IO_PULL_UP);
    io_drive_capacity_write(ESPI_IO0_PIN, IO_OUTPUT_MAX_DRIVER);
    io_set_pin(ESPI_IO0_PIN);

    io_cfg_input(ESPI_IO1_PIN);
    io_pull_write(ESPI_IO1_PIN, IO_PULL_UP);

    io_cfg_output(ESPI_RST_PIN);
    io_cfg_pushpull(ESPI_RST_PIN);
    io_pull_write(ESPI_RST_PIN, IO_PULL_UP);
    io_drive_capacity_write(ESPI_RST_PIN, IO_OUTPUT_MAX_DRIVER);
    io_clr_pin(ESPI_RST_PIN);
    DELAY_MS(ESPI_RESET_HOLD_MS);
    io_set_pin(ESPI_RST_PIN);
    DELAY_US(ESPI_TINIT_US);
}

static const char *espi_rsp_name(uint8_t rsp)
{
    switch (rsp)
    {
    case ESPI_RSP_ACCEPT:
        return "ACCEPT";
    case ESPI_RSP_DEFER:
        return "DEFER";
    case ESPI_RSP_NON_FATAL_ERROR:
        return "NON_FATAL_ERROR";
    case ESPI_RSP_FATAL_ERROR:
        return "FATAL_ERROR";
    case ESPI_RSP_WAIT_STATE:
        return "WAIT_STATE";
    case ESPI_RSP_NO_RESPONSE:
        return "NO_RESPONSE";
    default:
        return "UNKNOWN";
    }
}

static const char *espi_io_mode_name(uint8_t sel)
{
    switch (sel)
    {
    case 0:
        return "Single";
    case 1:
        return "Dual";
    case 2:
        return "Quad";
    default:
        return "Rsvd";
    }
}

static uint8_t espi_freq_mhz(uint8_t code)
{
    static const uint8_t table[] = {20, 25, 33, 50, 66};
    if (code < (sizeof(table) / sizeof(table[0])))
    {
        return table[code];
    }
    return 0;
}

int espi_get_configuration(uint16_t addr, espi_get_cfg_result_t *out)
{
    uint8_t cmd[3];
    uint8_t cmd_crc;
    uint8_t rsp;
    uint8_t d[4];
    uint8_t s[2];
    uint8_t crc_in[7];
    uint8_t waits = 0;

    if (out == 0)
    {
        return -1;
    }

    cmd[0] = ESPI_OPCODE_GET_CONFIGURATION;
    cmd[1] = (uint8_t)(addr >> 8);
    cmd[2] = (uint8_t)(addr & 0xFFu);
    cmd_crc = espi_crc8(cmd, 3);

    espi_cs_assert();
    espi_write_byte(cmd[0]);
    espi_write_byte(cmd[1]);
    espi_write_byte(cmd[2]);
    espi_write_byte(cmd_crc);
    espi_tar();

    do
    {
        rsp = espi_read_byte();
        if (rsp != ESPI_RSP_WAIT_STATE)
        {
            break;
        }
        waits++;
    } while (waits < ESPI_MAX_WAIT_STATES);

    out->wait_states = waits;
    out->rsp = rsp;

    d[0] = espi_read_byte();
    d[1] = espi_read_byte();
    d[2] = espi_read_byte();
    d[3] = espi_read_byte();
    s[0] = espi_read_byte();
    s[1] = espi_read_byte();
    out->crc = espi_read_byte();

    out->data = (uint32_t)d[0] | ((uint32_t)d[1] << 8) |
                ((uint32_t)d[2] << 16) | ((uint32_t)d[3] << 24);
    out->status = (uint16_t)s[0] | ((uint16_t)s[1] << 8);

    crc_in[0] = rsp;
    crc_in[1] = d[0];
    crc_in[2] = d[1];
    crc_in[3] = d[2];
    crc_in[4] = d[3];
    crc_in[5] = s[0];
    crc_in[6] = s[1];
    out->crc_ok = (espi_crc8(crc_in, 7) == out->crc);

    espi_cs_deassert();
    return 0;
}

void espi_log_get_cfg(uint16_t addr, const espi_get_cfg_result_t *r)
{
    if (r == 0)
    {
        return;
    }

    if (r->rsp == ESPI_RSP_NO_RESPONSE)
    {
        LOG_I("GET_CFG addr=0x%04X rsp=0x%02X NO_RESPONSE (no target)", addr, r->rsp);
        return;
    }

    if (r->wait_states >= ESPI_MAX_WAIT_STATES && r->rsp == ESPI_RSP_WAIT_STATE)
    {
        LOG_W("GET_CFG addr=0x%04X WAIT_STATE timeout (%u)", addr, r->wait_states);
        return;
    }

    LOG_I("GET_CFG addr=0x%04X rsp=0x%02X %s data=0x%08X status=0x%04X crc=%s",
          addr, r->rsp, espi_rsp_name(r->rsp), r->data, r->status,
          r->crc_ok ? "ok" : "fail");

    if (r->rsp != ESPI_RSP_ACCEPT)
    {
        return;
    }

    if (addr == ESPI_CFG_ADDR_GEN_CAP)
    {
        uint8_t io_sel = (uint8_t)((r->data >> 26) & 0x3u);
        uint8_t op_freq = (uint8_t)((r->data >> 20) & 0x7u);
        LOG_I("  CRC_en=%u IO_sel=%s freq=%uMHz ch_sup=0x%02X",
              (unsigned)((r->data >> 31) & 1u),
              espi_io_mode_name(io_sel),
              espi_freq_mhz(op_freq),
              (unsigned)(r->data & 0xFFu));
    }
    else if (addr == ESPI_CFG_ADDR_DEVICE_ID)
    {
        LOG_I("  Version ID=0x%02X", (unsigned)(r->data & 0xFFu));
    }
}
