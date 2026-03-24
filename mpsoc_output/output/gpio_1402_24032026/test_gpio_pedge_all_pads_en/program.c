#include "test_define.c"

static volatile int int_pend = 0;
static volatile int test_err = 0;
static volatile int current_pin = -1;

void Default_IRQHandler(void)
{
    unsigned int grp, grp2;
    int_pend = 0; // interrupt observed

#ifdef DEBUG_DISPLAY
    printf("IRQ: Handling posedge GPIO event\n");
#endif

    // Read and then mask group interrupts
    grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000);

    if ((grp & 0xFFFFFFFF) == 0) {
        printf("ERROR: No bits set in group status on IRQ\n");
        test_err++;
    }

    // Clear per-pin raw by writing per-pin 0x00010000 for each, then verify clear
    for (int j = 0; j < 32; j++) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j * 4), 0x00010000);
        wait_on(1);
    }

    grp2 = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (grp2 != 0x0) {
        printf("ERROR: Group status not cleared after per-pin clear (0x%08X)\n", grp2);
        test_err++;
    }

    // Clear sysreg raw and re-enable group
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, 0xFFFFFFFF);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF);
    GIC_ClearIRQ(87);
    GIC_ClearIRQ(88);
}

void test_case(void)
{
    unsigned int i;

#ifdef DEBUG_DISPLAY
    printf("Starting testcase: test_gpio_pedge_all_pads_en\n");
#endif

    // Enable IRQs
    GIC_EnableIRQ(87);
    GIC_EnableIRQ(88);

    // Enable system-level GPIO interrupts
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0xFFFFFFFF);

    // Configure posedge per-pad
    for (i = 0; i < 32; i++) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00020000);
        wait_on(1);
    }

    // Configure IO control groups for outputs
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x00FF00FF);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x00FF00FF);
    wait_on(10);

    // Enable all group interrupts
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF);

    // For each pin generate rising edge
    for (i = 0; i < 32; i++) {
        // Drive low then high
        write_reg(0xA0243FFC, 0x00000000);
        wait_on(10);
        int_pend = 1;
        current_pin = (int)i;
        write_reg(0xA0243FFC, 0xFFFFFFFF);

        // Poll with timeout for ISR to clear int_pend
        unsigned int t = 0;
        while (int_pend && t < 100000) { t++; }
        if (int_pend) {
            printf("ERROR: Timeout waiting for posedge interrupt on pin %u\n", i);
            test_err++;
            int_pend = 0;
            break; // Per CSV: break on timeout
        }

        // Drive low again before next iteration
        write_reg(0xA0243FFC, 0x00000000);
        wait_on(5);
    }

#ifdef DEBUG_DISPLAY
    printf("Completed testcase: test_gpio_pedge_all_pads_en (errors=%d)\n", test_err);
#endif

    if (test_err == 0) finish(0); else finish(1);
}
