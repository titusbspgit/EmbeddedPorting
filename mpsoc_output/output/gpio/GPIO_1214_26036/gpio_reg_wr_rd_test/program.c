#include <lss_sysreg.h>
#include <stdio.h>
#include "test_define.c"
#include <test_common.h>

int DEBUG_DISPLAY(const char *fmt, ...);
#ifndef TRACE_LOG
#define TRACE_LOG(...) DEBUG_DISPLAY(__VA_ARGS__)
#endif

/*
 * Test: gpio_reg_wr_rd_test
 * Description: Check default reset values and masked write/read semantics using
 * addr_array/default_value_array/read_mask_array/write_mask_array. Skip entries
 * per skip_array and skip_rst_array. Failures accumulate in def_fail_cnt and
 * wr_fail_cnt and determine finish(1) vs finish(0).
 */
void test_case(void) {
    int def_fail_cnt = 0;
    int wr_fail_cnt = 0;
    unsigned int data_rd, data_masked;
    unsigned int data_wr;
    unsigned int wr_n, exp_val;
    unsigned long addr;
    int i, p;

    TRACE_LOG("[GPIO] gpio_reg_wr_rd_test: start\n");

    /* Default value checks */
    for (i = 0; i < CNT; i++) {
        if (skip_rst_array[i] == 1) {
            TRACE_LOG("[RSTCHK] skip_rst idx=%d\n", i);
            continue;
        }
        if (read_mask_array[i] == 0x00000000u) {
            TRACE_LOG("[RSTCHK] skip idx=%d due to read_mask=0\n", i);
            continue;
        }
        addr = addr_array[i];
        data_rd = read_reg(addr);
        data_masked = (data_rd & 0xFFFFFFFEu);
        if (data_masked != (unsigned int)default_value_array[i]) {
            DEBUG_DISPLAY("DEF_MISMATCH: idx=%d addr=0x%08lX rd_masked=0x%08X exp=0x%08X\n",
                          i, addr, data_masked, (unsigned int)default_value_array[i]);
            def_fail_cnt++;
        }
    }

    /* Masked write/read checks */
    for (p = 0; p < 6; p++) {
        data_wr = data_patterns[p];
        TRACE_LOG("[WRRD] pattern[%d]=0x%08X\n", p, data_wr);
        for (i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) continue;
            if (write_mask_array[i] == 0x00000000u) continue;
            addr = addr_array[i];
            write_reg(addr, (data_wr & (unsigned int)write_mask_array[i]));
            if (read_mask_array[i] == 0x00000000u) continue;
            data_rd = (read_reg(addr) & (unsigned int)read_mask_array[i]);
            wr_n = ((unsigned int)write_mask_array[i] ^ 0xFFFFFFFFu);
            exp_val = ((data_wr & (unsigned int)read_mask_array[i] & (unsigned int)write_mask_array[i]) |
                       (wr_n & (unsigned int)read_mask_array[i] & (unsigned int)default_value_array[i]));
            if (data_rd != exp_val) {
                DEBUG_DISPLAY("WRRD_MISMATCH: idx=%d addr=0x%08lX wr=0x%08X rd=0x%08X exp=0x%08X rm=0x%08X wm=0x%08X\n",
                              i, addr, data_wr, data_rd, exp_val,
                              (unsigned int)read_mask_array[i], (unsigned int)write_mask_array[i]);
                wr_fail_cnt++;
            }
        }
    }

    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0)) {
        DEBUG_DISPLAY("gpio_reg_wr_rd_test: FAIL def=%d wr=%d\n", def_fail_cnt, wr_fail_cnt);
        finish(1);
    } else {
        DEBUG_DISPLAY("gpio_reg_wr_rd_test: PASS\n");
        finish(0);
    }
}
