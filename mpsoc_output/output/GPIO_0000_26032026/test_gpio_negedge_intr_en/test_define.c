#include <test_common.h>
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>
#include <lss_sysreg.h>

/* Optional helper: base GPIO0 per-pin config register list starting at MIZAR_GPIO_GP0_GPIO_8 */
static const unsigned long int gpio0_pin_cfg_base = MIZAR_GPIO_GP0_GPIO_8;

/* IO control group macros are provided by gpio_def/offset */
