# Migration Notes: FreeRTOS → Zephyr

## Scope and Mapping
- Tasks → `k_thread_create` (priorities: SystemControl=2, Sensors=5, Storage=6, Watchdog=7)
- `vTaskDelay`/`vTaskDelayUntil` → `k_msleep`/`k_timer`
- Queues → `k_msgq` (log_q)
- Mutex/Semaphore → `k_mutex`/`k_sem` (mutex used for shared env)
- Timers → `k_timer` (fault LED cadence)
- Critical sections → `irq_lock`/`irq_unlock`
- `pvPortMalloc` → `k_malloc` (not required in current scaffold)
- Remove CubeIDE startup/linker/syscalls from Zephyr build

## Target Platform
- Zephyr 4.2.x, Zephyr SDK 0.17.2, board `stm32f4_disco` per Analysis Report
- Source repo MCU appears STM32F401; if you target that instead, change board and overlay accordingly

## Peripherals
- SHT3x (temp/humidity): Zephyr driver `sht3xd` (if present)
- BME280 (pressure): Zephyr driver `bme280`
- TSL25911 (light): simple I2C stub provided; complete per hardware
- SPI-NOR (W25Q64): LittleFS mounted on `log` partition; adjust size/JEDEC ID
- LEDs: GPIO via Zephyr aliases
- Watchdog: Zephyr WDT (iwdg)

## Timing/Behavior
- 1 Hz sampling with 5-sample average
- System severity is worst among sensors; violation if non-GREEN
- Fault LED blink: 1 s after 30 s violation; 300 ms after 60 s

## Follow-ups/Tuning
- Fill TSL25911 I2C sequences
- Confirm SPI-NOR wiring/size; set rotation policy
- Adjust stacks/priorities after profiling
- If PA9/TIM3 strict ISR required, use `k_timer` or `counter` API
