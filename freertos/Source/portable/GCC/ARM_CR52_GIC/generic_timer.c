/*
 * FreeRTOS Kernel V11.1.0
 * Copyright 2024 NXP.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#include <stdint.h>
#include "generic_timer.h"
#include "FreeRTOSConfig.h"
#if defined(_S32DS_FREERTOS_)
#include "OsIf_ArchCfg.h"
#include "core.h"
#else
#include "gic.h"
#endif
#include "FreeRTOS.h"

static volatile uint8_t PRIVILEGED_DATA cnt_div = 1;        /** The value is read from CFG_CNTDV to calibrate the timer counter(cnt_val). Default by 1 */

extern void SysTick_Handler(void);
#if ( configNUMBER_OF_CORES > 1 )
extern void vInitInterruptTable(int, int, void*);
#else
extern void vInitInterruptTable(int, void*);
#endif

#ifdef CPU_S32K566

#define STM_TIMER_USE 1

uint32_t STM_BASE_ADDRS[2] = {
    0x410a4000u,
    0x41154000u,
};

#define CFG_STM_ID 0
#define CFG_STM_CHANNEL 3 /* Range form 0 to 3*/

#define STM_CLOCK_HZ 8000000

#define STM_0_INT_ID(ch) (ch == 0 ? 315 : (ch == 1 ? 316 : (ch == 2 ? 317 : 318)))

#define STM_1_INT_ID(ch) (ch == 0 ? 319 : (ch == 1 ? 320 : (ch == 2 ? 321 : 322)))

#define STM_INT_ID(stm_id, ch) (stm_id == 0 ? STM_0_INT_ID(ch) : STM_1_INT_ID(ch))

#define STM_READ_CLOCK_DIV(stm_id, val) \
    val = (((*(volatile uint32_t *)(STM_BASE_ADDRS[stm_id])) >> 0x8) & 0xff) + 0x1;

#define STM_READ_CNT(stm_id, val) \
    val = *(volatile uint32_t *)(STM_BASE_ADDRS[stm_id] + 0x4);

#define STM_CHANNEL_ENABLE(stm_id, ch) \
    *(volatile uint32_t *)(STM_BASE_ADDRS[stm_id] + ((ch + 0x1) * 0x10)) = 0x1;

#define STM_CHANNEL_DISABLE(stm_id, ch) \
    *(volatile uint32_t *)(STM_BASE_ADDRS[stm_id] + ((ch + 0x1) * 0x10)) = 0x0;

#define STM_CHANNEL_READ_CMP(stm_id, ch, val) \
    val = *(volatile uint32_t *)(STM_BASE_ADDRS[stm_id] + ((ch + 0x1) * 0x10) + 0x8);

#define STM_CHANNEL_SET_CMP(stm_id, ch, val) \
    *(volatile uint32_t *)(STM_BASE_ADDRS[stm_id] + ((ch + 0x1) * 0x10) + 0x8) = val;

#define STM_CHANNEL_CLEAR_INT_FLAG(stm_id, ch) \
    *(volatile uint32_t *)(STM_BASE_ADDRS[stm_id] + ((ch + 0x1) * 0x10) + 0x4) = 0x1;

#define STM_TIMER_INIT(stm_id, ch, cmp) \
    *(volatile uint32_t *)(STM_BASE_ADDRS[stm_id]) |= 0x1; /* enable TEN */ \
    STM_CHANNEL_DISABLE(stm_id, ch); \
    STM_CHANNEL_CLEAR_INT_FLAG(stm_id, ch); \
    STM_CHANNEL_SET_CMP(stm_id, ch, cmp);\
    STM_CHANNEL_ENABLE(stm_id, ch);
#else

/*
 * Local function access to core registers
 */
#define READ_MPIDR_INSTR                    "mrc p15, 0, %[Rd], c0, c0, 5"

#if (configUSE_PHYSICAL_TIMER == 1)
#define WRITE_CNTP_CTL_INSTR                "mcr p15, 0, %[Rd], c14, c2, 1"
#define READ_CNTP_CTL_INSTR                 "mrc p15, 0, %[Rd], c14, c2, 1"
#define READ_CNTPCT_INSTR                   "mrrc p15, 0, %[Rd], %[Rd2], c14"
#define WRITE_CNTP_CVAL_INSTR               "mcrr p15, 2, %[Rd], %[Rd2], c14"
#define READ_CNTP_CVAL_INSTR                "mrrc p15, 2, %[Rd_lo], %[Rd_hi], c14"
#define TIMER_INT_ID                        PHYSICAL_TIMER_INT_ID
#endif

