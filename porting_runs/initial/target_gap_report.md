# Target Gap Report (STM32F407VG)

Priority P1
- Startup: Ensure toolchain links startup_stm32f407xx.s (added) and uses correct vector symbols.
- HAL sources: Repository does not contain STM32 HAL driver source (stm32f4xx_hal_*.c). The build must source these from CubeIDE or vendor pack.
- Linker: stm32f407vg.ld added; verify memory sizes and optional CCMRAM mapping.

Priority P2
- System clock: SystemClock_Config() sets 168MHz; verify HSE presence/PLL values on Discovery1 board.
- FreeRTOS heap/stack sizing: tune per task usage.

Priority P3
- W25Q128 timings and high-speed SPI prescaler tune after hardware test.
- Tight-loop delay calibration constants can be refined if needed.
