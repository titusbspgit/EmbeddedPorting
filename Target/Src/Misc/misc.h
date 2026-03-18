#ifndef MISC_H
#define MISC_H

#include <stdint.h>

void delay_us(uint32_t us);
void delay_ns(uint32_t ns);
void misc_disable_irqs(void);
void misc_reset_cpu(void);
void misc_trap(void);

/* helper */
static inline void misc_delay_ms(uint32_t ms)
{
  /* Busy-wait ms using us delay */
  while (ms-- > 0U) { delay_us(1000U); }
}

#endif /* MISC_H */
