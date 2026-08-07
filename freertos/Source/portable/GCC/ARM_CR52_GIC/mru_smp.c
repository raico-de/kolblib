/*
 * Copyright 2025 NXP.
 */

#include "FreeRTOS.h"

#if ( configNUMBER_OF_CORES > 1 )

#include <stdint.h>
#include "mru_smp.h"
#if defined(_S32DS_FREERTOS_)
#include "gic500.h"
#include "core.h"
#endif

#if defined (CPU_S32K566)

#define MRU_INSTANCE_COUNT 4

uint32_t MRU_BASE_ADDRS[MRU_INSTANCE_COUNT] = {
    0x41018000u, /** R52 core0 MRU0 base address */
    0x41034000u, /** R52 core0 MRU1 base address */
    0x41100000u, /** R52 core1 MRU2 base address */
    0x4111C000u, /** R52 core1 MRU3 base address */
};

#define MRU_SET_CFG(mruId, chId, idx, val) \
    *(volatile uint32_t *)(MRU_BASE_ADDRS[mruId] + (((chId) - 1) * 0x10) + ((idx) * 0x4)) = val

#define MRU_GET_CFG(mruId, chId, idx, val) \
    val = *(volatile uint32_t *)(MRU_BASE_ADDRS[mruId] + (((chId) - 1) * 0x10) + ((idx) * 0x4))

#define MRU_CLR_MBSTAT(mruId, chId, val) \
    *(volatile uint32_t *)(MRU_BASE_ADDRS[mruId] + (((chId) - 1) * 0x10) + 0x8) = val

#define MRU_GET_MBSTAT(mruId, chId, val) \
    val = *(volatile uint32_t *)(MRU_BASE_ADDRS[mruId] + (((chId) - 1) * 0x10) + 0x8)

#define MRU_SET_MAILBOX(mruId, chId, idx, val) \
    *(volatile uint32_t *)(MRU_BASE_ADDRS[mruId] + ((chId) * 0x4000) + ((idx) * 0x4)) = val

#define MRU_GET_MAILBOX(mruId, chId, idx, val) \
    val = *(volatile uint32_t *)(MRU_BASE_ADDRS[mruId] + ((chId) * 0x4000) + ((idx) * 0x4))

#elif defined (CPU_S32N55)

#define MRU_INSTANCE_COUNT 4

uint32_t MRU_BASE_ADDRS[MRU_INSTANCE_COUNT] = {
#if defined(RTU0_R52)
    0x50470000UL,      /** RTU0 clutser0 core0 RTU_MRU0 base address */
    0x50A20000UL,      /** RTU0 clutser0 core1 RTU_MRU2 base address */
    0x505C0000UL,      /** RTU0 cluster1 core0 RTU_MRU1 base address */
    0x50C00000UL,      /** RTU0 cluster1 core1 RTU_MRU3 base address */
#elif defined(RTU1_R52)
    0x51470000UL,      /** RTU1 clutser0 core0 RTU_MRU0 base address */
    0x51A20000UL,      /** RTU1 clutser0 core1 RTU_MRU2 base address */
    0x515C0000UL,      /** RTU1 cluster1 core0 RTU_MRU1 base address */
    0x51C00000UL,      /** RTU1 cluster1 core1 RTU_MRU3 base address */
#elif defined(RTU2_R52)
    0x52470000UL,      /** RTU2 clutser0 core0 RTU_MRU0 base address */
    0x52A20000UL,      /** RTU2 clutser0 core1 RTU_MRU2 base address */
    0x525C0000UL,      /** RTU2 cluster1 core0 RTU_MRU1 base address */
    0x52C00000UL,      /** RTU2 cluster1 core1 RTU_MRU3 base address */
#elif defined(RTU3_R52)
    0x53470000UL,      /** RTU3 clutser0 core0 RTU_MRU0 base address */
    0x53A20000UL,      /** RTU3 clutser0 core1 RTU_MRU2 base address */
    0x535C0000UL,      /** RTU3 cluster1 core0 RTU_MRU1 base address */
    0x53C00000UL,      /** RTU3 cluster1 core1 RTU_MRU3 base address */
#endif
};

#define MRU_SET_CFG(mruId, chId, idx, val) \
    *(volatile uint32_t*)(MRU_BASE_ADDRS[mruId] + (((chId) - 1) << 4) + ((idx) << 2)) = val

#define MRU_GET_CFG(mruId, chId, idx, val) \
    val = *(volatile uint32_t*)(MRU_BASE_ADDRS[mruId] + (((chId) - 1) << 4) + ((idx) << 2))

#define MRU_CLR_MBSTAT(mruId, chId, val) \
    *(volatile uint32_t*)(MRU_BASE_ADDRS[mruId] + (((chId) - 1) << 4) + 8) = val

#define MRU_GET_MBSTAT(mruId, chId, val) \
    val = *(volatile uint32_t*)(MRU_BASE_ADDRS[mruId] + (((chId) - 1) << 4) + 8)

#define MRU_SET_MAILBOX(mruId, chId, idx, val) \
    *(volatile uint32_t*)(MRU_BASE_ADDRS[mruId] + ((chId) << 16) + ((idx) << 2)) = val

#define MRU_GET_MAILBOX(mruId, chId, idx, val) \
    val = *(volatile uint32_t*)(MRU_BASE_ADDRS[mruId] + ((chId) << 16) + ((idx) << 2))

#endif

