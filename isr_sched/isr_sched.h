/**
 * @file isr_sched.h
 * @ingroup kolblib_isr_sched
 * @brief Parallel multi-rate ISR scheduler for S32K344 (PIT0), beside FreeRTOS.
 *
 * Rates (PIT0 channels, shared NVIC vector @c PIT0_IRQn):
 * - CH0: 62.5 µs (serviced first in ISR)
 * - CH1: 205 µs
 * - CH2: 1 ms
 * - CH3: 10 ms
 *
 * FreeRTOS runs via @c vTaskStartScheduler() (lowest layer). See kolblib/README.md.
 */
#ifndef KOLBLIB_ISR_SCHED_H
#define KOLBLIB_ISR_SCHED_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup kolblib Kolblib shared libraries
 * @brief Hand-maintained libs under @c wsnxp/kolblib (not NXP generate).
 * @{
 */

/**
 * @defgroup kolblib_isr_sched ISR multi-rate scheduler
 * @ingroup kolblib
 * @brief PIT0 CH0–CH3 callbacks + sticky flags for task polling.
 * @{
 */

/** @brief Assumes PIT0_CLK = 40 MHz (T102test Clock Tools). */
#ifndef ISR_SCHED_PIT_CLK_HZ
#define ISR_SCHED_PIT_CLK_HZ          (40000000UL)
#endif

#define ISR_SCHED_FLAG_62US5          (1u << 0)
#define ISR_SCHED_FLAG_205US          (1u << 1)
#define ISR_SCHED_FLAG_1MS            (1u << 2)
#define ISR_SCHED_FLAG_10MS           (1u << 3)

/** @brief Optional callback invoked from PIT0 ISR (keep extremely short). */
typedef void (*IsrSched_Callback_t)(void);

/**
 * @brief Callback table for the four PIT rates (NULL = unused).
 */
typedef struct
{
    IsrSched_Callback_t cb_62us5; /**< ISR context */
    IsrSched_Callback_t cb_205us;
    IsrSched_Callback_t cb_1ms;
    IsrSched_Callback_t cb_10ms;
} IsrSched_Config_t;

/**
 * @brief Init PIT0 timers + NVIC. Call after @c Clock_Ip_Init.
 * @param[in] cfg Callbacks (may be NULL → all NULL).
 * @code
 * IsrSched_Config_t c = { .cb_62us5 = FrdmA_Adc8_OnIsr62us5 };
 * IsrSched_Init(&c);
 * @endcode
 */
void IsrSched_Init(const IsrSched_Config_t *cfg);

/**
 * @brief Enable all four PIT channels (start ticking).
 * @code
 * IsrSched_Start();
 * @endcode
 */
void IsrSched_Start(void);

/**
 * @brief Disable all four PIT channels.
 * @code
 * IsrSched_Stop();
 * @endcode
 */
void IsrSched_Stop(void);

/**
 * @brief Atomically read-and-clear sticky rate flags set from ISR.
 * @return Bitmask of @c ISR_SCHED_FLAG_*.
 * @code
 * uint32_t f = IsrSched_GetFlags();
 * @endcode
 */
uint32_t IsrSched_GetFlags(void);

/**
 * @brief PIT0 IRQ handler — install in vector table (@c Vector_Table.s).
 * @code
 * // Vector_Table.s: .word IsrSched_PIT0_IRQHandler
 * @endcode
 */
void IsrSched_PIT0_IRQHandler(void);

/** @} */ /* kolblib_isr_sched */
/** @} */ /* kolblib */

#ifdef __cplusplus
}
#endif

#endif /* KOLBLIB_ISR_SCHED_H */
