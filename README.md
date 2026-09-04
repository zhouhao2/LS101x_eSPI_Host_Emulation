# LS EC eSPI Host Emulation

用 LS EC（SDK 芯片名 **leo**，手册名 **RiverSuzhou / RSzhou**）的 GPIO 位带模拟 eSPI **主机**，上电后周期性发送 `GET_CONFIGURATION`（opcode `0x21`），并通过 UART 打印从机响应。

片上 eSPI IP 仅为 Slave，本工程**不**使用该外设，也**不**调用 `pinmux_espi_init()`。

## 依赖

- 兄弟目录中的 SDK：[`../ls_sdk`](../ls_sdk)
- RISC-V GCC 工具链（与 `ls_sdk` 例程相同）
- Python 依赖：`pip install -r ../ls_sdk/requirements.txt`

可通过环境变量覆盖 SDK 路径：

```
set SDK_ROOT=D:\path\to\ls_sdk
```

## 构建

```
scons ic=leo
```

产物在 `build/leo/`（含 `espi_host_emulation.bin` / `.hex`）。

## 默认引脚

| 信号 | GPIO | 方向 | 空闲电平 |
|------|------|------|----------|
| CS# | PE15 | 输出 | 高 |
| CLK | PE13 | 输出 | 低（CPOL=0） |
| IO0 / MOSI | PE12 | 命令阶段输出，TAR 后输入 | 高 |
| IO1 / MISO | PE14 | 输入 | 上拉 |
| Reset# | PE04 | 输出 | 初始化拉低后释放为高 |

引脚在 [`espi_host_cfg.h`](espi_host_cfg.h) 中修改。

日志 UART3：PH05 TXD / PH04 RXD，波特率 115200。

## 接线（对接另一片 LS EC 的 eSPI Slave）

| 本工程 Host GPIO | 对端 Slave（原生 eSPI） |
|------------------|-------------------------|
| PE15 CS# | PD10 `espi_cs_n` |
| PE13 CLK | PD15 `espi_clk` |
| PE12 IO0 | PD14 `espi_dat[0]` |
| PE14 IO1 | PD13 `espi_dat[1]` |
| PE04 Reset# | 对端 eSPI Reset# / 复位 |

电气：eSPI 规范为 **1.8 V**。两片 LS EC 若 GPIO 同为 3.3 V 可互连；对接 Intel PCH 必须使用 1.8 V 电源域。

## 行为

1. `sys_init_none()` 后配置 GPIO，Reset# 拉低 10 ms 再释放，等待 `tINIT`。
2. 每 3 秒依次读取：
   - `0x0008` General Capabilities and Configurations
   - `0x0004` Device Identification
3. UART 打印 RSP / DATA / STATUS / CRC，并对 `0x0008` 解码通道、I/O 模式、频率等位域。

复位后总线默认 Single I/O、CRC 检查关闭。本 Host 仍按规范计算并发送 CRC-8（多项式 `x^8+x^2+x+1`，seed `0`）。位带时钟约 500 kHz，低于初始化上限 20 MHz。

## 日志示例

```
GET_CFG addr=0x0008 rsp=0x08 ACCEPT data=0x0011000F status=0x0000 crc=ok
  CRC_en=0 IO_sel=Single freq=20MHz ch_sup=0x0F
GET_CFG addr=0x0004 rsp=0x08 ACCEPT data=0x00000001 status=0x0000 crc=ok
  Version ID=0x01
```

无从机时（弱上拉读到 `0xFF`）：

```
GET_CFG addr=0x0008 rsp=0xFF NO_RESPONSE (no target)
```
