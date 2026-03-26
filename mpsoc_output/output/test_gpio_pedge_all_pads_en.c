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
 * Testcase: test_gpio_pedge_all_pads_en
 * Description (CSV):
 *   Enable group interrupt via MIZAR_LSS_SYSREG_INTR_EN1, program posedge interrupt per pin by writing 0x00020000
 *   to MIZAR_GPIO_GP0_GPIO_8+(i*4), set GPIO_IO_CTRL_GROUP[1..4] to 0x000000FF, enable MIZAR_GPIO_GP0_INTR1_INTR_EN1=0xFFFFFFFF,
 *   then generate rising edges using 0xA0243FFC and wait for ISR to clear int_pend; ISR checks MIZAR_GPIO_GP0_INTR1_INTR_STS1,
 *   clears per-pin raw via 0x00010000 writes, and clears MIZAR_LSS_SYSREG_RAW_STCR1.
 *
 * Acceptance (CSV):
 *   - Main loop: timeout must not trigger
 *   - ISR: STS1 non-zero on entry and becomes 0 after raw/per-pin clears; RAW_STCR1 cleared; finish(test_err)
 */

#define IRQ_GPIO0   87
#define PAD_STIM    0xA0243FFCU

static volatile unsigned int int_pend = 0U;
static volatile unsigned int test_err = 0U;
static volatile unsigned int cur_i = 0U;

static void isr_clear_and_reenable(void)
{
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
#ifdef DEBUG_DISPLAY
    printf("[ISR] Entry STS1=0x%08X\n", rdata_grp);
#endif
    if ((rdata_grp & 0xFFFFFFFFU) == 0U) {
#ifdef DEBUG_DISPLAY
        printf("[ISR] ERROR: STS1 is zero on entry\n");
#endif
        test_err++;
    }

    /* Mask group interrupts before raw clear */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000U);

    /* Clear per-pin raw by writing 0x00010000 to each per-pin control */
    for (unsigned int j = 0; j < 32U; j++) {
        const unsigned long paddr = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4UL));
        write_reg(paddr, 0x00010000U);
    }
    wait_on(2U);

    /* Verify group status cleared */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0U) {
#ifdef DEBUG_DISPLAY
        printf("[ISR] ERROR: STS1 not cleared: 0x%08X\n", rdata_grp);
#endif
        test_err++;
    }

    /* Clear system raw in LSS_SYSREG and re-enable group */
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, (1U << 0));
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);

    /* Clear GIC IRQ */
    GIC_ClearIRQ(IRQ_GPIO0);
}

void Default_IRQHandler(void)
{
#ifdef DEBUG_DISPLAY
    printf("[ISR] Default_IRQHandler entered (i=%u)\n", cur_i);
#endif
    int_pend = 0U;
    isr_clear_and_reenable();
}

void test_case(void)
{
    test_err = 0U;
#ifdef DEBUG_DISPLAY
    printf("[test_gpio_pedge_all_pads_en] Start\n");
#endif

    /* Enable GIC and SYSREG for GPIO0 */
    GIC_EnableIRQ(IRQ_GPIO0);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, (1U << 0));

    /* Configure posedge per pin: 0x00020000 */
    for (unsigned int i = 0; i < 32U; i++) {
        const unsigned long addr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4UL));
        write_reg(addr, 0x00020000U);
    }
    wait_on(10U);

    /* Configure IO control groups */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFU);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFU);
    wait_on(10U);

    /* Enable group interrupts */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFU);

    /* For each pin: generate rising edge and wait for ISR */
    for (unsigned int i = 0; i < 32U; i++) {
        cur_i = i;
        /* Drive low then high to create rising edge */
        write_reg(PAD_STIM, 0x00000000U);
        wait_on(10U);
        int_pend = 1U;
        write_reg(PAD_STIM, 0xFFFFFFFFU);

        unsigned int to = 2000U;
        while ((int_pend != 0U) && (to-- > 0U)) {
            wait_on(10U);
        }
        if (int_pend != 0U) {
#ifdef DEBUG_DISPLAY
            printf("[ERROR] Timeout waiting for ISR (i=%u)\n", i);
#endif
            test_err++;
            /* Fallback clear */
            isr_clear_and_reenable();
            int_pend = 0U;
        }

        /* Drive low again before next iteration */
        write_reg(PAD_STIM, 0x00000000U);
        wait_on(10U);
    }

#ifdef DEBUG_DISPLAY
    printf("[test_gpio_pedge_all_pads_en] Done test_err=%u\n", test_err);
#endif

    finish((test_err == 0U) ? 0 : 1);
}
