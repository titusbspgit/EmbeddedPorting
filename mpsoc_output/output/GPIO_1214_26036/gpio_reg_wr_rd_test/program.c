#include "test_define.c"

/*
 * gpio_reg_wr_rd_test
 * High-level: Verify default mask-read values and masked write/read behavior over addr_array[]
 * - Default check: for each i, if skip_rst_array[i]!=1 and read_mask_array[i]!=0, then
 *   read data_rd=(read_reg(addr[i]) & 0xFFFFFFFE), compare with default_value_array[i]
 * - Write/read: iterate patterns; if skip_array[i]!=1 and write_mask_array[i]!=0, write masked value;
 *   then if read_mask_array[i]!=0, read data_rd=(read_reg(addr[i]) & read_mask_array[i])
 *   exp_val = (data_wr & read_mask_array[i] & write_mask_array[i]) | ((~write_mask_array[i]) & read_mask_array[i] & default_value_array[i])
 *   compare data_rd vs exp_val
 */

static int def_fail_cnt = 0;
static int wr_fail_cnt = 0;

static const unsigned int patterns[] = {
    0xFFFFFFFFu, 0xAAAAAAAAu, 0x55555555u, 0xF5F5F5F5u, 0xA5A5A5A5u, 0xFFFF0000u
};

void test_case(void)
{
    unsigned int i, p;

#ifdef DEBUG_DISPLAY
    printf("[GPIO] Starting gpio_reg_wr_rd_test on %u registers\n", (unsigned)CNT);
#endif

    // Default value masked-read check
    for (i = 0; i < CNT; ++i) {
        unsigned int rm = read_mask_array[i];
        if (rm == 0u) continue; // nothing to check
        // skip reset-specific validation if provided via skip_rst_array; if absent, proceed
#ifdef HAVE_SKIP_RST_ARRAY
        if (skip_rst_array[i] == 1u) continue;
#endif
        unsigned long addr = addr_array[i];
        unsigned int rd = read_reg(addr) & 0xFFFFFFFEu;
        unsigned int exp = default_value_array[i];
        if (rd != exp) {
#ifdef DEBUG_DISPLAY
            printf("[GPIO][DEF] i=%u addr=0x%08lX rd=0x%08X exp=0x%08X\n", i, addr, rd, exp);
#endif
            def_fail_cnt++;
        }
    }

    // Masked write/read verification over patterns
    for (p = 0; p < (sizeof(patterns)/sizeof(patterns[0])); ++p) {
        unsigned int data_wr = patterns[p];
        for (i = 0; i < CNT; ++i) {
            if (skip_array[i] == 1u) continue; // honor skip list
            unsigned int wm = write_mask_array[i];
            if (wm == 0u) continue; // not writable
            unsigned int rm = read_mask_array[i];
            unsigned long addr = addr_array[i];
            // perform masked write
            write_reg(addr, (data_wr & wm));
            if (rm == 0u) continue; // nothing to validate on read
            // read-back masked value
            unsigned int rd = (read_reg(addr) & rm);
            unsigned int wr_n = (~wm);
            unsigned int exp = ((data_wr & rm & wm) | (wr_n & rm & default_value_array[i]));
            if (rd != exp) {
#ifdef DEBUG_DISPLAY
                printf("[GPIO][WR] pat=%u i=%u addr=0x%08lX rd=0x%08X exp=0x%08X wm=0x%08X rm=0x%08X\n", p, i, addr, rd, exp, wm, rm);
#endif
                wr_fail_cnt++;
            }
        }
    }

    if ((def_fail_cnt > 0) || (wr_fail_cnt > 0)) {
#ifdef DEBUG_DISPLAY
        printf("[GPIO] FAIL def=%d wr=%d\n", def_fail_cnt, wr_fail_cnt);
#endif
        finish(1);
    } else {
#ifdef DEBUG_DISPLAY
        printf("[GPIO] PASS\n");
#endif
        finish(0);
    }
}
