/*
 * Auto-generated GPIO testcase: test_gpio_negedge_intr_en
 * Run folder: gpio_1402_26032026
 * Notes:
 * - Header includes and macro defines sourced from Code_Gen_Helper.
 * - Register specs via RAG were unavailable for the impacted macros; keep TODOs.
 * - Impacted Registers (CSV):
 *   MIZAR_LSS_SYSREG_INTR_EN1,
 *   MIZAR_GPIO_GP0_GPIO_8,
 *   MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,
 *   MIZAR_GPIO_GP0_INTR1_INTR_EN1,
 *   MIZAR_GPIO_GP0_INTR1_INTR_STS1,
 *   MIZAR_LSS_SYSREG_RAW_STCR1
 */

/* Header includes (from helper) */
#include <stdio.h>
#include <lss_sysreg.h>
#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

/* Macro defines (from helper) */
#define CNT 49

/* Shared test state and helpers (structure only) */
static volatile int int_pend = 0;            /* set to 1 before edge, cleared in ISR */
static volatile unsigned int g_current_pin;  /* pin index 0..31 mapped to GPIO[8..39] */

/* Skip register list mapped to CSV order (none provided by helper) */
/* Order: {LSS_SYSREG_INTR_EN1, GP0_GPIO_8, GPIO_INTR_RAW_STCLR1, GP0_INTR1_INTR_EN1, GP0_INTR1_INTR_STS1, LSS_SYSREG_RAW_STCR1} */
static unsigned char skip_array[6] = {0,0,0,0,0,0};

/* Forward declarations to match harness expectations */
void test_case(void);
void Default_IRQHandler(void);

/* RAG Register Map (unavailable, kept for traceability)
   - MIZAR_LSS_SYSREG_INTR_EN1 : <no spec available>
   - MIZAR_GPIO_GP0_GPIO_8     : <no spec available>
   - MIZAR_GPIO_GPIO_INTR_RAW_STCLR1 : <no spec available>
   - MIZAR_GPIO_GP0_INTR1_INTR_EN1   : <no spec available>
   - MIZAR_GPIO_GP0_INTR1_INTR_STS1  : <no spec available>
   - MIZAR_LSS_SYSREG_RAW_STCR1      : <no spec available>
*/
