/*
 * Copyright 2025 NXP.
 */

#ifndef MRU_SMP_H
#define MRU_SMP_H

#ifdef __cplusplus
extern "C"
{
#endif

/* TODO: add generated S32DS config to select channel MRU, Mbox ID and set SMP value */
#define CFG_MRU_CHANNEL_SMP             6
#define CFG_MRU_INTERRUPT_SMP           2
#define CFG_MRU_MB_ID_SMP               0
#define CFG_MRU_MB_VAL_SMP              0xab

#if defined (CPU_S32K566)

/* Each Cortex-R52 core associated to 2 MRU, identified by IDs 0(first MRU) and 1(second MRU) */
#define CFG_MRU_ASSOCIATED_TO_CORE 1

#define CORE_ID_2_MRU_ID(coreId) (((coreId) * 0x2) + (CFG_MRU_ASSOCIATED_TO_CORE) * 0x1)

/* R52 core 0 MRU 0 interrupt ID */
#define MRU_0_INT_ID(interruptType) (interruptType == 0 ? \
                                267 : (interruptType == 1 ? \
                                        268 : (interruptType == 2 ? 269 : 270)))

/* R52 core 0 MRU 1 interrupt ID */
#define MRU_1_INT_ID(interruptType) (interruptType == 0 ? \
                                272 : (interruptType == 1 ? \
                                        273 : (interruptType == 2 ? 274 : 275)))

/* R52 core 1 MRU 2 interrupt ID */
#define MRU_2_INT_ID(interruptType) (interruptType == 0 ? \
                                277 : (interruptType == 1 ? \
                                        278 : (interruptType == 2 ? 279 : 280)))

/* R52 core 1 MRU 3 interrupt ID */
#define MRU_3_INT_ID(interruptType) (interruptType == 0 ? \
                                282 : (interruptType == 1 ? \
                                        283 : (interruptType == 2 ? 284 : 285)))

#define MRU_INT_ID(coreId) ((coreId == 0) && (CFG_MRU_ASSOCIATED_TO_CORE == 0) ? \
                                MRU_0_INT_ID(CFG_MRU_INTERRUPT_SMP) : \
                                ((coreId == 0) && (CFG_MRU_ASSOCIATED_TO_CORE == 1) ? \
                                MRU_1_INT_ID(CFG_MRU_INTERRUPT_SMP) : \
                                ((coreId == 1) && (CFG_MRU_ASSOCIATED_TO_CORE == 0) ? \
                                MRU_2_INT_ID(CFG_MRU_INTERRUPT_SMP) : \
                                MRU_3_INT_ID(CFG_MRU_INTERRUPT_SMP))))

#elif defined (CPU_S32N55)

#define CORE_ID_2_MRU_ID(coreId) (coreId)

#define MRU_INT_ID(interruptType) (interruptType == 0 ? \
                                17 : (interruptType == 1 ? \
                                        18 : (interruptType == 2 ? 19 : 20)))

#endif

#define MRU_INT_PRI         7

uint32_t getCoreIndex(void);
void MRUInit(uint32_t mruId, uint32_t ChannelId, uint32_t mbId, uint32_t InterruptType);
int MRURegisterMBCB(uint32_t cbIndex, void* cbHandler);
void MRUMB_Set(uint32_t mruId, uint32_t mruCh, uint32_t mbId, uint32_t val);
void MRU_IRQ_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /** MRU_SMP_H */
