#include "test_define.c"

// GPIO register reset and masked R/W verification
void test_case(void)
{
    int i, p;
    unsigned long addr;
    unsigned int data_rd, exp_val, wr_n;
    int def_fail_cnt = 0;
    int wr_fail_cnt = 0;

    DEBUG_DISPLAY("[GPIO] gpio_reg_wr_rd_test: start\n");

    // Default value checks
    for (i = 0; i < CNT; i++) {
        if (skip_rst_array[i]) continue;                     // CSV: conditional reset-skip
        if ((unsigned int)read_mask_array[i] == 0u) continue; // CSV: skip if read mask is zero
        addr = addr_array[i];
        data_rd = read_reg(addr) & 0xFFFFFFFEu;              // CSV: mask read value with 0xFFFFFFFE
        exp_val = ((unsigned int)default_value_array[i]) & ((unsigned int)read_mask_array[i]);
        if (data_rd != exp_val) {
            DEBUG_DISPLAY("[GPIO][RST-MISMATCH] idx=%d addr=0x%08lX rd=0x%08X exp=0x%08X\n", i, addr, data_rd, exp_val);
            def_fail_cnt++;
        } else {
            TRACE_LOG("[GPIO][RST-OK] idx=%d addr=0x%08lX val=0x%08X\n", i, addr, data_rd);
        }
    }

    // Write/Read checks across patterns
    for (p = 0; p < 6; p++) {
        unsigned int data_wr = data_patterns[p];
        TRACE_LOG("[GPIO][PATTERN] 0x%08X\n", data_wr);
        for (i = 0; i < CNT; i++) {
            if (skip_array[i]) continue;                      // CSV: conditional skip
            if ((unsigned int)write_mask_array[i] == 0u) continue; // CSV: skip if write mask is zero
            addr = addr_array[i];
            write_reg(addr, data_wr & (unsigned int)write_mask_array[i]);
            if ((unsigned int)read_mask_array[i] == 0u) continue; // CSV: also skip read/compare if read mask is zero
            data_rd = read_reg(addr) & (unsigned int)read_mask_array[i];
            wr_n = ((unsigned int)write_mask_array[i]) ^ 0xFFFFFFFFu;
            exp_val = ((data_wr & (unsigned int)read_mask_array[i] & (unsigned int)write_mask_array[i]) |
                       (wr_n    & (unsigned int)read_mask_array[i] & (unsigned int)default_value_array[i]));
            if (data_rd != exp_val) {
                DEBUG_DISPLAY("[GPIO][WR-CHK-FAIL] idx=%d addr=0x%08lX rd=0x%08X exp=0x%08X\n", i, addr, data_rd, exp_val);
                wr_fail_cnt++;
            } else {
                TRACE_LOG("[GPIO][WR-CHK-OK] idx=%d addr=0x%08lX val=0x%08X\n", i, addr, data_rd);
            }
        }
    }

    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        DEBUG_DISPLAY("[GPIO] gpio_reg_wr_rd_test: FAIL def=%d wr=%d\n", def_fail_cnt, wr_fail_cnt);
        finish(1);
    } else {
        DEBUG_DISPLAY("[GPIO] gpio_reg_wr_rd_test: PASS\n");
        finish(0);
    }
}
