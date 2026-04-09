# Porting Delta Checklist (F401RE -> F407VG)
- CPU clock: 84MHz -> 168MHz (updated SystemClock_Config)
- SRAM: 96KB -> 192KB (linker adds CCMRAM; RAM 128KB + CCM 64KB)
- Flash: ~500KB -> 1MB (linker FLASH=1024K)
- I2C: SHT30 on I2C2 (Temperature/Humidity init uses hi2c2); BME280/TSL25911 on I2C1
- SPI NOR: W25Q64->W25Q128; driver detects capacity via JEDEC (0x18)
- GPIO: LEDs to PD12..PD15; WDG to PD8; SPI1 CS to PC4 (main.h+GPIO init updated)
- TIM3: 1kHz tick -> ARR=9999 (10s); WDG_SetReload5s() for 5s
- FreeRTOS: configCPU_CLOCK_HZ=SystemCoreClock; tick 1kHz
