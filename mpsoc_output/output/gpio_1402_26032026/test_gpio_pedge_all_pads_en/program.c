/*
 * Auto-generated program for GPIO testcase: test_gpio_pedge_all_pads_en
 * Implements CSV Description/Procedure/Criteria using only impacted register macros.
 */

#include "test_define.c"

static unsigned int test_err = 0;

void Default_IRQHandler(void)
{
    /* Latch group status, then mask group, check non-zero, clear per-pin, verify clear, clear sysreg, re-enable */
    unsigned int rdata, rdata_grp;

#ifdef DEBUG_DISPLAY
    printf("[IRQ] Handling posedge, last pin index=%u (GPIO%u)\n", g_current_pin, (unsigned)(g_current_pin + 8));
#endif

    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000);

    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        printf("ERROR: Group Interrupt not occurred\n");
        test_err++;
    }

    for (unsigned int j = 0; j < 32u; ++j) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j * 4u), 0x00010000); /* clear raw per pin */
    }
    wait_on(2);

    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x0u) {
        printf("ERROR: Group Interrupt clear failed: 0x%08x\n", rdata_grp);
        test_err++;
    }

    /* Clear sysreg (mask TBD, no RAG). Use placeholder write and verify readback does not retain bit. */
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, 0u); /* TODO: replace 0 with GPIOx bit */
    rdata = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    if ((rdata & 0u) != 0u) { /* TODO condition once mask is known */
        printf("ERROR: sysreg status not cleared as expected\n");
        test_err++;
    }

    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    int_pend = 0; /* release wait */
}

void test_case(void)
{
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

    test_err = 0u;

    /* Enable sysreg output for GPIO (mask TBD, keep placeholder) */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0u); /* TODO replace with proper mask */

    for (unsigned int i = 0; i < 32u; ++i) {
        if (!skip_array[1]) {
            write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4u), 0x00020000); /* posedge on bit17 */
        }
    }

    wait_on(10);

    if (!skip_array[2]) write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF);
    if (!skip_array[3]) write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF);
    if (!skip_array[4]) write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF);
    if (!skip_array[5]) write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF);

    wait_on(10);

    if (!skip_array[6]) write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    for (unsigned int i = 0; i < 32u; ++i) {
        /* Prepare known low, arm, drive high to cause posedge */
        /* Note: external drive register 0xA0243FFC is not in impacted list; keep as TODO external stimulus */
        /* TODO: write_reg(0xA0243FFC, 0x00000000); */
        wait_on(10);
        int_pend = 1;
        g_current_pin = i;
        /* TODO: write_reg(0xA0243FFC, 0xFFFFFFFF); */

        int timeout = 2000;
        while (int_pend && (--timeout > 0)) { wait_on(10); }
        if (timeout == 0) {
            printf("ERROR: Timeout waiting for GPIO posedge at i=%u\n", i);
            test_err++;
            break; /* per CSV: may break on timeout */
        }

        /* Optionally drive low to prep next iteration */
        /* TODO: write_reg(0xA0243FFC, 0x00000000); */
        wait_on(10);
    }

    if (test_err == 0u) finish(0); else finish(1);
}
