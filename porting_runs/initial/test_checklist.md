# Test Checklist
- Boot at 168 MHz; HAL_GetTick() increments at 1kHz
- LEDs PD12..PD15 toggle on commands; fault blink cadence works
- I2C1: Read ID from BME280/TSL25911
- I2C2: Read temperature/humidity from SHT30-DIS
- SPI1 NOR: Read JEDEC ID -> expect capacity code 0x18; log JSON record; verify wrap-around
- TIM3: Measure 10s then set 5s via WDG_SetReload5s(); verify PD8 pulse each expiry
- FreeRTOS: All tasks run; stacks within limits; no asserts
