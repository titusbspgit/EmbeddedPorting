#ifndef MISC_H
#define MISC_H
#include <stdint.h>
void delay_us(uint32_t usec);
unsigned int irq_lock_save(void);
void irq_unlock_restore(unsigned int key);
void cpu_reset(void);
void halt_irqs_off(void);
#endif
