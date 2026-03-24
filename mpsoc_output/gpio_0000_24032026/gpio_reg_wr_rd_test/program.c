#include "test_define.c"

/* Auto-generated program.c for gpio_reg_wr_rd_test
   Implements:
   - Default/reset value check (ignoring bit 0 on read)
   - Masked write/readback check across defined patterns
   Uses only macros and arrays provided in test_define.c and referenced headers.
*/

volatile int def_fail_cnt = 0;
volatile int wr_fail_cnt  = 0;
volatile int test_err     = 0;

static inline int get_skip_rst(int idx) {
#ifdef HAVE_SKIP_RST_ARRAY
    return skip_rst_array[idx];
#else
    return skip_array[idx];
#endif
}

static void chk_rst_val(void) {
    for (int i = 0; i < CNT; i++) {
        if (get_skip_rst(i)) {
#ifdef DEBUG_DISPLAY
            printf("SKIP_RST: index=%d\n", i);
#endif
            continue;
        }

        /* If read mask is zero, nothing meaningful to compare */
        if (read_mask_array[i] == 0) {
#ifdef DEBUG_DISPLAY
            printf("SKIP_RST_READMASK0: index=%d\n", i);
#endif
            continue;
        }

        unsigned int addr    = (unsigned int)addr_array[i];
        unsigned int data_rd = (unsigned int)read_reg(addr);

        /* Ignore bit0 on read per test description */
        unsigned int data_m  = (data_rd & 0xFFFFFFFEu);
        unsigned int exp_val = (unsigned int)default_value_array[i];

        if (data_m != exp_val) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("DEF_MISMATCH: i=%d addr=0x%08X read_m=0x%08X exp=0x%08X\n",
                   i, addr, data_m, exp_val);
#endif
        } else {
#ifdef DEBUG_DISPLAY
            printf("DEF_OK: i=%d addr=0x%08X read_m=0x%08X exp=0x%08X\n",
                   i, addr, data_m, exp_val);
#endif
        }
    }
}

static void chk_rd_wr(void) {
    static const unsigned int patterns[] = {
        0xFFFFFFFFu, 0xAAAAAAAAu, 0x55555555u, 0xF5F5F5F5u, 0xA5A5A5A5u, 0xFFFF0000u
    };

    /* Write phase for each pattern across all addresses */
    for (unsigned int p = 0; p < (sizeof(patterns)/sizeof(patterns[0])); p++) {
        unsigned int data_wr = patterns[p];

        for (int i = 0; i < CNT; i++) {
            if (skip_array[i]) {
#ifdef DEBUG_DISPLAY
                printf("SKIP_WR: index=%d\n", i);
#endif
                continue;
            }

            unsigned int wmask = (unsigned int)write_mask_array[i];
            if (wmask == 0u) {
#ifdef DEBUG_DISPLAY
                printf("SKIP_WR_WMASK0: index=%d\n", i);
#endif
                continue;
            }

            unsigned int addr = (unsigned int)addr_array[i];
            unsigned int wval = (data_wr & wmask);

            write_reg(addr, wval);

#ifdef DEBUG_DISPLAY
            printf("WRITE: i=%d addr=0x%08X wval=0x%08X wmask=0x%08X pat=0x%08X\n",
                   i, addr, wval, wmask, data_wr);
#endif
        }

        /* Read/verify phase for each index */
        for (int i = 0; i < CNT; i++) {
            if (skip_array[i]) {
#ifdef DEBUG_DISPLAY
                printf("SKIP_RD: index=%d\n", i);
#endif
                continue;
            }

            unsigned int rmask = (unsigned int)read_mask_array[i];
            unsigned int wmask = (unsigned int)write_mask_array[i];

            /* If either mask is zero, no valid comparison to make */
            if (rmask == 0u || wmask == 0u) {
#ifdef DEBUG_DISPLAY
                printf("SKIP_CMP_MASK0: index=%d rmask=0x%08X wmask=0x%08X\n", i, rmask, wmask);
#endif
                continue;
            }

            unsigned int addr    = (unsigned int)addr_array[i];
            unsigned int rd_m    = ((unsigned int)read_reg(addr) & rmask);
            unsigned int wr_n    = ~wmask;
            unsigned int def_v   = (unsigned int)default_value_array[i];

            unsigned int exp_val = ((data_wr & rmask & wmask) |
                                    (wr_n   & rmask & def_v));

            if (rd_m != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("WR_MISMATCH: i=%d addr=0x%08X rd_m=0x%08X exp=0x%08X "
                       "rmask=0x%08X wmask=0x%08X def=0x%08X pat=0x%08X\n",
                       i, addr, rd_m, exp_val, rmask, wmask, def_v, data_wr);
#endif
            } else {
#ifdef DEBUG_DISPLAY
                printf("WR_OK: i=%d addr=0x%08X rd_m=0x%08X exp=0x%08X\n",
                       i, addr, rd_m, exp_val);
#endif
            }
        }
    }
}

/* Entry point */
void test_case(void) {
    def_fail_cnt = 0;
    wr_fail_cnt  = 0;
    test_err     = 0;

    chk_rst_val();
    chk_rd_wr();

    if (def_fail_cnt > 0 || wr_fail_cnt > 0) {
        finish(1);
    } else {
        finish(0);
    }
}

void Default_IRQHandler(void) {
#ifdef DEBUG_DISPLAY
    printf("ERROR: Default IRQ handler invoked unexpectedly.\n");
#endif
    test_err++;
}
