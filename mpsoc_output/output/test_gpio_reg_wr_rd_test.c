#include <stdio.h>
#include <test_common.h>
#include <lss_sysreg.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

// extern void finish(int);
// extern void wait_on(unsigned int);
// extern unsigned int read_reg(unsigned long addr);
// extern void write_reg(unsigned long addr, unsigned int val);
// extern void GIC_EnableIRQ(int id);
// extern void GIC_ClearIRQ(int id);

/*
 * Testcase: gpio_reg_wr_rd_test
 * Description:
 *   Check default reset values and masked write/read semantics for registers
 *   addressed via addr_array, comparing against default_value_array with
 *   read_mask_array, and performing masked writes using write_mask_array;
 *   entries are conditionally skipped per skip_array and skip_rst_array;
 *   failures accumulate in def_fail_cnt and wr_fail_cnt and drive finish().
 *
 * Procedure (from CSV):
 *   - Default value check:
 *       for i in [0..CNT-1]:
 *         if skip_rst_array[i]==1 -> continue
 *         if read_mask_array[i]==0x00000000 -> continue
 *         data = (read_reg(addr_array[i]) & 0xFFFFFFFE)
 *         compare data == default_value_array[i]
 *         on mismatch -> def_fail_cnt++
 *   - Write/read check:
 *       patterns = {0xFFFFFFFF,0xAAAAAAAA,0x55555555,0xF5F5F5F5,0xA5A5A5A5,0xFFFF0000}
 *       for each pattern and each i:
 *         if skip_array[i]==1 -> continue
 *         if write_mask_array[i]==0x00000000 -> continue
 *         write (pattern & write_mask_array[i]) to addr
 *         if write_mask_array[i]==0 or read_mask_array[i]==0 -> continue
 *         data_rd = (read_reg(addr) & read_mask_array[i])
 *         wr_n = (write_mask_array[i] ^ 0xFFFFFFFF)
 *         exp = ((pattern & read_mask & write_mask) | (wr_n & read_mask & default))
 *         if data_rd != exp -> wr_fail_cnt++
 *   - At end: if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0)
 */

#define CNT 49

static const unsigned long addr_array[CNT] = {
    MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11,
    MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15,
    MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19,
    MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23,
    MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27,
    MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31,
    MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35,
    MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39,
    MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,
    MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1,
    MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,
    MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2,
    MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4,
    MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2,
    MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4
};

