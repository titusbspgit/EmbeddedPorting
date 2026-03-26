/*
 * Auto-generated GPIO testcase: test_gpio_pedge_all_pads_en
 * Run folder: gpio_1402_26032026
 * Notes:
 * - Header includes and macro defines sourced from Code_Gen_Helper.
 * - Register specs via RAG were unavailable for the impacted macros; keep TODOs.
 * - Impacted Registers (CSV):
 *   MIZAR_LSS_SYSREG_INTR_EN1,
 *   MIZAR_GPIO_GP0_GPIO_8,
 *   MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,
 *   MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,
 *   MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,
 *   MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,
 *   MIZAR_GPIO_GP0_INTR1_INTR_EN1,
 *   MIZAR_GPIO_GP0_INTR1_INTR_STS1,
 *   MIZAR_LSS_SYSREG_RAW_STCR1
 */

/* Header includes (from helper) */
#include <lss_sysreg.h>
#include <stdio.h>
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

/* Macro defines (from helper) */
#define CNT 49

/* Shared test state (structure only) */
static volatile int int_pend = 0;
static volatile unsigned int g_current_pin;

/* Skip register list mapped to CSV order (none provided by helper) */
/* Order: {LSS_SYSREG_INTR_EN1, GP0_GPIO_8, IO_CTRL_GRP1..4, INTR1_EN1, INTR1_STS1, LSS_SYSREG_RAW_STCR1} */
static unsigned char skip_array[9] = {0,0,0,0,0,0,0,0,0};

/* Forward declarations */
void test_case(void);
void Default_IRQHandler(void);

/* RAG Register Map (unavailable placeholders) */
/*   MIZAR_LSS_SYSREG_INTR_EN1 : <no spec available>
 *   MIZAR_GPIO_GP0_GPIO_8     : <no spec available>
 *   MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4 : <no spec available>
 *   MIZAR_GPIO_GP0_INTR1_INTR_EN1   : <no spec available>
 *   MIZAR_GPIO_GP0_INTR1_INTR_STS1  : <no spec available>
 *   MIZAR_LSS_SYSREG_RAW_STCR1      : <no spec available>
 */