#if (configUSE_VIRTUAL_TIMER == 1)
#define WRITE_CNTV_CTL_INSTR                "mcr p15, 0, %[Rd], c14, c3, 1"
#define READ_CNTV_CTL_INSTR                 "mrc p15, 0, %[Rd], c14, c3, 1"
#define READ_CNTVCT_INSTR                   "mrrc p15, 1, %[Rd], %[Rd2], c14"
#define WRITE_CNTV_CVAL_INSTR               "mcrr p15, 3, %[Rd], %[Rd2], c14"
#define READ_CNTV_CVAL_INSTR                "mrrc p15, 3, %[Rd_lo], %[Rd_hi], c14"
#define TIMER_INT_ID                        VIRTUAL_TIMER_INT_ID
#endif

#if (configUSE_PHYSICAL_TIMER == 1)
/*-----------------------------------------------------------*/

static uint64_t read_CNTP_CVAL(void);
static uint64_t read_CNTP_CVAL(void)
{
    uint32_t low = 0;
    uint32_t high = 0;
    ASMV_KEYWORD(
        READ_CNTP_CVAL_INSTR
        : [Rd_lo] "=r" (low), [Rd_hi] "=r" (high)
    );
    return ((uint64_t)high << 32) | low;
}

/*-----------------------------------------------------------*/

static void write_CNTP_CTL(uint32_t val);
static void write_CNTP_CTL(uint32_t val)
{
    uint32_t val32 = val;
    ASMV_KEYWORD(
        WRITE_CNTP_CTL_INSTR
        : /* No Output */
        : [Rd]"r"(val32)
    );
}
/*-----------------------------------------------------------*/

static uint32_t read_CNTP_CTL(void);
static uint32_t read_CNTP_CTL(void)
{
    uint64_t cntp_ctl = 0;
    ASMV_KEYWORD(
        READ_CNTP_CTL_INSTR
        : [Rd]"=r"(cntp_ctl)
    );
    return cntp_ctl;
}
/*-----------------------------------------------------------*/

static void write_CNTP_CVAL(uint64_t val);
static void write_CNTP_CVAL(uint64_t val)
{
    uint32_t val32 = val;
    uint32_t val32_1 = val >> 32;
    ASMV_KEYWORD(
        WRITE_CNTP_CVAL_INSTR
        : /* No Output */
        : [Rd]"r"(val32),
          [Rd2]"r"(val32_1):
    );
}
/*-----------------------------------------------------------*/

static uint64_t read_CNTPCT(void);
static uint64_t read_CNTPCT(void)
{
    uint64_t cntp_cval = 0;
    uint32_t val1,val2;
    ASMV_KEYWORD(
        READ_CNTPCT_INSTR
        : [Rd]"=r"(val1),
          [Rd2]"=r"(val2):
    );
    cntp_cval = ((uint64_t)val2);
    cntp_cval = (cntp_cval << 32);
    cntp_cval |= val1;
    return cntp_cval;
}
/*-----------------------------------------------------------*/
#endif

#if (configUSE_VIRTUAL_TIMER == 1)
/*-----------------------------------------------------------*/

static uint64_t read_CNTV_CVAL(void);
static uint64_t read_CNTV_CVAL(void)
{
    uint32_t low = 0;
    uint32_t high = 0;
    ASMV_KEYWORD(
        READ_CNTV_CVAL_INSTR
        : [Rd_lo] "=r" (low), [Rd_hi] "=r" (high)
    );
    return ((uint64_t)high << 32) | low;
}

/*-----------------------------------------------------------*/

static void write_CNTV_CTL(uint32_t val);
static void write_CNTV_CTL(uint32_t val)
{
    uint32_t val32 = val;
    ASMV_KEYWORD(
        WRITE_CNTV_CTL_INSTR
        : /* No Output */
        : [Rd]"r"(val32)
    );
}
/*-----------------------------------------------------------*/

static uint32_t read_CNTV_CTL(void);
static uint32_t read_CNTV_CTL(void)
{
    uint64_t cntp_ctl = 0;
    ASMV_KEYWORD(
        READ_CNTV_CTL_INSTR
        : [Rd]"=r"(cntp_ctl)
    );
    return cntp_ctl;
}
/*-----------------------------------------------------------*/

