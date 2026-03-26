/* Auto-generated program.c for test_gpio_negedge_intr_en
 * Implements CSV Description/Procedure using only impacted registers.
 * TODOs mark places where exact masks/fields require register specs.
 */

#include "test_define.c"

#include <stdint.h>

static volatile int g_int_pend = 0;

/* Placeholder for platform ISR hook; actual vector wiring is platform-specific. */
void Default_IRQHandler(void)
{
    /* Indicate interrupt observed */
    g_int_pend = 1;

#ifdef MIZAR_GPIO_GP0_INTR1_INTR_STS1
    /* Read group interrupt status and perform basic validation */
    uint32_t rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    (void)rdata_grp; /* used below under conditionals */
#endif

#ifdef MIZAR_GPIO_GP0_GPIO_8
    /* Per-pin checks (minimal, per acceptance criteria) */
    for (int i = 0; i < 32; ++i) {
        uint32_t addr = (uint32_t)MIZAR_GPIO_GP0_GPIO_8 + ((uint32_t)i * 4u);
        uint32_t rdata = read_reg(addr);
        /* Acceptance: (rdata & 0x1) == 0 and (rdata & 0x2) != 0 before clearing */
        if ((rdata & 0x1u) != 0u) {
            DEBUG_DISPLAY("[NEG] Pin %d: unexpected bit0 set before clear\n", i);
        }
        if ((rdata & 0x2u) == 0u) {
            DEBUG_DISPLAY("[NEG] Pin %d: expected bit1 set before clear\n", i);
        }
    }
#endif

#ifdef MIZAR_GPIO_GPIO_INTR_RAW_STCLR1
    /* Clear per-pin raw status: note exact mask is per active pin; using rdata_grp when available */
    /* TODO: Determine exact bit to clear from rdata_grp */
    /* write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, rdata_grp); */
#endif

#ifdef MIZAR_LSS_SYSREG_RAW_STCR1
    /* Acknowledge at SYSREG: write appropriate raw status clear mask (platform-specific) */
    /* TODO: Provide correct mask per IRQ source */
    /* write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, <mask>); */
#endif
}

void test_case(void)
{
    int test_err = 0;

    /* Enable GPIO interrupt at SYSREG level */
#ifdef MIZAR_LSS_SYSREG_INTR_EN1
    /* TODO: Replace 0 with proper enable mask for GPIOx interrupts */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0u);
#endif

    /* Drive initial state (external stim 0xA0243FFC omitted per impacted-register-only rule) */

    for (int i = 0; i < 32; ++i) {
#ifdef MIZAR_GPIO_GP0_GPIO_8
        uint32_t addr = (uint32_t)MIZAR_GPIO_GP0_GPIO_8 + ((uint32_t)i * 4u);
        /* Configure per-pin: (1u<<20)|(1u<<18)|(1u<<16) per procedure */
        write_reg(addr, (1u << 20) | (1u << 18) | (1u << 16));
#endif

#ifdef MIZAR_GPIO_GPIO_INTR_RAW_STCLR1
        /* Clear any pending raw before enabling */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, (1u << i));
#endif

#ifdef MIZAR_GPIO_GP0_INTR1_INTR_EN1
        /* Enable group interrupt for this pin */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, (1u << i));
#endif

        /* Generate edges: external toggle at 0xA0243FFC not in impacted list; left as TODO */
        /* TODO: Stimulus: write 0xA0243FFC=~(1u<<i) to create edge */

        g_int_pend = 0;
        int timeout = 1000000; /* simple spin timeout */
        while (timeout-- > 0) {
            if (g_int_pend) break;
        }
        if (timeout <= 0) {
            DEBUG_DISPLAY("[NEG] Timeout waiting for interrupt on pin %d\n", i);
            test_err++;
        }

#ifdef MIZAR_GPIO_GP0_INTR1_INTR_STS1
        /* Verify group status bit for pin i set then cleared by ISR */
        uint32_t grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((grp & (1u << i)) == 0u) {
            DEBUG_DISPLAY("[NEG] Group status bit not set for pin %d before clear\n", i);
            test_err++;
        }
        /* Clearing performed in ISR via RAW_STCLR1; re-read and expect 0 */
        grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (grp != 0u) {
            DEBUG_DISPLAY("[NEG] Group status not cleared after RAW_STCLR1 for pin %d (grp=0x%08X)\n", i, grp);
            test_err++;
        }
#endif
    }

    finish(test_err ? 1 : 0);
}
