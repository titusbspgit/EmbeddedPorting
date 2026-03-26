#include <lss_sysreg.h>
#include <stdio.h>
#include "test_define.c"
#include <test_common.h>

int DEBUG_DISPLAY(const char *fmt, ...);
#ifndef TRACE_LOG
#define TRACE_LOG(...) DEBUG_DISPLAY(__VA_ARGS__)
#endif

/*
 * Test: test_gpio_pedge_all_pads_en
 * Description: Enable group interrupt, program posedge per-pin, configure IO_CTRL groups,
 * enable group mask, generate rising edges with 0xA0243FFC, and validate ISR clears per-pin
 * raw and group status per CSV acceptance criteria.
 */

static inline void drive_pattern(unsigned int val) { write_reg(0xA0243FFC, val); }

void Default_IRQHandler(void) {
    unsigned int wr_val = (1u << i);
    unsigned int rdata_grp;
    int j;

    int_pend = 0; /* Signal main loop */

    /* Read group status and require non-zero */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if ((rdata_grp & 0xFFFFFFFFu) == 0u) {
        DEBUG_DISPLAY("[IRQ] ERR: Group STS1 is zero on IRQ i=%d\n", i);
        test_err++;
    }

    /* Mask group to quiesce while clearing RAW */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000u);

    /* Clear per-pin raw using control write 0x00010000 across all pins */
    for (j = 0; j < 32; j++) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (j * 4), 0x00010000u);
    }
    wait_on(2);

    /* Verify group status cleared */
    rdata_grp = read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1);
    if (rdata_grp != 0u) {
        DEBUG_DISPLAY("[IRQ] ERR: Group STS1 not cleared sts=0x%08X\n", rdata_grp);
        test_err++;
    }

    /* Clear SYSREG RAW and re-enable group */
#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR);
    GIC_ClearIRQ(87);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR);
    GIC_ClearIRQ(88);
#endif

    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);
}

void test_case(void) {
    int t;
    int pad;

#ifdef GPIO0
    GIC_EnableIRQ(87);
#endif
#ifdef GPIO1
    GIC_EnableIRQ(88);
#endif

#ifdef GPIO0
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR);
#endif
#ifdef GPIO1
    write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);
#endif

    /* Program posedge per pin */
    for (i = 0; i < 32; i++) {
        write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i * 4), 0x00020000u);
    }
    wait_on(10);

    /* Configure IO control groups */
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FFu);
    write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FFu);
    wait_on(10);

    /* Enable group mask */
    write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFFu);

    /* Generate rising edges across pads */
    for (pad = 0; pad < 32; pad++) {
        i = pad; /* current pad index used in ISR */
        drive_pattern(0x00000000u);
        wait_on(10);
        int_pend = 1;
        drive_pattern(0xFFFFFFFFu);

        /* Poll with timeout for ISR to clear */
        t = 2000;
        while ((int_pend != 0) && (t-- > 0)) {
            wait_on(10);
        }
        if (int_pend != 0) {
            DEBUG_DISPLAY("[MAIN] ERR: Timeout waiting for IRQ clear pad=%d\n", pad);
            test_err++;
        }

        /* Drive low again before next pad */
        drive_pattern(0x00000000u);
        wait_on(10);
    }

    finish(test_err);
}
