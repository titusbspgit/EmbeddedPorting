#include "test_define.c"

// Test: gpio_reg_wr_rd_test
// Description (from CSV): Check default reset values and masked write/read semantics for registers addressed via addr_array,
// comparing against default_value_array with read_mask_array, and performing masked writes using write_mask_array; entries
// are conditionally skipped per skip_array and skip_rst_array; failures accumulate in def_fail_cnt and wr_fail_cnt and drive finish().

void test_case(void)
{
    unsigned int def_fail_cnt = 0;
    unsigned int wr_fail_cnt = 0;

    // STEP: Default value check
    for (unsigned int i = 0; i < CNT; i++) {
        if (skip_rst_array[i] == 1) { continue; }
        if (read_mask_array[i] == 0x00000000) { continue; }
        unsigned long addr = addr_array[i];
        unsigned int rd = read_reg(addr);
        unsigned int data = (rd & 0xFFFFFFFEu); // per CSV, mask with 0xFFFFFFFE
        unsigned int exp = default_value_array[i];
        if (data != exp) {
#ifdef DEBUG_DISPLAY
            printf("[DEF_MISMATCH] idx=%u addr=0x%08lX rd=0x%08X data=0x%08X exp=0x%08X\n", i, addr, rd, data, exp);
#endif
            def_fail_cnt++;
        } else {
#ifdef DEBUG_DISPLAY
            printf("[DEF_OK] idx=%u addr=0x%08lX rd=0x%08X masked=0x%08X\n", i, addr, rd, data);
#endif
        }
    }

    // STEP: Masked write/read checks across patterns
    const unsigned int patterns[] = { 0xFFFFFFFFu, 0xAAAAAAAAu, 0x55555555u, 0xF5F5F5F5u, 0xA5A5A5A5u, 0xFFFF0000u };
    const unsigned int NPAT = sizeof(patterns)/sizeof(patterns[0]);

    for (unsigned int p = 0; p < NPAT; p++) {
        unsigned int data_wr = patterns[p];
        for (unsigned int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1) { continue; }
            if (write_mask_array[i] == 0x00000000) { continue; }

            unsigned long addr = addr_array[i];
            unsigned int wmask = write_mask_array[i];
            unsigned int rmask = read_mask_array[i];

            // Write masked value
            unsigned int wval = (data_wr & wmask);
            write_reg(addr, wval);
#ifdef DEBUG_DISPLAY
            printf("[WR] ptn=0x%08X idx=%u addr=0x%08lX wmask=0x%08X wval=0x%08X\n", data_wr, i, addr, wmask, wval);
#endif

            // If no readable bits or no writable bits, skip read/compare
            if ((wmask == 0x00000000) || (rmask == 0x00000000)) { continue; }

            // Read back and mask
            unsigned int rd = read_reg(addr);
            unsigned int rd_m = (rd & rmask);

            // Expected value per CSV formula
            unsigned int wr_n = (wmask ^ 0xFFFFFFFFu);
            unsigned int exp_val = ((data_wr & rmask & wmask) | (wr_n & rmask & default_value_array[i]));

            if (rd_m != exp_val) {
#ifdef DEBUG_DISPLAY
                printf("[WR_RD_MISMATCH] ptn=0x%08X idx=%u addr=0x%08lX rd=0x%08X rd_m=0x%08X exp=0x%08X rmask=0x%08X wmask=0x%08X\n",
                       data_wr, i, addr, rd, rd_m, exp_val, rmask, wmask);
#endif
                wr_fail_cnt++;
            } else {
#ifdef DEBUG_DISPLAY
                printf("[WR_RD_OK] ptn=0x%08X idx=%u addr=0x%08lX rd_m=0x%08X exp=0x%08X\n", data_wr, i, addr, rd_m, exp_val);
#endif
            }
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[SUMMARY] def_fail_cnt=%u wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
#endif

    if ((def_fail_cnt > 0u) || (wr_fail_cnt > 0u)) {
        finish(1);
    } else {
        finish(0);
    }
}

// Optional default handler if required by platform; does not alter result in this test
void Default_IRQHandler(void)
{
#ifdef DEBUG_DISPLAY
    printf("[INFO] Default_IRQHandler invoked (no-op for gpio_reg_wr_rd_test)\n");
#endif
}
