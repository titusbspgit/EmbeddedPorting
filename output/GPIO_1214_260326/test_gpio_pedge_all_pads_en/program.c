#include "test_define.c"

// Test: test_gpio_pedge_all_pads_en
// Description: Enable group interrupt output, configure posedge per pin and IO_CTRL groups, enable group interrupts,
// then stimulate rising edges and validate via ISR; clear raw status and re-enable group as required.

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
    sys_en |= (1u << 0); // assumed group bit for GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, sys_en);

    // Configure posedge per-pin and IO control groups
    for (unsigned int i = 0; i < 32u; i++) {
        unsigned long addr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4u));
        write_reg(addr, 0x00020000u);
#ifdef DEBUG_DISPLAY
        printf("[CFG] POS-EDGE i=%u CTRL(addr=0x%08lX) <= 0x00020000\n", i, addr);
#endif
    }

    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
#ifdef DEBUG_DISPLAY
    printf("[CFG] IO_CTRL_GROUP[1..4] <= 0x000000FF\n");
#endif

    // Enable group interrupt outputs
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
#ifdef DEBUG_DISPLAY
    printf("[EN] EN1 <= 0xFFFFFFFF\n");
#endif

    for (unsigned int i = 0; i < 32u; i++) {
        // Drive low, then high to create rising edge on bit i
        write_reg(0xA0243FFCu, 0x00000000u);
        wait_on(10);
        int_pend = 1; cur_i = i;
        write_reg(0xA0243FFCu, 0xFFFFFFFFu);
#ifdef DEBUG_DISPLAY
        printf("[STIM] PAD 0xA0243FFC: 0x00000000 -> 0xFFFFFFFF (posedge on bit %u)\n", i);
#endif

        unsigned int to = 2000u;
        while ((int_pend != 0u) && (to-- > 0u)) {
            wait_on(10);
        }
        if (int_pend != 0u) {
#ifdef DEBUG_DISPLAY
            printf("[ERR] Timeout waiting for ISR, i=%u\n", i);
#endif
            test_err++;
        }

        // Ensure we drive low after handling to prep next
        write_reg(0xA0243FFCu, 0x00000000u);
        wait_on(10);
    }

#ifdef DEBUG_DISPLAY
    printf("[DONE] Errors=%u\n", test_err);
#endif

    finish((test_err == 0u) ? 0 : 1);
}

void Default_IRQHandler(void)
{
    unsigned int i = (unsigned int)cur_i;
    unsigned int local_wr = (1u << i);
    int_pend = 0;

    // Expect non-zero group status on entry
    unsigned int sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if ((sts & 0xFFFFFFFFu) == 0u) {
#ifdef DEBUG_DISPLAY
        printf("[ISR_ERR] STS1 zero on entry (i=%u)\n", i);
#endif
        test_err++;
    }

    // Mask group, clear per-pin raw via 0x00010000, verify group clears, then clear system and re-enable group
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    for (unsigned int j = 0; j < 32u; j++) {
        unsigned long addr = (MIZAR_GPIO_GP0_GPIO_8 + (j * 4u));
        write_reg(addr, 0x00010000u);
    }

    wait_on(2);
    sts = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (sts != 0x00000000u) {
#ifdef DEBUG_DISPLAY
        printf("[ISR_ERR] STS1 not cleared after per-pin RAW clear (STS1=0x%08X)\n", sts);
#endif
        test_err++;
    }

    // Clear system RAW status and re-enable
    unsigned int raw = read_reg(MIZAR_LSS_SYSREG_RAW_STCR1);
    raw |= (1u << 0); // assumed GPIO0 bit
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, raw);

    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
#ifdef GPIO0
    GIC_ClearIRQ(87);
#endif
}
