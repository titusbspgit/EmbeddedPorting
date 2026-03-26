#include "test_define.c"

/*
 * test_gpio_pedge_all_pads_en
 * High-level: Enable group interrupt, configure posedge on all pads, set IO_CTRL groups,
 * enable group EN1, generate rising edges, handle ISR to clear and confirm group status.
 */

static volatile unsigned int int_pend = 0u;
static int test_err = 0;

static void drive_rising_edge(void)
{
    // Create a rising transition on all pads via literal driver at 0xA0243FFC
    write_reg(0xA0243FFCu, 0x00000000u);
    wait_on(1);
    int_pend = 1u;
    write_reg(0xA0243FFCu, 0xFFFFFFFFu);
}

void Default_IRQHandler(void)
{
    unsigned int sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);

    // Temporarily mask group enable
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    if (sts == 0u) {
        test_err++;
    } else {
        // Clear per-pin raw via control register writes
        for (unsigned int j = 0; j < 32; ++j) {
            unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
            write_reg(raddr, 0x00010000u); // clear raw per-pin per CSV
        }
        wait_on(2);

        // Group status should be zero after clears
        if (read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) != 0u) {
            test_err++;
        }

        // Clear SYSREG raw and confirm
        write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, sts);
    }

    // Re-enable group
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    int_pend = 0u;
}

void test_case(void)
{
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

    // Enable group interrupt output
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0x1u);

    // Configure posedge per-pin
    for (unsigned int i = 0; i < 32; ++i) {
        write_reg((MIZAR_GPIO_GP0_GPIO_8 + (i * 4u)), 0x00020000u);
    }
    wait_on(1);

    // IO control groups configuration: set to 0x000000FF per CSV
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(1);

    // Enable group EN1
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    // For each i, generate rising edge and wait for ISR
    for (unsigned int i = 0; i < 32; ++i) {
        drive_rising_edge();
        // Poll with timeout for ISR to clear int_pend
        for (unsigned int t = 0; t < 2000u && int_pend != 0u; ++t) {
            wait_on(1);
        }
        if (int_pend != 0u) {
            test_err++;
            int_pend = 0u; // force clear
        }
        // Drive low again before next iteration
        write_reg(0xA0243FFCu, 0x00000000u);
        wait_on(1);
    }

#ifdef GPIO0
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    GIC_ClearIRQ(88);
#endif

    finish(test_err ? 1 : 0);
}
