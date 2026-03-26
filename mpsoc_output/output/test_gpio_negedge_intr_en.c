#include <stdio.h>
#include <test_common.h>
#include <lss_sysreg.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

// extern void finish(int);
// extern void wait_on(unsigned int);
// extern unsigned int read_reg(unsigned long addr);
// extern void write_reg(unsigned long addr, unsigned int val);
// extern void GIC_EnableIRQ(int id);
// extern void GIC_ClearIRQ(int id);

/*
 * Testcase: test_gpio_negedge_intr_en
 * Description (CSV):
 *   Enable group interrupt output in MIZAR_LSS_SYSREG_INTR_EN1, configure per-pin control at
 *   MIZAR_GPIO_GP0_GPIO_8+(i*4) with (1u<<20)|(1u<<18)|(1u<<16), clear MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 and enable
 *   MIZAR_GPIO_GP0_INTR1_INTR_EN1 per-bit, then generate a negedge via writes to 0xA0243FFC and wait for ISR to clear
 *   int_pend; ISR validates MIZAR_GPIO_GP0_INTR1_INTR_STS1 and clears per-pin and system raw statuses.
 *
 * Acceptance (CSV):
 *   - Main loop: timeout must not expire while waiting for int_pend to clear
 *   - ISR: verifies per-pin and group status, clears raw, finish(test_err)
 */

#define IRQ_GPIO0   87
#define PAD_STIM    0xA0243FFCU

static volatile unsigned int int_pend = 0U;
static volatile unsigned int test_err = 0U;
static volatile unsigned int cur_i = 0U;

static void isr_clear_pin_and_group(unsigned int i)
{
    const unsigned long pin_addr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4UL));
    unsigned int local_wr = (1U << i);

    /* Read back pin control and group status */
    unsigned int rdata_pin = read_reg(pin_addr);
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);

#ifdef DEBUG_DISPLAY
    printf("[ISR] i=%u pin=0x%08lX pin_val=0x%08X grp_sts=0x%08X\n", i, pin_addr, rdata_pin, rdata_grp);
#endif

    /* If configured negedge latched, expect grp bit set */
    if ((rdata_grp & local_wr) == 0U) {
#ifdef DEBUG_DISPLAY
        printf("[ISR] ERROR: Group status bit not set for i=%u (grp=0x%08X)\n", i, rdata_grp);
#endif
        test_err++;
    }

    /* Clear per-pin latched/raw bits per CSV: (1u<<20)|(1u<<16) */
    write_reg(pin_addr, ((1U<<20) | (1U<<16)));

    /* Clear RAW status for this pin in RAW_STCLR */
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);

    /* Verify group status clears to 0 */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if ((rdata_grp & local_wr) != 0U) {
#ifdef DEBUG_DISPLAY
        printf("[ISR] ERROR: Group status not cleared for i=%u (grp=0x%08X)\n", i, rdata_grp);
#endif
        test_err++;
    }

    /* Clear system raw via LSS_SYSREG */
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, /* clear GPIO0 */ (1U << 0));

    /* Clear GIC IRQ */
    GIC_ClearIRQ(IRQ_GPIO0);
}

void Default_IRQHandler(void)
{
#ifdef DEBUG_DISPLAY
    printf("[ISR] Default_IRQHandler entered (i=%u)\n", cur_i);
#endif
    int_pend = 0U;
    isr_clear_pin_and_group(cur_i);
}

void test_case(void)
{
    test_err = 0U;

#ifdef DEBUG_DISPLAY
    printf("[test_gpio_negedge_intr_en] Start\n");
#endif

    /* Optionally enable GIC IRQ for GPIO0 */
    GIC_EnableIRQ(IRQ_GPIO0);

    /* Enable system-level GPIO0 interrupt output */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, /* enable GPIO0 bit */ (1U << 0));

    /* Drive PAD_STIM to HIGH initially */
    write_reg(PAD_STIM, 0xFFFFFFFFU);

    /* Configure per-pin control for negedge: (1<<20)|(1<<18)|(1<<16) */
    for (unsigned int i = 0; i < 32U; i++) {
        const unsigned long addr1 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4UL));
        write_reg(addr1, ((1U<<20) | (1U<<18) | (1U<<16)));
    }

    /* Iterate over pins: clear RAW, enable per-bit EN1, then generate negedge and wait for ISR */
    for (unsigned int i = 0; i < 32U; i++) {
        unsigned int wr_val = (1U << i);
        cur_i = i;

        /* Clear RAW and enable this pin in EN1 */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10U);

        /* Generate falling edge on PAD_STIM: high -> ~(1<<i) */
        int_pend = 1U;
        write_reg(PAD_STIM, 0xFFFFFFFFU);
        write_reg(PAD_STIM, (~wr_val));

        /* Poll with timeout until ISR clears int_pend */
        unsigned int to = 5000U;
        while ((int_pend != 0U) && (to-- > 0U)) {
            wait_on(10U);
        }
        if (int_pend != 0U) {
#ifdef DEBUG_DISPLAY
            printf("[ERROR] Timeout waiting for ISR (i=%u)\n", i);
#endif
            test_err++;
            /* attempt cleanup */
            isr_clear_pin_and_group(i);
            int_pend = 0U;
        }

        /* Mask this pin before next iteration */
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000U);
    }

#ifdef DEBUG_DISPLAY
    printf("[test_gpio_negedge_intr_en] Done test_err=%u\n", test_err);
#endif

    finish((test_err == 0U) ? 0 : 1);
}
