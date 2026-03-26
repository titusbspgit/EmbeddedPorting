/* Auto-generated program.c for test_gpio_negedge_intr_en
 * Implements CSV Description/Procedure using only impacted registers.
 * Updated: ISR now clears RAW_STCLR1 using group mask and acknowledges SYSREG; errors accumulated.
 */

#include "test_define.c"

#include <stdint.h>

static volatile int g_int_pend = 0;
static volatile int g_isr_errs = 0;

/* Placeholder for platform ISR hook; actual vector wiring is platform-specific. */
void Default_IRQHandler(void)
{
    g_int_pend = 1;

#ifdef MIZAR_GPIO_GP0_INTR1_INTR_STS1
    uint32_t rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp == 0u) {
        DEBUG_DISPLAY("[NEG][ISR] Expected non-zero group status before clear\n");
        g_isr_errs++;
    }
#else
    uint32_t rdata_grp = 0u;
#endif

#ifdef MIZAR_GPIO_GP0_GPIO_8
    for (int i = 0; i < 32; ++i) {
        uint32_t addr = (uint32_t)MIZAR_GPIO_GP0_GPIO_8 + ((uint32_t)i * 4u);
        uint32_t rdata = read_reg(addr);
        if ((rdata & 0x1u) != 0u) {
            DEBUG_DISPLAY("[NEG][ISR] Pin %d: unexpected bit0 set before clear\n", i);
            g_isr_errs++;
        }
        if ((rdata & 0x2u) == 0u) {
            DEBUG_DISPLAY("[NEG][ISR] Pin %d: expected bit1 set before clear\n", i);
            g_isr_errs++;
        }
    }
#endif

#ifdef MIZAR_GPIO_GPIO_INTR_RAW_STCLR1
    /* Clear per-pin raw status using observed group mask */
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, rdata_grp);
#endif

#ifdef MIZAR_GPIO_GP0_INTR1_INTR_STS1
    /* Verify group cleared */
    uint32_t grp_after = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (grp_after != 0u) {
        DEBUG_DISPLAY("[NEG][ISR] Group status not cleared after RAW_STCLR1 (0x%08X)\n", grp_after);
        g_isr_errs++;
    }
#endif

#ifdef MIZAR_LSS_SYSREG_RAW_STCR1
    /* Acknowledge at SYSREG: write same mask */
    if (rdata_grp != 0u) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, rdata_grp);
    }
#endif
}

void test_case(void)
{
    int test_err = 0;

#ifdef MIZAR_LSS_SYSREG_INTR_EN1
    /* TODO: Replace 0 with proper enable mask for GPIOx interrupts */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0u);
#endif

    for (int i = 0; i < 32; ++i) {
#ifdef MIZAR_GPIO_GP0_GPIO_8
        uint32_t addr = (uint32_t)MIZAR_GPIO_GP0_GPIO_8 + ((uint32_t)i * 4u);
        /* Configure: (1u<<20)|(1u<<18)|(1u<<16) per procedure */
        write_reg(addr, (1u << 20) | (1u << 18) | (1u << 16));
#endif

#ifdef MIZAR_GPIO_GPIO_INTR_RAW_STCLR1
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, (1u << i));
#endif

#ifdef MIZAR_GPIO_GP0_INTR1_INTR_EN1
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, (1u << i));
#endif

        g_int_pend = 0;
        int timeout = 1000000;
        while (timeout-- > 0) {
            if (g_int_pend) break;
        }
        if (timeout <= 0) {
            DEBUG_DISPLAY("[NEG] Timeout waiting for interrupt on pin %d\n", i);
            test_err++;
        }

        /* Accumulate ISR-detected errors and reset for next iteration */
        if (g_isr_errs) {
            test_err += g_isr_errs;
            g_isr_errs = 0;
        }
    }

    finish(test_err ? 1 : 0);
}