static const unsigned int default_value_array[CNT] = {
    GPIO_GP0_GPIO_8_DEFAULT_VAL, GPIO_GP0_GPIO_9_DEFAULT_VAL, GPIO_GP0_GPIO_10_DEFAULT_VAL, GPIO_GP0_GPIO_11_DEFAULT_VAL,
    GPIO_GP0_GPIO_12_DEFAULT_VAL, GPIO_GP0_GPIO_13_DEFAULT_VAL, GPIO_GP0_GPIO_14_DEFAULT_VAL, GPIO_GP0_GPIO_15_DEFAULT_VAL,
    GPIO_GP0_GPIO_16_DEFAULT_VAL, GPIO_GP0_GPIO_17_DEFAULT_VAL, GPIO_GP0_GPIO_18_DEFAULT_VAL, GPIO_GP0_GPIO_19_DEFAULT_VAL,
    GPIO_GP0_GPIO_20_DEFAULT_VAL, GPIO_GP0_GPIO_21_DEFAULT_VAL, GPIO_GP0_GPIO_22_DEFAULT_VAL, GPIO_GP0_GPIO_23_DEFAULT_VAL,
    GPIO_GP0_GPIO_24_DEFAULT_VAL, GPIO_GP0_GPIO_25_DEFAULT_VAL, GPIO_GP0_GPIO_26_DEFAULT_VAL, GPIO_GP0_GPIO_27_DEFAULT_VAL,
    GPIO_GP0_GPIO_28_DEFAULT_VAL, GPIO_GP0_GPIO_29_DEFAULT_VAL, GPIO_GP0_GPIO_30_DEFAULT_VAL, GPIO_GP0_GPIO_31_DEFAULT_VAL,
    GPIO_GP0_GPIO_32_DEFAULT_VAL, GPIO_GP0_GPIO_33_DEFAULT_VAL, GPIO_GP0_GPIO_34_DEFAULT_VAL, GPIO_GP0_GPIO_35_DEFAULT_VAL,
    GPIO_GP0_GPIO_36_DEFAULT_VAL, GPIO_GP0_GPIO_37_DEFAULT_VAL, GPIO_GP0_GPIO_38_DEFAULT_VAL, GPIO_GP0_GPIO_39_DEFAULT_VAL,
    GPIO_GPIO_INTR_RAW_STCLR1_DEFAULT_VAL,
    GPIO_GP0_INTR1_INTR_EN1_DEFAULT_VAL, GPIO_GP0_INTR1_INTR_STS1_DEFAULT_VAL,
    GPIO_GP0_INTR2_INTR_EN1_DEFAULT_VAL, GPIO_GP0_INTR2_INTR_STS1_DEFAULT_VAL,
    GPIO_GPIO_IO_CTRL_GROUP1_DEFAULT_VAL, GPIO_GPIO_IO_CTRL_GROUP2_DEFAULT_VAL,
    GPIO_GPIO_IO_CTRL_GROUP3_DEFAULT_VAL, GPIO_GPIO_IO_CTRL_GROUP4_DEFAULT_VAL,
    GPIO_GPIO_DOUT_GROUP1_DEFAULT_VAL, GPIO_GPIO_DOUT_GROUP2_DEFAULT_VAL,
    GPIO_GPIO_DOUT_GROUP3_DEFAULT_VAL, GPIO_GPIO_DOUT_GROUP4_DEFAULT_VAL,
    GPIO_GPIO_DIN_GROUP1_DEFAULT_VAL, GPIO_GPIO_DIN_GROUP2_DEFAULT_VAL,
    GPIO_GPIO_DIN_GROUP3_DEFAULT_VAL, GPIO_GPIO_DIN_GROUP4_DEFAULT_VAL
};

static const unsigned int read_mask_array[CNT] = {
    GPIO_GP0_GPIO_8_READ_MASK, GPIO_GP0_GPIO_9_READ_MASK, GPIO_GP0_GPIO_10_READ_MASK, GPIO_GP0_GPIO_11_READ_MASK,
    GPIO_GP0_GPIO_12_READ_MASK, GPIO_GP0_GPIO_13_READ_MASK, GPIO_GP0_GPIO_14_READ_MASK, GPIO_GP0_GPIO_15_READ_MASK,
    GPIO_GP0_GPIO_16_READ_MASK, GPIO_GP0_GPIO_17_READ_MASK, GPIO_GP0_GPIO_18_READ_MASK, GPIO_GP0_GPIO_19_READ_MASK,
    GPIO_GP0_GPIO_20_READ_MASK, GPIO_GP0_GPIO_21_READ_MASK, GPIO_GP0_GPIO_22_READ_MASK, GPIO_GP0_GPIO_23_READ_MASK,
    GPIO_GP0_GPIO_24_READ_MASK, GPIO_GP0_GPIO_25_READ_MASK, GPIO_GP0_GPIO_26_READ_MASK, GPIO_GP0_GPIO_27_READ_MASK,
    GPIO_GP0_GPIO_28_READ_MASK, GPIO_GP0_GPIO_29_READ_MASK, GPIO_GP0_GPIO_30_READ_MASK, GPIO_GP0_GPIO_31_READ_MASK,
    GPIO_GP0_GPIO_32_READ_MASK, GPIO_GP0_GPIO_33_READ_MASK, GPIO_GP0_GPIO_34_READ_MASK, GPIO_GP0_GPIO_35_READ_MASK,
    GPIO_GP0_GPIO_36_READ_MASK, GPIO_GP0_GPIO_37_READ_MASK, GPIO_GP0_GPIO_38_READ_MASK, GPIO_GP0_GPIO_39_READ_MASK,
    GPIO_GPIO_INTR_RAW_STCLR1_READ_MASK,
    GPIO_GP0_INTR1_INTR_EN1_READ_MASK, GPIO_GP0_INTR1_INTR_STS1_READ_MASK,
    GPIO_GP0_INTR2_INTR_EN1_READ_MASK, GPIO_GP0_INTR2_INTR_STS1_READ_MASK,
    GPIO_GPIO_IO_CTRL_GROUP1_READ_MASK, GPIO_GPIO_IO_CTRL_GROUP2_READ_MASK,
    GPIO_GPIO_IO_CTRL_GROUP3_READ_MASK, GPIO_GPIO_IO_CTRL_GROUP4_READ_MASK,
    GPIO_GPIO_DOUT_GROUP1_READ_MASK, GPIO_GPIO_DOUT_GROUP2_READ_MASK,
    GPIO_GPIO_DOUT_GROUP3_READ_MASK, GPIO_GPIO_DOUT_GROUP4_READ_MASK,
    GPIO_GPIO_DIN_GROUP1_READ_MASK, GPIO_GPIO_DIN_GROUP2_READ_MASK,
    GPIO_GPIO_DIN_GROUP3_READ_MASK, GPIO_GPIO_DIN_GROUP4_READ_MASK
};

