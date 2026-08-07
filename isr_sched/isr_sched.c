/**
 * @file isr_sched.c
 * @ingroup kolblib_isr_sched
 * @brief PIT0 multi-rate scheduler for S32K344 (register-level, no Gpt required).
 *
 * If Gpt is later generated for PIT0, keep this module or replace hooks —
 * do not enable both on the same PIT0 channels.
 */
#include "isr_sched.h"

#include "S32K344.h"
#include "nvic.h" /* NVIC_EnableIRQ / NVIC_SetPriority; pulls Std_Types (uint8) + NULL */

#ifndef PIT0_IRQn
#define PIT0_IRQn   (96)
#endif

/* LDVAL = period_ticks - 1. period_s * clk = ticks */
#define ISR_SCHED_LDVAL_62US5 \
    ((ISR_SCHED_PIT_CLK_HZ / 16000UL) - 1UL)   /* 40e6 * 62.5e-6 = 2500 */
#define ISR_SCHED_LDVAL_205US \
    ((ISR_SCHED_PIT_CLK_HZ * 205UL) / 1000000UL - 1UL) /* 8200 */
#define ISR_SCHED_LDVAL_1MS \
    ((ISR_SCHED_PIT_CLK_HZ / 1000UL) - 1UL)    /* 40000 */
#define ISR_SCHED_LDVAL_10MS \
    ((ISR_SCHED_PIT_CLK_HZ / 100UL) - 1UL)     /* 400000 */

static IsrSched_Config_t s_cfg;
static volatile uint32_t s_flags;

static void IsrSched_ChannelInit(uint32_t ch, uint32_t ldval)
{
    IP_PIT_0->TIMER[ch].TCTRL = 0U;
    IP_PIT_0->TIMER[ch].LDVAL = ldval;
    IP_PIT_0->TIMER[ch].TFLG  = PIT_TFLG_TIF_MASK; /* w1c */
    IP_PIT_0->TIMER[ch].TCTRL = PIT_TCTRL_TIE_MASK; /* enable IRQ, TEN set in Start */
}

void IsrSched_Init(const IsrSched_Config_t *cfg)
{
    if (cfg != NULL)
    {
        s_cfg = *cfg;
    }
    else
    {
        s_cfg.cb_62us5 = NULL;
        s_cfg.cb_205us = NULL;
        s_cfg.cb_1ms = NULL;
        s_cfg.cb_10ms = NULL;
    }
    s_flags = 0U;

    /* Module enable (clear MDIS) */
    IP_PIT_0->MCR = 0U;

    IsrSched_ChannelInit(0U, ISR_SCHED_LDVAL_62US5);
    IsrSched_ChannelInit(1U, ISR_SCHED_LDVAL_205US);
    IsrSched_ChannelInit(2U, ISR_SCHED_LDVAL_1MS);
    IsrSched_ChannelInit(3U, ISR_SCHED_LDVAL_10MS);

    /*
     * Shared PIT0 vector: pick a priority numerically ABOVE FreeRTOS max-syscall
     * (lower urgency) so FromISR remains legal if used later. Default: flags only.
     * configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY is typically 5 → use 6 here.
     */
    NVIC_SetPriority((uint8)PIT0_IRQn, (uint8)6U);
    NVIC_EnableIRQ((uint8)PIT0_IRQn);
}

void IsrSched_Start(void)
{
    uint32_t ch;
    for (ch = 0U; ch < 4U; ch++)
    {
        IP_PIT_0->TIMER[ch].TFLG = PIT_TFLG_TIF_MASK;
        IP_PIT_0->TIMER[ch].TCTRL |= PIT_TCTRL_TEN_MASK;
    }
}

void IsrSched_Stop(void)
{
    uint32_t ch;
    for (ch = 0U; ch < 4U; ch++)
    {
        IP_PIT_0->TIMER[ch].TCTRL &= (uint32_t)~PIT_TCTRL_TEN_MASK;
    }
}

uint32_t IsrSched_GetFlags(void)
{
    uint32_t f;
    __asm volatile ("cpsid i" ::: "memory");
    f = s_flags;
    s_flags = 0U;
    __asm volatile ("cpsie i" ::: "memory");
    return f;
}

void IsrSched_PIT0_IRQHandler(void)
{
    /* Service highest rate first (software priority). */
    if ((IP_PIT_0->TIMER[0].TFLG & PIT_TFLG_TIF_MASK) != 0U)
    {
        IP_PIT_0->TIMER[0].TFLG = PIT_TFLG_TIF_MASK;
        s_flags |= ISR_SCHED_FLAG_62US5;
        if (s_cfg.cb_62us5 != NULL)
        {
            s_cfg.cb_62us5();
        }
    }
    if ((IP_PIT_0->TIMER[1].TFLG & PIT_TFLG_TIF_MASK) != 0U)
    {
        IP_PIT_0->TIMER[1].TFLG = PIT_TFLG_TIF_MASK;
        s_flags |= ISR_SCHED_FLAG_205US;
        if (s_cfg.cb_205us != NULL)
        {
            s_cfg.cb_205us();
        }
    }
    if ((IP_PIT_0->TIMER[2].TFLG & PIT_TFLG_TIF_MASK) != 0U)
    {
        IP_PIT_0->TIMER[2].TFLG = PIT_TFLG_TIF_MASK;
        s_flags |= ISR_SCHED_FLAG_1MS;
        if (s_cfg.cb_1ms != NULL)
        {
            s_cfg.cb_1ms();
        }
    }
    if ((IP_PIT_0->TIMER[3].TFLG & PIT_TFLG_TIF_MASK) != 0U)
    {
        IP_PIT_0->TIMER[3].TFLG = PIT_TFLG_TIF_MASK;
        s_flags |= ISR_SCHED_FLAG_10MS;
        if (s_cfg.cb_10ms != NULL)
        {
            s_cfg.cb_10ms();
        }
    }
}
