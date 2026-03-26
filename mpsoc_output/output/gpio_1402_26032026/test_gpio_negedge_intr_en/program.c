/*
 * Auto-generated program for GPIO testcase: test_gpio_negedge_intr_en
 * Implements CSV Description/Procedure/Criteria using only impacted register macros.
 */

#include "test_define.c"

/* Local test error counter */
static unsigned int test_err = 0;

/* Default IRQ Handler: validates status, clears raw/group/sysreg per CSV */
void Default_IRQHandler(void)
{
    /* Use the last armed pin index */
#ifdef DEBUG_DISPLAY
    printf("[IRQ] Handling negedge for pin index=%u (GPIO%u)\n", g_current_pin, (unsigned)(g_current_pin + 8));
#endif

    /* Read back per-pin config/state register */
    unsigned int raddr = MIZAR_GPIO_GP0_GPIO_8 + (g_current_pin * 4u);
    unsigned int rdata = read_reg(raddr);

    /* Acceptance checks from CSV:
       - (rdata & 0x1) == 0
       - (rdata & 0x2) != 0
    */
    if ((rdata & 0x1u) != 0u) { test_err++; }
    if ((rdata & 0x2u) == 0u) { test_err++; }

    /* Verify group status bit for the active pin is set before clearing */
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if ((rdata_grp & (1u << g_current_pin)) == 0u) { test_err++; }

    /* Acknowledge/clear: per-pin raw status and group */
    write_reg(raddr, (1u << 20) | (1u << 16));             /* mask re-arm per-pin per CSV */
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, (1u << g_current_pin));

    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0x0u) { test_err++; }

    /* Acknowledge sysreg: mask TBD (no RAG data). Use placeholder write. */
    /* TODO: replace 0 with the correct LSS_SYSREG_RAW_STCR1 GPIOx bit */
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, 0u);

    /* Clear pending flag to release the wait loop */
    int_pend = 0;
}

void test_case(void)
{
    test_err = 0u;

    /* Enable LSS SYSREG GPIO interrupt output (mask TBD, no RAG data) */
    /* TODO: replace 0 with (LSS_SYSREG_INTR_EN1_GPIO0_INTR | LSS_SYSREG_INTR_EN1_GPIO1_INTR) as applicable */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, 0u);

    /* Configure each pin (GPIO8..GPIO39): negedge enable + other required bits per CSV */
    for (unsigned int i = 0; i < 32u; ++i) {
        unsigned int addr1 = MIZAR_GPIO_GP0_GPIO_8 + (i * 4u);
        if (!skip_array[1]) { /* GP0_GPIO_8 family */
            write_reg(addr1, (1u << 20) | (1u << 18) | (1u << 16));
        }
        wait_on(10);
    }

    /* Iterate through each pin: clear raw, enable per-pin interrupt, then create an edge and wait for ISR */
    for (unsigned int i = 0; i < 32u; ++i) {
        unsigned int wr_val = (1u << i);

        if (!skip_array[2]) { write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); }
        if (!skip_array[3]) { write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); }
        wait_on(10);

        g_current_pin = i;
        int_pend = 1; /* armed before driving edge */

        /* TODO: Generate falling edge on GPIO pin i (CSV mentions toggling external register 0xA0243FFC). */
        /* Not used here since it is not part of impacted register macros. Provide external stimulus. */

        /* Poll with timeout per CSV acceptance (no timeout expected) */
        unsigned int timeout = 5000u;
        while (int_pend && timeout--) { wait_on(10); }
        if (timeout == 0u) {
#ifdef DEBUG_DISPLAY
            printf("ERROR: Timeout waiting for GPIO%u negedge interrupt\n", (unsigned)(i + 8));
#endif
            test_err++;
        }
    }

    /* Finish per acceptance: pass if no errors, else fail */
    if (test_err == 0u) { finish(0); } else { finish(1); }
}