static void write_CNTV_CVAL(uint64_t val);
static void write_CNTV_CVAL(uint64_t val)
{
    uint32_t val32 = val;
    uint32_t val32_1 = val >> 32;
    ASMV_KEYWORD(
        WRITE_CNTV_CVAL_INSTR
        : /* No Output */
        : [Rd]"r"(val32),
          [Rd2]"r"(val32_1):
    );
}
/*-----------------------------------------------------------*/

static uint64_t read_CNTVCT(void);
static uint64_t read_CNTVCT(void)
{
    uint64_t cntv_cval = 0;
    uint32_t val1,val2;
    ASMV_KEYWORD(
        READ_CNTVCT_INSTR
        : [Rd]"=r"(val1),
          [Rd2]"=r"(val2):
    );
    cntv_cval = ((uint64_t)val2);
    cntv_cval = (cntv_cval << 32);
    cntv_cval |= val1;
    return cntv_cval;
}
/*-----------------------------------------------------------*/
#endif

static uint32_t read_MPIDR(void);
static uint32_t read_MPIDR(void)
{
    uint64_t mpidr = 0;
    ASMV_KEYWORD(
        READ_MPIDR_INSTR
        : [Rd]"=r"(mpidr)
    );
    return mpidr;
}
/*-----------------------------------------------------------*/

/*
 *   Get cluster
 */
uint32_t GET_CLUSTER_ID(void) PRIVILEGED_FUNCTION;
uint32_t GET_CLUSTER_ID(void)
{
	return MPIDR_GET_CLUSTER(read_MPIDR());
}
/*-----------------------------------------------------------*/

#endif

/*
 * Update the tick count to next interrupt
 */
void vUpdateTimer(void) __attribute__((weak)) PRIVILEGED_FUNCTION;
void vUpdateTimer(void)
{
    uint64_t cmp_val;
    uint64_t counter;
    uint32_t ulCPSR;
    uint64_t period;

#ifdef STM_TIMER_USE
    period = (uint64_t)((STM_CLOCK_HZ / cnt_div) / configTICK_RATE_HZ);
    STM_CHANNEL_DISABLE(CFG_STM_ID, CFG_STM_CHANNEL);
    STM_CHANNEL_CLEAR_INT_FLAG(CFG_STM_ID, CFG_STM_CHANNEL);
    STM_CHANNEL_READ_CMP(CFG_STM_ID, CFG_STM_CHANNEL, cmp_val);

    OSASM(" mrs %[out], cpsr     \t\n"
           " cpsid if            \t\n"
           : [out] "=r"(ulCPSR)
    );

    do
    {
        /* Update compare value to next period */
        cmp_val += period;
        STM_CHANNEL_SET_CMP(CFG_STM_ID, CFG_STM_CHANNEL, cmp_val);
        /* Get current counter */
        STM_READ_CNT(CFG_STM_ID, counter);
    } while (counter > cmp_val);

    OSASM(" msr cpsr_c, %[in]    \t\n"
           :: [in] "r"(ulCPSR)
    );
    STM_CHANNEL_ENABLE(CFG_STM_ID, CFG_STM_CHANNEL);

#else

    period = (uint64_t)((configCPU_CLOCK_HZ / cnt_div) / configTICK_RATE_HZ);

#if ((configUSE_PHYSICAL_TIMER == 1) && (configUSE_VIRTUAL_TIMER == 1))
    configASSERT(ERROR);

#elif (configUSE_PHYSICAL_TIMER == 1)
    cmp_val = read_CNTP_CVAL();

    OSASM(" mrs %[out], cpsr     \t\n"    /* save current CPSR to ulCPSR */
           " cpsid if            \t\n"    /* disable interrupts*/
           : [out] "=r"(ulCPSR)
    );

    do
    {
        /* Update compare value to next period */
        cmp_val += period;
        write_CNTP_CVAL(cmp_val);
        /* Get current counter */
        counter = read_CNTPCT();
    } while (counter > cmp_val);

    write_CNTP_CTL(read_CNTP_CTL() | ENABLE);
    OSASM(" msr cpsr_c, %[in]    \t\n"    /* Restore saved CPSR */
           :: [in] "r"(ulCPSR)
    );

#elif (configUSE_VIRTUAL_TIMER == 1)
    cmp_val = read_CNTV_CVAL();

    OSASM(" mrs %[out], cpsr     \t\n"
           " cpsid if            \t\n"
           : [out] "=r"(ulCPSR)
    );

    do
    {
        /* Update compare value to next period */
        cmp_val += period;
        write_CNTV_CVAL(cmp_val);
        /* Get current counter */
        counter = read_CNTVCT();
    } while (counter > cmp_val);

    write_CNTV_CTL(read_CNTV_CTL() | ENABLE);
    OSASM(" msr cpsr_c, %[in]    \t\n"
           :: [in] "r"(ulCPSR)
    );

#else
    configASSERT(ERROR);
#endif

#endif
}

