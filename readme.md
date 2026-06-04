# SEGGER Flash Loader for STM32L431RC + W25Q16JV

Open Flash Loader example for external NOR flash via QUADSPI on **STM32L431RC** (HC AllCANLED), using [SEGGER Flash Loader](https://wiki.segger.com/SEGGER_Flash_Loader).

## Hardware

- MCU: STM32L431RC (Cortex-M4, 64 KB SRAM)
- Flash: Winbond W25Q16JV (16 Mbit / 2 MB) on QUADSPI bank 1
- Clock (matches Zephyr DTS): **HSI16 + PLL** → 80 MHz SYSCLK, QSPI max **20 MHz**

| QSPI signal | Pin |
|-------------|-----|
| CLK | PB10 |
| NCS | PB11 |
| IO0 | PB1 |
| IO1 | PB0 |
| IO2 | PA7 |
| IO3 | PA6 |

## External flash partitions (virtual base `0x90000000`)

| Label | Offset | Size |
|-------|--------|------|
| lfs1 | 0x000000 | 1800 KB |
| image-1 | 0x1C2000 | 200 KB |
| storage | 0x1F4000 | 40 KB |
| settings | 0x1FE000 | 8 KB |

## Build

```bash
make clean && make
```

Output: `build/firmware.elf` (copy next to `Devices.xml` for J-Link).

## J-Link device pack

Install device description:

```
Linux/macOS: $HOME/.config/SEGGER/JLinkDevices/ST/L431RC_W25Q16/
Windows:     %APPDATA%\SEGGER\JLinkDevices\ST\L431RC_W25Q16\
```

Copy `Jflash/.config/SEGGER/JLinkDevices/ST/L431RC_W25Q16/Devices.xml` and `build/firmware.elf` into that folder.

Device name in J-Flash: **ST L431RC_W25Q16**, flash bank **SPI Flash**.

**Note:** Halt the target before programming so application code does not use QSPI at the same time.

## J-Flash CLI example

```bash
JFlash -openprj Jflash/stm32l431.jflash -production -exit
```

RTT logs are available via J-Link RTT Viewer during flash operations.

## Loader requirements

- Implements `SEGGER_FL_Prepare`, `Restore`, `Program`, `Erase`, and `Read` (external flash)
- `FlashDevice` descriptor in `DevDscr` section
- Linker sections: `PrgCode` → code/rodata → `PrgData` → data → `DevDscr`

See [J-Link Device Support Kit](https://wiki.segger.com/J-Link_Device_Support_Kit) for XML details.
