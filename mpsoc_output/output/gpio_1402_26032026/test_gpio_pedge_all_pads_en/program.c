/* Auto-generated program.c for test_gpio_pedge_all_pads_en
 * Implements CSV Description/Procedure using only impacted registers.
 * Updated: ISR masks group, clears per-pin via GPIOx register writes, acknowledges SYSREG; errors accumulated.
 */

#include "test_define.c"
#include <stdint.h>

static volatile int g_int_pend = 0;
static volatile int g_isr_errs = 0;

void Default_IRQHandler(void)
{
    g_int_pend = 1;
#ifdef MIZAR_GPIO_GP0_INTR1_INTR_STS1
    uint32_t rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        DEBUG_DISPLAY("[PEDGE][ISR] Expected non-zero group status before clear\n");
        g_isr_errs++;
    }
#ifdef MIZAR_GPIO_GP0_INTR1_INTR_EN1
    /* Mask group during service */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);
#endif
#endif

#ifdef MIZAR_GPIO_GP0_GPIO_8
    for (int j = 0; j < 32; ++j) {
        uint32_t addr = (uint32_t)MIZAR_GPIO_GP0_GPIO_8 + ((uint32_t)j * 4u);
        /* Clear per-pin by writing 0x00010000 per CSV procedure */
        write_reg(addr, 0x00010000u);
    }
#endif

#ifdef MIZAR_GPIO_GP0_INTR1_INTR_STS1
    /* Expect group cleared to 0 */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0u) {
        DEBUG_DISPLAY("[PEDGE][ISR] Group status not zero after per-pin clear (0x%08X)\n", rdata_grp);
        g_isr_errs++;
    }
#endif

#ifdef MIZAR_LSS_SYSREG_RAW_STCR1
    /* Acknowledge at SYSREG: write same mask if known */
#ifdef MIZAR_GPIO_GP0_INTR1_INTR_STS1
    if (rdata_grp != 0u) {
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, rdata_grp);
    }
#endif
#endif

#ifdef MIZAR_GPIO_GP0_INTR1_INTR_EN1
    /* Re-enable group */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
#endif
}

void test_case(void)
{
    int test_err = 0;

#ifdef MIZAR_LSS_SYSREG_INTR_EN1
    /* TODO: Replace 0 with proper enable mask for GPIOx interrupts */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0u);
#endif

#ifdef MIZAR_GPIO_GP0_GPIO_8
    for (int i = 0; i < 32; ++i) {
        uint32_t addr = (uint32_t)MIZAR_GPIO_GP0_GPIO_8 + ((uint32_t)i * 4u);
        /* Configure posedge: 0x00020000 per CSV */
        write_reg(addr, 0x00020000u);
    }
#endif

#ifdef MIZAR_GPIO_GPIO_IO_CTRL_GROUP1
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
#endif
#ifdef MIZAR_GPIO_GPIO_IO_CTRL_GROUP2
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
#endif
#ifdef MIZAR_GPIO_GPIO_IO_CTRL_GROUP3
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
#endif
#ifdef MIZAR_GPIO_GPIO_IO_CTRL_GROUP4
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
#endif

#ifdef MIZAR_GPIO_GP0_INTR1_INTR_EN1
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
#endif

    for (int i = 0; i < 32; ++i) {
        g_int_pend = 0;
        int timeout = 1000000;
        while (timeout-- > 0) {
            if (g_int_pend) break;
        }
        if (timeout <= 0) {
            DEBUG_DISPLAY("[PEDGE] Timeout waiting for interrupt on pin %d\n", i);
            test_err++;
            break; /* As per CSV: break on timeout */
        }

        /* Accumulate ISR-detected errors and reset for next iteration */
        if (g_isr_errs) {
            test_err += g_isr_errs;
            g_isr_errs = 0;
        }
    }

    finish(test_err ? 1 : 0);
}
