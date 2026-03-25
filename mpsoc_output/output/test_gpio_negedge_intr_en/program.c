// Auto-generated program.c for test_gpio_negedge_intr_en
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

    // Pre-drive stimulus high as per CSV before generating edges
    // CSV uses a magic address 0xA0243FFC to create edges.
    // Using as-is because it is part of the CSV procedure.
    // If your environment abstracts this, replace with the appropriate driver call.
    // write_reg(0xA0243FFC, 0xFFFFFFFFu);

    // Step 2: Per-pin configuration for negative-edge interrupt enable
    for (unsigned i = 0; i < 32; ++i) {
        unsigned int addr = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        // Bits per CSV: (1u<<20)|(1u<<18)|(1u<<16)
        write_reg(addr, (1u<<20) | (1u<<18) | (1u<<16));
        wait_cycles(100);
    }

    // Step 3: For each pin, clear RAW, enable INTR_EN1 bit, then create a negative edge and poll int_pend
    for (unsigned i = 0; i < 32; ++i) {
        unsigned int wr_val = (1u << i);

        // Clear raw pending for pin i
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);

        // Enable interrupt for pin i
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_cycles(100);

        // Set pending flag and generate a negative edge per CSV
        int_pend = 1;

        // Drive high, then drive the selected pin low using the stimulus register
        // NOTE: The stimulus address is referenced in CSV but not listed under impacted registers; kept as-is for fidelity.
        write_reg(0xA0243FFCu, 0xFFFFFFFFu); // drive high
        wait_cycles(100);
        write_reg(0xA0243FFCu, ~wr_val);     // drop selected bit to create negedge

        // Poll for ISR to clear int_pend
        unsigned int timeout = 0;
        while (int_pend && timeout < 1000000u) {
            timeout++;
        }
        if (int_pend) {
#ifdef DEBUG_DISPLAY
            DEBUG_DISPLAY("ERROR: Negedge interrupt timeout on pin %u\n", i);
#else
            printf("ERROR: Negedge interrupt timeout on pin %u\n", i);
#endif
            test_err++;
            // Clear for safety to avoid cascading timeouts
            int_pend = 0;
        }
    }

    // Finalize per acceptance criteria
    if (test_err == 0) {
        finish(0);
    } else {
        finish(1);
    }
}

void Default_IRQHandler(void) {
    // CSV handler semantics
    int_pend = 0;

    for (unsigned i = 0; i < 32; ++i) {
        unsigned int addr = (unsigned int)(MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        unsigned int rdata = read_reg(addr);

        // For negedge path, input level 0 is expected (CSV check)
        if ((rdata & 0x1u) != 0u) {
#ifdef DEBUG_DISPLAY
            DEBUG_DISPLAY("ERROR: Pin %u level bit indicates 1 on negedge path\n", i);
#else
            printf("ERROR: Pin %u level bit indicates 1 on negedge path\n", i);
#endif
            test_err++;
        }

        // If interrupt latch bit is set, verify group status contains this pin, then clear
        if ((rdata & 0x2u) != 0u) {
            unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if ((rdata_grp & (1u << i)) == 0u) {
#ifdef DEBUG_DISPLAY
                DEBUG_DISPLAY("ERROR: Group status missing pin %u\n", i);
#else
                printf("ERROR: Group status missing pin %u\n", i);
#endif
                test_err++;
            }

            // Re-arm selected bits per CSV: (1u<<20)|(1u<<16)
            write_reg(addr, (1u<<20) | (1u<<16));

            // Acknowledge raw status for this pin
            write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, (1u << i));

            // Verify group cleared to 0
            rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
            if (rdata_grp != 0u) {
#ifdef DEBUG_DISPLAY
                DEBUG_DISPLAY("ERROR: Group status not cleared after STCLR1 (val=0x%08X)\n", rdata_grp);
#else
                printf("ERROR: Group status not cleared after STCLR1 (val=0x%08X)\n", rdata_grp);
#endif
                test_err++;
            }
        }
    }

    // System-level raw clear/ack (mask not specified in CSV/RAG)
    // TODO: write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, <appropriate_mask>);
    // TODO: GIC_ClearIRQ(87/88) if applicable
}
