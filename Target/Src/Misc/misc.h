#ifndef MISC_H
#define MISC_H
#include "stm32f4xx_hal.h"
void delay_us(uint32_t us);
void delay_ns(uint32_t ns);
void Disable_IRQs(void);
void System_Reset(void);
void Halt(void);
#endif
