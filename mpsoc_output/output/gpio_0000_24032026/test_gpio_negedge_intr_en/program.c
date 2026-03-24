#include "test_define.c"

static volatile int int_pend = 0;
static volatile int test_err = 0;
static volatile int current_pin = -1;

void Default_IRQHandler(void)
{
    int i = current_pin;
    unsigned int rdata;
    unsigned int grp;
    unsigned int grp2;

    int_pend = 0; // interrupt observed

    if (i < 0)
        return;

#ifdef DEBUG_DISPLAY
    printf("IRQ: Handling negedge GPIO pin %d\n", i);
#endif

    // Read per-pin config/state
    rdata = read_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4));

    // Acceptance: (rdata & 0x1) == 0
    if ((rdata & 0x1) != 0) {
        printf("ERROR: GPIO_%d rdata & 0x1 not zero (0x%08X)\n", i, rdata);
        test_err++;
    }

    // Acceptance: (rdata & 0x2) != 0
    if ((rdata & 0x2) == 0) {
        printf("ERROR: GPIO_%d rdata & 0x2 is zero (0x%08X)\n", i, rdata);
        test_err++;
    } else {
        // Verify group status bit set for this pin
        grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((grp & (1u << i)) == 0) {
            printf("ERROR: GPIO_%d group status bit not set (grp=0x%08X)\n", i, grp);
            test_err++;
        }

        // Clear per-pin raw status and leave enable/type bits as needed
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), ((1u << 20) | (1u << 16)));
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, (1u << i));

        // After clear, group status should be 0
        grp2 = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (grp2 != 0x0) {
            printf("ERROR: GPIO group status not cleared (0x%08X)\n", grp2);
            test_err++;
        }
    }

    // Acknowledge at system register level and clear GIC
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, 0xFFFFFFFF);
    GIC_ClearIRQ(87);
    GIC_ClearIRQ(88);
}

void test_case(void)
{
    unsigned int i;

#ifdef DEBUG_DISPLAY
    printf("Starting testcase: test_gpio_negedge_intr_en\n");
#endif

    // Optionally enable IRQs
    GIC_EnableIRQ(87);
    GIC_EnableIRQ(88);

    // Enable system-level GPIO interrupts
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIOx_INTR);

    // Pre-drive high
    write_reg(0xA0243FFC, 0xFFFFFFFF);

    // Configure each pad: negedge + enable + unmask: (1<<20)|(1<<18)|(1<<16)
    for (i = 0; i < 32; i++) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), ((1u << 20) | (1u << 18) | (1u << 16)));
        wait_on(1);
    }

    // Exercise each pin
    for (i = 0; i < 32; i++) {
        unsigned int wr_val = (1u << i);

        // Clear any raw pending and enable just this pin in the group
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        // Generate a negedge on this pin
        int_pend = 1;
        current_pin = (int)i;
        write_reg(0xA0243FFC, 0xFFFFFFFF);
        wait_on(10);
        write_reg(0xA0243FFC, ~wr_val);

        // Poll with timeout for ISR to clear int_pend
        unsigned int t = 0;
        while (int_pend && t < 100000) { t++; }
        if (int_pend) {
            printf("ERROR: Timeout waiting for negedge interrupt on pin %u\n", i);
            test_err++;
            int_pend = 0;
        }
    }

#ifdef DEBUG_DISPLAY
    printf("Completed testcase: test_gpio_negedge_intr_en (errors=%d)\n", test_err);
#endif

    if (test_err == 0) finish(0); else finish(1);
}
