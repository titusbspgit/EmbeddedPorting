#include "test_define.c"

// Generated test: test_gpio_negedge_intr_en
// Description: Negative-edge interrupt enable and servicing across GPIO[8..39];
// Includes system interrupt enable, per-pin configuration, raw status clear,
// group status verification, and system/GIC clears.

#include <stdint.h>

static volatile unsigned int int_pend = 0;   // Armed before edge; cleared in ISR
static volatile unsigned int cur_idx  = 0;   // Current pin index (0..31)
static volatile unsigned int last_wr_val = 0; // One-hot for cur_idx
static volatile int test_err = 0;            // Accumulates errors

// Optional external GIC APIs if available in BSP
extern void GIC_EnableIRQ(unsigned int irq);
extern void GIC_ClearPendingIRQ(unsigned int irq);

static inline void brief_wait(void) { wait_on(1); }

static inline unsigned int reg_read(unsigned long int addr)
{
    return *(volatile unsigned int *)(addr);
}

static inline void reg_write(unsigned long int addr, unsigned int val)
{
    *(volatile unsigned int *)(addr) = val;
}

void Default_IRQHandler(void)
{
    // Clear SW pending flag first
    int_pend = 0;

    // Read per-pin register for current index
    unsigned long int pin_addr = addr_array[cur_idx];
    unsigned int rdata = reg_read(pin_addr);

    // Expect level bit low after negedge: (rdata & 0x1) == 0
    if ((rdata & 0x1u) != 0u) {
        test_err++;
        DEBUG_DISPLAY("ERR[%u]: Level bit expected 0 after negedge, got 1 (rdata=0x%08X)\n", cur_idx, rdata);
    }

    // Expect per-pin interrupt status asserted: (rdata & 0x2) != 0
    if ((rdata & 0x2u) == 0u) {
        test_err++;
        DEBUG_DISPLAY("ERR[%u]: Per-pin status bit not set after negedge (rdata=0x%08X)\n", cur_idx, rdata);
    }

    // Group interrupt status should have the one-hot bit set
    unsigned int grp = reg_read(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if ((grp & last_wr_val) == 0u) {
        test_err++;
        DEBUG_DISPLAY("ERR[%u]: Group INTR_STS1 missing expected bit. grp=0x%08X, exp_onehot=0x%08X\n", cur_idx, grp, last_wr_val);
    }

    // Acknowledge per-pin: write bits 20 and 16 (W1C/ACK per block definition)
    reg_write(pin_addr, (1u << 20) | (1u << 16));
    brief_wait();

    // Clear raw group status (W1C) for this bit
    reg_write(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, last_wr_val);
    brief_wait();

    // Group status should now be 0
    grp = reg_read(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (grp != 0x0u) {
        test_err++;
        DEBUG_DISPLAY("ERR[%u]: Group INTR_STS1 not cleared to 0. grp=0x%08X\n", cur_idx, grp);
    }

    // Clear system-level raw status (broad clear if mask unknown)
    reg_write(MIZAR_LSS_SYSREG_RAW_STCR1, 0xFFFFFFFFu);

    // Clear GIC IRQ for GPIO0 (IRQ 87) if BSP provides API
    if (&GIC_ClearPendingIRQ) {
        GIC_ClearPendingIRQ(87u);
    }
}

void test_case(void)
{
    DEBUG_DISPLAY("Start: test_gpio_negedge_intr_en\n");

    // Environment and system enables (GPIO0 path)
    // Try to enable IRQ 87 if API is present; otherwise continue.
    if (&GIC_EnableIRQ) {
        GIC_EnableIRQ(87u);
    }

    // Enable GPIO system interrupt: set all ones to ensure the instance is enabled
    unsigned int sys_en = reg_read(MIZAR_LSS_SYSREG_INTR_EN1);
    reg_write(MIZAR_LSS_SYSREG_INTR_EN1, sys_en | 0xFFFFFFFFu);

    // Per-pin configuration for indices 0..31 (GPIO[8..39])
    for (cur_idx = 0; cur_idx < 32; ++cur_idx) {
        unsigned long int pin_addr = addr_array[cur_idx];
        unsigned int val = reg_read(pin_addr);
        val |= (1u << 20) | (1u << 18) | (1u << 16); // set ack/clear + mode bits per spec
        reg_write(pin_addr, val);
        brief_wait();
    }

    // Per-pin stimulus
    for (unsigned int i = 0; i < 32; ++i) {
        cur_idx = i;
        last_wr_val = (1u << i);

        // Clear pending raw for this bit (W1C)
        reg_write(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, last_wr_val);
        brief_wait();

        // Enable the corresponding group interrupt bit (set bit i)
        unsigned int gen = reg_read(MIZAR_GPIO_GP0_INTR1_INTR_EN1);
        gen |= last_wr_val;
        reg_write(MIZAR_GPIO_GP0_INTR1_INTR_EN1, gen);
        brief_wait();

        // Arm SW pending flag
        int_pend = 1u;

        // Drive pad control at 0xA0243FFC high, then create high->low on selected bit
        reg_write(0xA0243FFCu, 0xFFFFFFFFu);
        brief_wait();
        reg_write(0xA0243FFCu, ~last_wr_val);

        // Wait for ISR to service (int_pend cleared in ISR); timeout 5000 iterations
        unsigned int to = 5000u;
        while (to-- && int_pend) {
            brief_wait();
        }
        if (int_pend) {
            test_err++;
            DEBUG_DISPLAY("TIMEOUT: No ISR observed for i=%u (onehot=0x%08X)\n", i, last_wr_val);
            // Attempt service inline to progress and avoid cascading timeouts
            Default_IRQHandler();
        }

        // Prep for next iteration: drive back to all ones
        reg_write(0xA0243FFCu, 0xFFFFFFFFu);
        brief_wait();
    }

    DEBUG_DISPLAY("Finish: test_gpio_negedge_intr_en, test_err=%d\n", test_err);
    finish(test_err ? 1 : 0);
}
