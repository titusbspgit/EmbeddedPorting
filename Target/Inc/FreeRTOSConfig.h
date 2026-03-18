#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

#ifndef configCPU_CLOCK_HZ
#define configCPU_CLOCK_HZ              ( SystemCoreClock )
#endif
#define configTICK_RATE_HZ              ( ( TickType_t ) 1000 )
#define configUSE_PREEMPTION            1
#define configUSE_IDLE_HOOK             0
#define configUSE_TICK_HOOK             0
#define configMAX_PRIORITIES            ( 7 )
#define configMINIMAL_STACK_SIZE        ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE           ( ( size_t ) ( 32 * 1024 ) )
#define configMAX_TASK_NAME_LEN         ( 16 )
#define configUSE_16_BIT_TICKS          0
#define configIDLE_SHOULD_YIELD         1

/* Co-routines */
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Software timer definitions. */
#define configUSE_TIMERS                1
#define configTIMER_TASK_PRIORITY       ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH        10
#define configTIMER_TASK_STACK_DEPTH    256

/* Hook function related definitions. */
#define configUSE_MALLOC_FAILED_HOOK    1
#define configCHECK_FOR_STACK_OVERFLOW  2

/* CMSIS-RTOS V2 */
#define configUSE_OS2_THREAD_SUSPEND_RESUME 0

/* Interrupt priorities - STM32F4 uses 4 bits of priority. */
#define configKERNEL_INTERRUPT_PRIORITY         ( 15 << 4 )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( 5 << 4 )

/* Assert */
void vAssertCalled(const char *file, int line);
#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); vAssertCalled(__FILE__, __LINE__); for( ;; ); }

#endif /* FREERTOS_CONFIG_H */
