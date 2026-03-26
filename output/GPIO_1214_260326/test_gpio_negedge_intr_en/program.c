#include "test_define.c"

// Test: test_gpio_negedge_intr_en
// Description: Enable group interrupt output, configure per-pin control for negedge detection, clear RAW, enable per-bit,
// then stimulate a negedge on each pin and validate via ISR that status is set/cleared correctly.

static volatile unsigned int int_pend = 0;
static volatile unsigned int cur_i = 0;
static volatile unsigned int test_err = 0;

void test_case(void)
{
#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif

    // Enable System-level GPIO0 interrupt output
#ifdef DEBUG_DISPLAY
    printf("[SETUP] Enable SYSREG GPIO0 group interrupt via MIZAR_LSS_SYSREG_INTR_EN1\n");
#endif
    unsigned int sys_en = read_reg(MIZAR_LSS_SYSREG_INTR_EN1);
    sys_en |= (1u << 0); // Note: exact bit macro not provided; enabling assumed group bit
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, sys_en);

    // Configure per-pin control: (1u<<20)|(1u<<18)|(1u<<16) for negedge as per CSV
    for (unsigned int i = 0; i < 32u; i++) {
        unsigned long addr1 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr1, ((1u<<20)|(1u<<18)|(1u<<16)));
#ifdef DEBUG_DISPLAY
        printf("[CFG] i=%u CTRL(addr=0x%08lX) <= 0x%08X\n", i, addr1, ((1u<<20)|(1u<<18)|(1u<<16)));
#endif
        wait_on(10);
    }

    // Clear RAW and enable per-pin interrupts
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, 0xFFFFFFFFu);
#ifdef DEBUG_DISPLAY
    printf("[CFG] RAW_STCLR1 <= 0xFFFFFFFF\n");
#endif

    // Iterate pins: enable bit, generate negedge via PAD_STIM writes, wait for ISR to clear int_pend
    for (unsigned int i = 0; i < 32u; i++) {
        unsigned int wr_val = (1u << i);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); // enable only this bit
#ifdef DEBUG_DISPLAY
        printf("[EN] EN1 <= 0x%08X (i=%u)\n", wr_val, i);
#endif
        wait_on(10);

        // Stimulus: drive high then low to create a falling edge for bit i
        int_pend = 1; cur_i = i;
        write_reg(0xA0243FFCu, 0xFFFFFFFFu);
        write_reg(0xA0243FFCu, (~wr_val));
#ifdef DEBUG_DISPLAY
        printf("[STIM] PAD 0xA0243FFC: 0xFFFFFFFF -> 0x%08X (negedge on bit %u)\n", (~wr_val), i);
#endif

        // Poll with timeout until ISR clears int_pend
        unsigned int to = 5000u;
        while ((int_pend != 0u) && (to-- > 0u)) {
            wait_on(10);
        }
        if (int_pend != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] Timeout waiting for ISR, i=%u\n", i);
#endif
            test_err++;
            // attempt cleanup
            write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);
        }

        // Disable bit before next iteration
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);
    }

#ifdef DEBUG_DISPLAY
    printf("[DONE] Errors=%u\n", test_err);
#endif

    finish((test_err == 0u) ? 0 : 1);
}

void Default_IRQHandler(void)
{
    // Capture current pin index and compute mask
    unsigned int i = (unsigned int)cur_i;
    unsigned int local_wr = (1u << i);
    int_pend = 0; // signal handled

    // Validate group status reflects the interrupt
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if ((rdata_grp & local_wr) == 0u) {
#ifdef DEBUG_DISPLAY
        printf("[ISR_ERR] STS1 bit not set for i=%u (STS1=0x%08X)\n", i, rdata_grp);
#endif
        test_err++;
    }

    // Clear per-pin raw via control register and RAW_STCLR1, then verify STS1 clears
    unsigned long ctrl_addr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
    write_reg(ctrl_addr, ((1u<<20)|(1u<<16))); // clear edge bits per CSV
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);
#ifdef DEBUG_DISPLAY
    printf("[ISR_CLR] CTRL(addr=0x%08lX)<=0x%08X; RAW_STCLR1<=0x%08X\n", ctrl_addr, ((1u<<20)|(1u<<16)), local_wr);
#endif

    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if ((rdata_grp & local_wr) != 0u) {
#ifdef DEBUG_DISPLAY
        printf("[ISR_ERR] STS1 not cleared for i=%u (STS1=0x%08X)\n", i, rdata_grp);
#endif
        test_err++;
    }

    // Clear system RAW status and GIC IRQ
    unsigned int raw_clr = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    raw_clr |= (1u << 0); // assumed GPIO0 raw clear bit
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, raw_clr);
#ifdef GPIO0
    GIC_ClearIRQ(87);
#endif
}
