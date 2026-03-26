#define DEBUG_DISPLAY 1
#include "test_define.c"

/*
 * Test: test_gpio_pedge_all_pads_en
 * Goal: Configure posedge interrupt on all pads, set IO control groups, enable group intr,
 *       generate rising edges per pin, validate ISR sees group status, clear raw + verify 0.
 */

volatile int int_pend = 0;
static int test_err = 0;

void Default_IRQHandler(void);

void test_case(void)
{
    unsigned int i;

#ifdef GPIO0
    GIC_EnableIRQ(87);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* Enable posedge detection for all 32 pins on GPIO0 */
    for (i = 0; i < 32; ++i) {
        write_reg(gpio0_pin_cfg_base + (i * 4u), 0x00020000u);
    }
    wait_on(10);

    /* Configure IO control groups to enable pads */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    /* Enable all group interrupts */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    for (i = 0; i < 32; ++i) {
        unsigned int wr_val = (1u << i);
        /* Drive low then high to create rising edge */
        write_reg(0xA0243FFC, 0x00000000u);
        wait_on(10);
        int_pend = 1;
        write_reg(0xA0243FFC, 0xFFFFFFFFu);

        /* Poll with timeout for ISR */
        unsigned int to;
        for (to = 0; to < 2000u; ++to) {
            wait_on(10);
            if (int_pend == 0)
                break;
        }
        if (int_pend != 0) {
#ifdef DEBUG_DISPLAY
            printf("TIMEOUT: pedge pin %u\n", i);
#endif
            test_err++;
        }

        /* Prepare for next iteration */
        write_reg(0xA0243FFC, 0x00000000u);
        wait_on(10);
    }

    finish(test_err);
}

void Default_IRQHandler(void)
{
    extern unsigned int i;
    unsigned int wr_val = (1u << i);

    int_pend = 0;

    /* Group status should be non-zero on entry */
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
#ifdef DEBUG_DISPLAY
        printf("ISR: pedge no group status set for pin %u\n", i);
#endif
        test_err++;
    }

    /* Mask group, clear per-pin raw via per-pin writes (0x00010000), then verify clear */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);
    for (unsigned int j = 0; j < 32; ++j) {
        write_reg(gpio0_pin_cfg_base + (j * 4u), 0x00010000u);
    }
    wait_on(2);

    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0u) {
#ifdef DEBUG_DISPLAY
        printf("ISR: pedge group status not cleared: 0x%08X\n", rdata_grp);
#endif
        test_err++;
    }

#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    GIC_ClearIRQ(88);
#endif

    /* Re-enable group for next pin */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
}
