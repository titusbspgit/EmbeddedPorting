// Auto-generated program.c for test_gpio_pedge_all_pads_en
// Follows the provided template structure. Does not invent registers/macros.
// Implements CSV procedure and acceptance criteria using only impacted register macros.

#include "test_define.c"

volatile int int_pend = 0;
int test_err = 0;

static inline void wait_cycles(volatile unsigned int n){ while(n--) __asm__ volatile("nop"); }

void test_case(void) {
    // Step 0: Optional IRQ routing (left as TODO if platform mapping is unknown)
    // TODO: GIC_EnableIRQ(87); // GPIO0
    // TODO: GIC_EnableIRQ(88); // GPIO1

    // Step 1: Enable system-level GPIO interrupts
    // CSV: "Enable MIZAR_LSS_SYSREG_INTR_EN1 for GPIO interrupts"
    // NOTE: Exact bit mask not provided in CSV/Helper/RAG; caller/toolchain headers should define it.
    // TODO: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIOx_INTR);

    // Step 2: Configure posedge on per-pin control registers
    for (unsigned i = 0; i < 32; ++i) {
        unsigned int addr = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr, 0x00020000u); // posedge per CSV
        wait_cycles(100);
    }

    // Step 3: Configure GPIO IO control groups to allow inputs
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_cycles(100);

    // Step 4: Enable group interrupt enables
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    // Step 5: For each pin, generate a rising edge and poll int_pend
    for (unsigned i = 0; i < 32; ++i) {
        // Drive low then high using stimulus address per CSV
        write_reg(0xA0243FFCu, 0x00000000u); // low
        wait_cycles(100);
        int_pend = 1;
        write_reg(0xA0243FFCu, 0xFFFFFFFFu); // high (posedge)

        unsigned int timeout = 0;
        while (int_pend && timeout < 1000000u) {
            timeout++;
        }
        if (int_pend) {
#ifdef DEBUG_DISPLAY
            DEBUG_DISPLAY("ERROR: Posedge interrupt timeout on pin %u\n", i);
#else
            printf("ERROR: Posedge interrupt timeout on pin %u\n", i);
#endif
            test_err++;
            int_pend = 0; // avoid cascading timeouts
        }

        // Drive low again
        write_reg(0xA0243FFCu, 0x00000000u);
        wait_cycles(100);
    }

    // Finalize
    if (test_err == 0) {
        finish(0);
    } else {
        finish(1);
    }
}

void Default_IRQHandler(void) {
    // CSV handler semantics
    int_pend = 0;

    // Latch group status and then mask enables
    unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
#ifdef DEBUG_DISPLAY
        DEBUG_DISPLAY("ERROR: No pins set in group status on posedge IRQ\n");
#else
        printf("ERROR: No pins set in group status on posedge IRQ\n");
#endif
        test_err++;
    }

    // Clear per-pin: write per-pin control to clear latch per CSV
    for (unsigned j = 0; j < 32; ++j) {
        unsigned int addr = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
        write_reg(addr, 0x00010000u); // per CSV handler clear
        wait_cycles(100);
    }

    // Verify group cleared
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0u) {
#ifdef DEBUG_DISPLAY
        DEBUG_DISPLAY("ERROR: Group status not cleared after per-pin clear (val=0x%08X)\n", rdata_grp);
#else
        printf("ERROR: Group status not cleared after per-pin clear (val=0x%08X)\n", rdata_grp);
#endif
        test_err++;
    }

    // System-level raw clear/ack (mask not specified in CSV/RAG)
    // TODO: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, <appropriate_mask>);

    // Re-enable group enables
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    // Optional GIC clear
    // TODO: GIC_ClearIRQ(87/88)
}
