#include "misc.h"

void delay_us(uint32_t us)
{
    uint32_t cycles_per_us = SystemCoreClock / 1000000UL;
    uint32_t iterations = (cycles_per_us / 5UL) * us;
    if (iterations == 0U) { iterations = 1U; }
    volatile uint32_t i = iterations;
    while (i-- > 0U) { __NOP(); }
}

void delay_ns(uint32_t ns)
{
    uint32_t cycles_per_ns = SystemCoreClock / 1000000000UL;
    uint32_t iterations = (cycles_per_ns / 5UL) * ns;
    if (iterations == 0U) { iterations = 1U; }
    volatile uint32_t i = iterations;
    while (i-- > 0U) { __NOP(); }
}

void Disable_IRQs(void)
{
    __disable_irq();
}

void System_Reset(void)
{
    __disable_irq();
    NVIC_SystemReset();
}

void Halt(void)
{
    __disable_irq();
    for(;;){ }
}