static const unsigned int write_mask_array[CNT] = {
    GPIO_GP0_GPIO_8_WRITE_MASK, GPIO_GP0_GPIO_9_WRITE_MASK, GPIO_GP0_GPIO_10_WRITE_MASK, GPIO_GP0_GPIO_11_WRITE_MASK,
    GPIO_GP0_GPIO_12_WRITE_MASK, GPIO_GP0_GPIO_13_WRITE_MASK, GPIO_GP0_GPIO_14_WRITE_MASK, GPIO_GP0_GPIO_15_WRITE_MASK,
    GPIO_GP0_GPIO_16_WRITE_MASK, GPIO_GP0_GPIO_17_WRITE_MASK, GPIO_GP0_GPIO_18_WRITE_MASK, GPIO_GP0_GPIO_19_WRITE_MASK,
    GPIO_GP0_GPIO_20_WRITE_MASK, GPIO_GP0_GPIO_21_WRITE_MASK, GPIO_GP0_GPIO_22_WRITE_MASK, GPIO_GP0_GPIO_23_WRITE_MASK,
    GPIO_GP0_GPIO_24_WRITE_MASK, GPIO_GP0_GPIO_25_WRITE_MASK, GPIO_GP0_GPIO_26_WRITE_MASK, GPIO_GP0_GPIO_27_WRITE_MASK,
    GPIO_GP0_GPIO_28_WRITE_MASK, GPIO_GP0_GPIO_29_WRITE_MASK, GPIO_GP0_GPIO_30_WRITE_MASK, GPIO_GP0_GPIO_31_WRITE_MASK,
    GPIO_GP0_GPIO_32_WRITE_MASK, GPIO_GP0_GPIO_33_WRITE_MASK, GPIO_GP0_GPIO_34_WRITE_MASK, GPIO_GP0_GPIO_35_WRITE_MASK,
    GPIO_GP0_GPIO_36_WRITE_MASK, GPIO_GP0_GPIO_37_WRITE_MASK, GPIO_GP0_GPIO_38_WRITE_MASK, GPIO_GP0_GPIO_39_WRITE_MASK,
    GPIO_GPIO_INTR_RAW_STCLR1_WRITE_MASK,
    GPIO_GP0_INTR1_INTR_EN1_WRITE_MASK, GPIO_GP0_INTR1_INTR_STS1_WRITE_MASK,
    GPIO_GP0_INTR2_INTR_EN1_WRITE_MASK, GPIO_GP0_INTR2_INTR_STS1_WRITE_MASK,
    GPIO_GPIO_IO_CTRL_GROUP1_WRITE_MASK, GPIO_GPIO_IO_CTRL_GROUP2_WRITE_MASK,
    GPIO_GPIO_IO_CTRL_GROUP3_WRITE_MASK, GPIO_GPIO_IO_CTRL_GROUP4_WRITE_MASK,
    GPIO_GPIO_DOUT_GROUP1_WRITE_MASK, GPIO_GPIO_DOUT_GROUP2_WRITE_MASK,
    GPIO_GPIO_DOUT_GROUP3_WRITE_MASK, GPIO_GPIO_DOUT_GROUP4_WRITE_MASK,
    GPIO_GPIO_DIN_GROUP1_WRITE_MASK, GPIO_GPIO_DIN_GROUP2_WRITE_MASK,
    GPIO_GPIO_DIN_GROUP3_WRITE_MASK, GPIO_GPIO_DIN_GROUP4_WRITE_MASK
};

