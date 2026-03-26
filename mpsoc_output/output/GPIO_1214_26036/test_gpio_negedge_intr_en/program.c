#include "test_define.c"

/*
 * test_gpio_negedge_intr_en
 * High-level: Enable group interrupt, configure per-pin negedge + enable intr per bit, 
 * drive falling edge via 0xA0243FFC, wait for ISR to clear int_pend, validate group status.
 */

static volatile unsigned int int_pend = 0u;
static int test_err = 0;

static void drive_pad_pattern(unsigned int mask)
{
    // 0xA0243FFC is a literal pad driver register per CSV
    write_reg(0xA0243FFCu, 0xFFFFFFFFu);
    wait_on(1);
    write_reg(0xA0243FFCu, ~mask);
}

void Default_IRQHandler(void)
{
    // Note: "i" loop index is implicit from main context; handle per-bit via status readback
    unsigned int sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (sts == 0u) {
        test_err++;
        goto exit_isr;
    }

    // For each bit set in sts, clear per-pin raw via GPIO_8+(i*4) using ctrl bits (keep posedge disabled)
    for (unsigned int i = 0; i < 32; ++i) {
        unsigned int bit = (1u << i);
        if ((sts & bit) == 0u) continue;
        unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        // keep (1u<<20) raw clear, (1u<<16) enable stays as needed; ensure negedge bit (1u<<18) configured separately in setup
        write_reg(raddr, ((1u<<20) | (1u<<16)));
    }

    // Clear group raw
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, sts);

    // Post-clear readback should be 0
    if (read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1) != 0u) {
        test_err++;
    }

    // Clear SYSREG raw (GPIO0/1 intr) per CSV narrative — symbolic values expected in headers
    // These symbol macros must be provided by headers; if not, this becomes a build-time dependency
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, sts);

exit_isr:
    int_pend = 0u;
}

void test_case(void)
{
    // Optionally enable GIC IRQ numbers as per platform; assume helpers/macros exist
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

    // Enable group interrupt output
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0x1u);

    // Initialize pad driver high
    write_reg(0xA0243FFCu, 0xFFFFFFFFu);

    // Program per-pin control for negedge and enable per-bit interrupts
    for (unsigned int i = 0; i < 32; ++i) {
        unsigned long addr1 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr1, ((1u<<20) | (1u<<18) | (1u<<16)));
        wait_on(1);
    }

    // Per-pin operation
    for (unsigned int i = 0; i < 32; ++i) {
        unsigned int wr = (1u << i);
        // Clear raw and enable only targeted bit
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr);
        wait_on(1);

        int_pend = 1u;
        drive_pad_pattern(wr); // create falling edge on selected bit

        // Poll with timeout for ISR to clear int_pend
        for (unsigned int t = 0; t < 5000u && int_pend != 0u; ++t) {
            wait_on(1);
        }
        if (int_pend != 0u) {
            test_err++;
            int_pend = 0u; // force clear to proceed
        }
    }

#ifdef GPIO0
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    GIC_ClearIRQ(88);
#endif

    finish(test_err ? 1 : 0);
}
