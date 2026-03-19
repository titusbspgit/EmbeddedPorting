# EnvMon Zephyr Migration (test run)

## Overview
- Migrated scaffold of the FreeRTOS-based "EnvMonPlt" to Zephyr RTOS 4.2.x on stm32f4_disco (STM32F407G-DISC1).
- Preserves original behavior: 1 Hz sampling (5-sample moving average), thresholds → LEDs, JSON logging to SPI-NOR via LittleFS, watchdog feed, fault LED cadence.

## Prerequisites
- Zephyr RTOS 4.2.x and Zephyr SDK 0.17.2 installed
- West workspace initialized (ZEPHYR_BASE set)
- Python and west installed (`pip install west`)
- ST-Link/OpenOCD for flashing

## Build
From the root of this folder (RTOS/zephyr_migration):
```
west build -b stm32f4_disco .
west flash
west debug
```

## Platform Notes
- SPI-NOR W25Q64 on SPI1 with CS=PA4 assumed. Adjust `boards/stm32f4_disco.overlay` for actual wiring and size.
- I2C1 on PB8/PB9 assumed for sensors SHT3x/BME280; TSL2591 is provided as a simple I2C stub to be completed per hardware.
- If building for STM32F401 (original repo SoC), switch the target board (e.g., `-b nucleo_f401re`) and update the overlay accordingly; see MIGRATION_NOTES.md.
