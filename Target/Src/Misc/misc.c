#include "misc.h"
#include "stm32f4xx_hal.h"

/* Simple tight-loop delays. Accuracy depends on optimization and flash wait states. */
static uint32_t loops_per_us = 0U;
static uint32_t loops_per_ns_scaled = 0U; /* scaled to avoid float: loops per ns * 1024 */

static void calibrate(void)
{
  /* Assume ~5 cycles per loop; calibrate from SystemCoreClock. */
  const uint32_t cycles_per_loop = 5U; /* empirical; adjust if characterized */
  loops_per_us = SystemCoreClock / (cycles_per_loop * 1000000UL);
  if (loops_per_us == 0U) { loops_per_us = 1U; }
  /* For ns: cycles per ns = SystemCoreClock / 1e9; loops = cycles / cycles_per_loop */
  uint64_t num = (uint64_t)SystemCoreClock * 1024ULL; /* scale by 1024 */
  uint64_t den = (uint64_t)cycles_per_loop * 1000000000ULL;
  loops_per_ns_scaled = (uint32_t)(num / den);
  if (loops_per_ns_scaled == 0U) { loops_per_ns_scaled = 1U; }
}

void delay_us(uint32_t us)
{
  if (loops_per_us == 0U) { calibrate(); }
  while (us-- > 0U)
  {
    volatile uint32_t i = loops_per_us;
    while (i-- > 0U) { __NOP(); }
  }
}

void delay_ns(uint32_t ns)
{
  if (loops_per_ns_scaled == 0U) { calibrate(); }
  /* Clamp extremely small values */
  if (ns == 0U) { return; }
  uint64_t total_loops_scaled = (uint64_t)ns * (uint64_t)loops_per_ns_scaled;
  uint32_t loops = (uint32_t)(total_loops_scaled >> 10); /* divide by 1024 */
  if (loops == 0U) { loops = 1U; }
  volatile uint32_t i = loops;
  while (i-- > 0U) { __NOP(); }
}

void misc_disable_irqs(void)
{
  __disable_irq();
}

void misc_reset_cpu(void)
{
  NVIC_SystemReset();
}

void misc_trap(void)
{
  __disable_irq();
  for (;;) { __NOP(); }
}
