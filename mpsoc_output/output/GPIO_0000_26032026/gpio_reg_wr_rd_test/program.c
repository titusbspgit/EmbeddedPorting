#define DEBUG_DISPLAY 1
#include "test_define.c"

/*
 * Test: gpio_reg_wr_rd_test
 * - Verify default reset values (masked) for all impacted registers.
 * - Perform masked write/read tests with multiple patterns and compare against expected value
 *   using read_mask_array and write_mask_array semantics.
 * Acceptance:
 * - No mismatches in default value checks and write/read checks -> finish(0)
 * - Any mismatch increments counters and leads to finish(1)
 */

static const unsigned int patterns[] = {
    0xFFFFFFFFu, 0xAAAAAAAAu, 0x55555555u, 0xF5F5F5F5u, 0xA5A5A5A5u, 0xFFFF0000u
};

void test_case(void)
{
    unsigned int i, p;
    unsigned int data_rd, data_wr, data_m, wr_n, exp_val;
    int def_fail_cnt = 0;
    int wr_fail_cnt = 0;

    /* Default reset value check with read masking (and additional mask 0xFFFFFFFE) */
    for (i = 0; i < CNT; ++i) {
        if (skip_rst_array[i] == 1)
            continue;
        if (read_mask_array[i] == 0x00000000)
            continue;

        data_rd = read_reg(addr_array[i]);
        data_m  = (data_rd & 0xFFFFFFFEu);
        if (data_m != (unsigned int)default_value_array[i]) {
#ifdef DEBUG_DISPLAY
            printf("DEFVAL MISMATCH: idx=%u addr=0x%08lx rd=0x%08X exp=0x%08X\n",
                   i, addr_array[i], data_m, (unsigned)default_value_array[i]);
#endif
            def_fail_cnt++;
        }
    }

    /* Write/read check with mask semantics */
    for (p = 0; p < (sizeof(patterns)/sizeof(patterns[0])); ++p) {
        data_wr = patterns[p];
        for (i = 0; i < CNT; ++i) {
            if (skip_array[i] == 1)
                continue;
            if (write_mask_array[i] == 0x00000000)
                continue;

            /* Perform masked write */
            write_reg(addr_array[i], (data_wr & (unsigned int)write_mask_array[i]));

            if ((write_mask_array[i] == 0x00000000) || (read_mask_array[i] == 0x00000000))
                continue;

            data_rd = read_reg(addr_array[i]);
            data_rd &= (unsigned int)read_mask_array[i];
            wr_n = ((unsigned int)write_mask_array[i] ^ 0xFFFFFFFFu);
            exp_val = ((data_wr & (unsigned int)read_mask_array[i] & (unsigned int)write_mask_array[i]) |
                       (wr_n    & (unsigned int)read_mask_array[i] & (unsigned int)default_value_array[i]));

            if (data_rd != exp_val) {
#ifdef DEBUG_DISPLAY
                printf("WRCHK MISMATCH: idx=%u addr=0x%08lx pat=0x%08X rd=0x%08X exp=0x%08X rm=0x%08X wm=0x%08X def=0x%08X\n",
                       i, addr_array[i], data_wr, data_rd, exp_val,
                       (unsigned)read_mask_array[i], (unsigned)write_mask_array[i], (unsigned)default_value_array[i]);
#endif
                wr_fail_cnt++;
            }
        }
    }

#ifdef DEBUG_DISPLAY
    printf("Summary: def_fail_cnt=%d, wr_fail_cnt=%d\n", def_fail_cnt, wr_fail_cnt);
#endif

    if (def_fail_cnt > 0 || wr_fail_cnt > 0)
        finish(1);
    else
        finish(0);
}
