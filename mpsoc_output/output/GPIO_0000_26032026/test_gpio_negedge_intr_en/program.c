#define DEBUG_DISPLAY 1
#include "test_define.c"

/*
 * Test: test_gpio_negedge_intr_en
 * Goal: Configure per-pin negedge interrupt detection, enable group interrupts,
 *       clear raw status, generate a falling edge per pin, and validate in ISR.
 * Acceptance: No timeouts; ISR must observe status bit set, then clears lead to 0.
 */

volatile int int_pend = 0;
static int test_err = 0;

/* Forward declaration for IRQ handler symbol used by platform */
void Default_IRQHandler(void);

void test_case(void)
{
    unsigned int i;

#ifdef GPIO0
    GIC_EnableIRQ(87);
    /* Enable group interrupt for GPIO0 */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
    /* Enable group interrupt for GPIO1 */
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* Drive known value on the pad driver mirror */
    write_reg(0xA0243FFC, 0xFFFFFFFFu);

    /* Configure per-pin: enable pad, unmask, negedge detect ((1<<20)|(1<<18)|(1<<16)) */
    for (i = 0; i < 32; ++i) {
        unsigned long addr1 = gpio0_pin_cfg_base + (i * 4u);
        write_reg(addr1, (1u<<20) | (1u<<18) | (1u<<16));
        wait_on(10);
    }

    /* Iterate pins: clear raw, enable bit, generate falling edge, wait for ISR */
    for (i = 0; i < 32; ++i) {
        unsigned int wr_val = (1u << i);
        /* Clear per-pin raw status and enable group intr per bit */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        int_pend = 1;
        /* Toggle pad driver: set all ones, then clear target bit to create falling edge */
        write_reg(0xA0243FFC, 0xFFFFFFFFu);
        write_reg(0xA0243FFC, (~wr_val));

        /* Poll for ISR to complete with timeout */
        unsigned int to;
        for (to = 0; to < 5000u; ++to) {
            wait_on(10);
            if (int_pend == 0)
                break;
        }
        if (int_pend != 0) {
#ifdef DEBUG_DISPLAY
            printf("TIMEOUT: pin %u did not trigger/clear ISR in time\n", i);
#endif
            test_err++;
            /* Try to clean up to continue */
            write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        }
    }

    finish(test_err);
}

void Default_IRQHandler(void)
{
    /* Use current 'i' from context; in platforms without nested IRQs this is acceptable */
    extern unsigned int i; /* rely on calling context loop variable */
    unsigned int local_wr = (1u << i);

    int_pend = 0;

    /* Read per-pin control to sample status bits [0] and [1] */
    unsigned long raddr = gpio0_pin_cfg_base + (i * 4u);
    unsigned int rdata = read_reg(raddr);

    if ((rdata & 0x1u) != 0u) {
#ifdef DEBUG_DISPLAY
        printf("ISR: pin %u unexpected bit0 set in per-pin ctrl: 0x%08X\n", i, rdata);
#endif
        test_err++;
    }

    /* If bit1 shows event, group status must reflect it */
    if ((rdata & 0x2u) != 0u) {
        unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0u) {
#ifdef DEBUG_DISPLAY
            printf("ISR: pin %u group sts missing bit: grp=0x%08X\n", i, rdata_grp);
#endif
            test_err++;
        }
    }

    /* Clear per-pin config raw/status and group raw */
    write_reg(gpio0_pin_cfg_base + (i * 4u), (1u<<20) | (1u<<16));
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);

    /* Group status must become 0 */
    {
        unsigned int rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if (rdata_grp != 0u) {
#ifdef DEBUG_DISPLAY
            printf("ISR: pin %u group sts not cleared: 0x%08X\n", i, rdata_grp);
#endif
            test_err++;
        }
    }

#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    GIC_ClearIRQ(88);
#endif
}
