#include <lss_sysreg.h>
#include <stdio.h>
#include "test_define.c"
#include <test_common.h>

int DEBUG_DISPLAY(const char *fmt, ...);
#ifndef TRACE_LOG
#define TRACE_LOG(...) DEBUG_DISPLAY(__VA_ARGS__)
#endif

/*
 * Test: test_gpio_negedge_intr_en
 * Description: Enable group interrupt output, configure per-pin negedge control at
 * MIZAR_GPIO_GP0_GPIO_8+(i*4) with (1u<<20)|(1u<<18)|(1u<<16), clear RAW and enable
 * per-bit group enable, generate falling edge using 0xA0243FFC toggling, and validate ISR
 * clears pending and group status as per acceptance criteria.
 */

static inline void drive_pattern(unsigned int val) { write_reg(0xA0243FFC, val); }

void Default_IRQHandler(void) {
    unsigned int local_wr = (1u << i);
    unsigned long raddr = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4));
    unsigned int rdata;
    unsigned int rdata_grp;

    int_pend = 0; /* Signal main loop */

    /* Read per-pin control/status */
    rdata = read_reg(raddr);
    /* If bit[0] (hypothetical) set -> error per CSV */
    if ((rdata & 0x1u) != 0u) {
        DEBUG_DISPLAY("[IRQ] ERR: Per-pin bit0 set i=%d rdata=0x%08X\n", i, rdata);
        test_err++;
    }
    /* If bit[1] non-zero, then group status must reflect local_wr */
    if ((rdata & 0x2u) != 0u) {
        rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
        if ((rdata_grp & local_wr) == 0u) {
            DEBUG_DISPLAY("[IRQ] ERR: Group STS1 missing bit for i=%d sts=0x%08X\n", i, rdata_grp);
            test_err++;
        }
    }

    /* Re-program per-pin to clear raw via control write per CSV */
    write_reg(raddr, (1u<<20) | (1u<<16));
    /* Clear RAW per-pin pending */
    write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr);
    /* Verify group status cleared */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0u) {
        DEBUG_DISPLAY("[IRQ] ERR: Group STS1 not cleared sts=0x%08X\n", rdata_grp);
        test_err++;
    }

    /* Clear SYSREG RAW and GIC IRQ */
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    GIC_ClearIRQ(88);
#endif
}

void test_case(void) {
    int t;
    unsigned int wr_val;
    unsigned long addr1;

#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

    /* Enable system/group interrupt output */
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* Initialize external drive line */
    drive_pattern(0xFFFFFFFFu);

    /* Configure per-pin negedge detection */
    for (i = 0; i < 32; i++) {
        addr1 = (MIZAR_GPIO_GP0_GPIO_8 + (i * 4));
        write_reg(addr1, (1u<<20) | (1u<<18) | (1u<<16));
        wait_on(10);
    }

    for (i = 0; i < 32; i++) {
        wr_val = (1u << i);
        /* Clear any existing raw and enable this bit in group enable */
        write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val);
        wait_on(10);

        /* Generate falling edge for bit i: 0xFFFFFFFF -> ~wr_val */
        int_pend = 1;
        drive_pattern(0xFFFFFFFFu);
        drive_pattern(~wr_val);

        /* Poll for ISR to clear int_pend */
        t = 5000;
        while ((int_pend != 0) && (t-- > 0)) {
            wait_on(10);
        }
        if (int_pend != 0) {
            DEBUG_DISPLAY("[MAIN] ERR: Timeout waiting for IRQ clear i=%d\n", i);
            test_err++;
            /* attempt cleanup */
            write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val);
        }
    }

    finish(test_err);
}