/*-----------------------------------------------------------*/

/*
 * Setup the generic timer to generate the tick interrupts at the required
 * frequency.
 */
void prvSetupTimerInterrupt(void) __attribute__( ( weak ) ) PRIVILEGED_FUNCTION;
void prvSetupTimerInterrupt(void)
{
    uint64_t cnt_val = 0;

#ifdef STM_TIMER_USE
    STM_READ_CLOCK_DIV(CFG_STM_ID, cnt_div);
    STM_READ_CNT(CFG_STM_ID, cnt_val);
    cnt_val += (uint64_t)(uint64_t)((STM_CLOCK_HZ / cnt_div ) / configTICK_RATE_HZ);
    STM_TIMER_INIT(CFG_STM_ID, CFG_STM_CHANNEL, cnt_val);
#if defined(_S32DS_FREERTOS_)
    Core_registerIsrHandler(STM_INT_ID(CFG_STM_ID, CFG_STM_CHANNEL), SysTick_Handler);
    Core_enableIsrSource(STM_INT_ID(CFG_STM_ID, CFG_STM_CHANNEL), TIMER_PRIO);
#else
    vInitInterruptTable(STM_INT_ID(CFG_STM_ID, CFG_STM_CHANNEL), SysTick_Handler);
    vGicEnableInterrupt(STM_INT_ID(CFG_STM_ID, CFG_STM_CHANNEL), TIMER_PRIO);
#endif

#else

#if defined(S32Z27) || defined(S32E27) || defined(S32E28)
    uint32_t cluster = 0;

    /* Setup divider value used for generating the Cortex-R52 generic timer clock-enable */
    cluster = GET_CLUSTER_ID();
    cnt_div = READ_CFG_CNTDV(cluster) + 1;  /** The value of CFG_CNTDV is 0-7 (3bit): 0 <=> div 1 */
#else
    cnt_div = 1;
#endif

#if((configUSE_PHYSICAL_TIMER == 1)&&(configUSE_VIRTUAL_TIMER == 1))
    configASSERT(ERROR);
#elif (configUSE_PHYSICAL_TIMER == 1)
    cnt_val = read_CNTPCT();

    /* Calculate the constants required to configure the tick interrupt. */
    cnt_val += (uint64_t)((configCPU_CLOCK_HZ/cnt_div)/configTICK_RATE_HZ);
    write_CNTP_CTL(read_CNTP_CTL() | ENABLE);
    write_CNTP_CVAL(cnt_val);
#elif (configUSE_VIRTUAL_TIMER == 1)
    cnt_val = read_CNTVCT();

    /* Calculate the constants required to configure the tick interrupt. */
    cnt_val += (uint64_t)((configCPU_CLOCK_HZ/cnt_div)/configTICK_RATE_HZ);
    write_CNTV_CTL(read_CNTV_CTL() | ENABLE);
    write_CNTV_CVAL(cnt_val);
#else
    configASSERT(ERROR);
#endif
    /* Configure SysTick to interrupt at the requested rate. */

#if defined(_S32DS_FREERTOS_)
    Core_registerIsrHandler(TIMER_INT_ID, SysTick_Handler);
    Core_enableIsrSource(TIMER_INT_ID, OSINTC_GIC_CONVERT_PRIO_SET(TIMER_PRIO));
#else
#if ( configNUMBER_OF_CORES > 1 )
    vInitInterruptTable(configCORE_MASTER, TIMER_INT_ID, SysTick_Handler);
#else
    vInitInterruptTable(TIMER_INT_ID, SysTick_Handler);
#endif
    vGicEnableInterrupt(TIMER_INT_ID, TIMER_PRIO);
#endif

#endif
}