#define MRU_CFG0_IE_MASK        0x00000004UL
#define MRU_CFG0_CHE_MASK       0x00000001UL
uint32_t getCoreIndex(void) PRIVILEGED_FUNCTION;
uint32_t getCoreIndex(void)
{
    uint32_t reg0, reg1;
    __asm volatile(" mrc p15, 0, %0, c0, c0, 5   \n" : "=r"(reg0) : : ); /* 3.3.78 Multiprocessor Affinity Register */
    __asm volatile(" lsr %0, %1, #7              \n" : "=r"(reg1) : "r"(reg0) : );
    __asm volatile(" add %0, %1, %2              \n" : "=r"(reg0) : "r"(reg0), "r"(reg1) : );
    __asm volatile(" and %0, %1, #0xf            \n" : "=r"(reg0) : "r"(reg0) : );
	return reg0;
}

/**
 * @brief Init MRU
 * @details Fucntion used to enable MRU chanels [1-12], message boxes [0-7] and configuration interrupt type [0-3]
 * for message boxes. Without this MRU module does not work.
 */
void MRUInit(uint32_t mruId, uint32_t ChannelId, uint32_t mbId, uint32_t InterruptType) PRIVILEGED_FUNCTION;
void MRUInit(uint32_t mruId, uint32_t ChannelId, uint32_t mbId, uint32_t InterruptType)
{
    uint32_t val = 0;
#if defined (CPU_S32K566)
    uint32_t coreId = getCoreIndex();
    uint32_t interruptID = MRU_INT_ID(coreId);
#elif defined (CPU_S32N55)
    uint32_t interruptID = MRU_INT_ID(InterruptType);
#endif

    /* Enable channel */
    MRU_GET_CFG(mruId, ChannelId, 0, val);
    val |= MRU_CFG0_CHE_MASK;
    MRU_SET_CFG(mruId, ChannelId, 0, val);

    /* Clear corresponding message boxes */
    MRU_SET_MAILBOX(mruId, ChannelId, mbId, 0);

    /* Clear corresponding mailbox status */
    MRU_GET_MBSTAT(mruId, ChannelId, val);
    val |= (1 << (mbId + 16));
    MRU_CLR_MBSTAT(mruId, ChannelId, val);

    /* Set corresponding message boxes interrupts type */
    MRU_GET_CFG(mruId, ChannelId, 1, val);
    val |= (InterruptType << (mbId * 2));
    MRU_SET_CFG(mruId, ChannelId, 1, val);

    /* Enable corresponding mailboxes and their associated interrupts */
    MRU_GET_CFG(mruId, ChannelId, 0, val);
    val |= ((1 << (mbId + 16)) | (MRU_CFG0_IE_MASK));
    MRU_SET_CFG(mruId, ChannelId, 0, val);
#if defined(_S32DS_FREERTOS_)
    /* Register IRQ interrupt */
    Core_registerIsrHandler(interruptID, MRU_IRQ_Handler);
    gic500_setIntPriority(interruptID, MRU_INT_PRI);
    gic500_enableIntID(interruptID);
#endif
}

/**
* @brief        Write mailbox if available
* @details      This function writes the selected mailbox, using 12th channel
*
* @param[in]    mruId           select the MRU ID [0..3]
* @param[in]    mruCh           select the MRU Channel ID [1..12]
* @param[in]    mbId            select the mailbox [0..3]
* @param[in]    val             value to be written
*
* @return       none
*/
void MRUMB_Set(uint32_t mruId, uint32_t mruCh, uint32_t mbId, uint32_t val) PRIVILEGED_FUNCTION;
void MRUMB_Set(uint32_t mruId, uint32_t mruCh, uint32_t mbId, uint32_t val)
{
    MRU_SET_MAILBOX(mruId, mruCh, mbId, val);
}

typedef void (*pfMailBoxCallBack)(uint32_t, uint32_t);

/**
* @brief        Callback for each MRU interrupt
*/
pfMailBoxCallBack PRIVILEGED_DATA MailBoxArray[4] = {(pfMailBoxCallBack)0, (pfMailBoxCallBack)0, (pfMailBoxCallBack)0, (pfMailBoxCallBack)0,};

/**
* @brief        Handle MRU interrupt and call Hypervisor CallBack if any
* @details      This function handles interrupt of the 12th channel and calls specified CallBack
*
* @return       none
*/
void MRU_IRQ_Handler(void) PRIVILEGED_FUNCTION;
void MRU_IRQ_Handler(void)
{
    uint32_t mruSts = 0;
    uint32_t coreId = getCoreIndex();
    uint32_t mruId = CORE_ID_2_MRU_ID(coreId);
    uint32_t mbId = CFG_MRU_MB_ID_SMP;
    uint32_t interruptType = CFG_MRU_INTERRUPT_SMP;

    MRU_GET_MBSTAT(mruId, CFG_MRU_CHANNEL_SMP, mruSts);
    MRU_CLR_MBSTAT(mruId, CFG_MRU_CHANNEL_SMP, mruSts);
    if ((mruSts & (1 << (mbId + 16))) && MailBoxArray[interruptType])
    {
        uint32_t mbVal;
        MRU_GET_MAILBOX(mruId, CFG_MRU_CHANNEL_SMP, mbId, mbVal);
        MailBoxArray[interruptType](mbVal, coreId);  /* mailbox callback */
    }
}

/**
* @brief        Register a MRU CallBack
* @details      This function register a CallBack on 12th channel
*
* @param[in]    cbIndex         select the MRU interrupt type [0..3]
* @param[in]    cbHandler       callback handler
*
* @return       The function returns 0 success, -1 already registered
*/
int MRURegisterMBCB(uint32_t cbIndex, void* cbHandler) PRIVILEGED_FUNCTION;
int MRURegisterMBCB(uint32_t cbIndex, void* cbHandler)
{
    int retVal = -1;

    if (!MailBoxArray[cbIndex])
    {
        MailBoxArray[cbIndex] = (pfMailBoxCallBack)cbHandler;
        retVal = 0;
    }

    return retVal;
}
#endif /*if ( configNUMBER_OF_CORES > 1 )*/
