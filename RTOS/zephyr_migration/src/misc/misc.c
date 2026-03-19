#include <zephyr/kernel.h>
#include <zephyr/irq.h>
#include <zephyr/sys/reboot.h>
#include "misc.h"

void delay_us(uint32_t usec) { k_busy_wait(usec); }
unsigned int irq_lock_save(void) { return irq_lock(); }
void irq_unlock_restore(unsigned int key) { irq_unlock(key); }
void cpu_reset(void) { sys_reboot(SYS_REBOOT_COLD); }
void halt_irqs_off(void) { unsigned int k = irq_lock(); for (;;) { (void)k; } }