static const unsigned int skip_array[CNT] = {
    /* all zeros per directive */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static const unsigned int skip_rst_array[CNT] = {
    /* all zeros per directive */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

void test_case(void)
{
    unsigned int def_fail_cnt = 0; 
    unsigned int wr_fail_cnt = 0;

#ifdef DEBUG_DISPLAY
    printf("[gpio_reg_wr_rd_test] Start default value checks and masked write/read...\n");
#endif

    /* Default value checks */
    for (unsigned int i = 0; i < CNT; i++) {
        if (skip_rst_array[i] == 1U) {
#ifdef DEBUG_DISPLAY
            printf("  [i=%u] Skipped by skip_rst_array.\n", i);
#endif
            continue;
        }
        if (read_mask_array[i] == 0x00000000U) continue;
        unsigned int rd = read_reg(addr_array[i]);
        unsigned int data = (rd & 0xFFFFFFFEU);
        unsigned int exp  = default_value_array[i];
        if (data != exp) {
            def_fail_cnt++;
#ifdef DEBUG_DISPLAY
            printf("  [i=%u] DEFAULT MISMATCH: addr=0x%08lX rd=0x%08X exp=0x%08X\n", i, addr_array[i], data, exp);
#endif
        }
    }

    /* Masked write/read checks */
    const unsigned int patterns[] = {
        0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U, 0xF5F5F5F5U, 0xA5A5A5A5U, 0xFFFF0000U
    };

    for (unsigned int p = 0; p < (sizeof(patterns)/sizeof(patterns[0])); p++) {
        unsigned int data_wr = patterns[p];
#ifdef DEBUG_DISPLAY
        printf("  Pattern %u: 0x%08X\n", p, data_wr);
#endif
        for (unsigned int i = 0; i < CNT; i++) {
            if (skip_array[i] == 1U) continue;
            unsigned int wmask = write_mask_array[i];
            unsigned int rmask = read_mask_array[i];
            if (wmask == 0x00000000U) continue;
            /* Perform masked write */
            write_reg(addr_array[i], (data_wr & wmask));
            if ((wmask == 0x00000000U) || (rmask == 0x00000000U)) continue;

            unsigned int data_rd = (read_reg(addr_array[i]) & rmask);
            unsigned int wr_n = (wmask ^ 0xFFFFFFFFU);
            unsigned int exp_val = ((data_wr & rmask & wmask) | (wr_n & rmask & default_value_array[i]));
            if (data_rd != exp_val) {
                wr_fail_cnt++;
#ifdef DEBUG_DISPLAY
                printf("    [i=%u] WRITE/READ MISMATCH: addr=0x%08lX rd=0x%08X exp=0x%08X wmask=0x%08X rmask=0x%08X\n",
                       i, addr_array[i], data_rd, exp_val, wmask, rmask);
#endif
            }
        }
    }

#ifdef DEBUG_DISPLAY
    printf("[gpio_reg_wr_rd_test] def_fail_cnt=%u, wr_fail_cnt=%u\n", def_fail_cnt, wr_fail_cnt);
#endif

    if ((def_fail_cnt > 0U) || (wr_fail_cnt > 0U)) {
        finish(1);
    } else {
        finish(0);
    }
}

/* Default IRQ handler (not used in this test; provided for completeness) */
void Default_IRQHandler(void)
{
#ifdef DEBUG_DISPLAY
    printf("[gpio_reg_wr_rd_test] Default_IRQHandler invoked (no-op)\n");
#endif
}
