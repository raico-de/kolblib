/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "FreeRTOS.h"
#include "task.h"

#if (configENABLE_MPU == 1)
/* MPU includes. */
#include "mpu_wrappers.h"
#include "mpu_syscall_numbers.h"
#endif

#include <string.h>
#if ( configNUMBER_OF_CORES > 1 )
#include "mru_smp.h"
#endif
#if ( configENABLE_FPU == 1 )

    /* Values that can be used to allocate the stack for the FPU . */
    #define portSIZE_OF_ONE_STACK_ELEMENT                   (sizeof(StackType_t))
    #define portFPU_STACK_DEEP_SIZE                         ( ( uint16_t ) 68 ) /* 8 bytes aligned, D0-D31, FPSCR, FPEXC */
    #define portTASK_STACK_DEEP_SIZE                        ( ( uint8_t ) 18 )
    #define portSIZE_OF_STACK_FPU                           (portFPU_STACK_DEEP_SIZE * portSIZE_OF_ONE_STACK_ELEMENT)
    #define portSIZE_OF_TASK_STACK_USE_FPU                  ((portFPU_STACK_DEEP_SIZE + portTASK_STACK_DEEP_SIZE) * portSIZE_OF_ONE_STACK_ELEMENT)

    /* This variable points the TCB handler of the task
     * which owns the FPU, use to save FPU. */
    volatile void *volatile prevFpuCtx = NULL;

    /* This variable points the TCB handler of the previous
     * task which owns the FPU, use to load FPU. */
    volatile void *volatile newFpuCtx = NULL;

    /* This boolean variable indicates switching by irq function hay yield function?
     * b_SwitchByIRQ = 0: switch by yeild function.
     * b_SwitchByIRQ = 1: switch by IRQ function*/
    volatile UBaseType_t b_SwitchByIRQ = 0;

    /* This variable is the condition to allocate
     * the stack for the FPU. */
    UBaseType_t task_use_fpu = 1;

    /* This variable is the FPU usage interrupt number
     * in the nested interrupt. */
    UBaseType_t ulPortInterruptHasFPUContext = 0;

    /* This is a pointer where the illegal_handler save the
     * new fpu context and update the global_irq_fpu_stack */
    StackType_t new_global_irq_fpu_stack = 0;
    StackType_t global_irq_fpu_stack = 0;

#endif /* configENABLE_FPU */


#if ( configNUMBER_OF_CORES > 1 )
UBaseType_t PRIVILEGED_DATA uxCriticalNesting[ configNUMBER_OF_CORES ];
UBaseType_t PRIVILEGED_DATA uxInterruptNested[ configNUMBER_OF_CORES ];
extern TaskHandle_t pxCurrentTCB[ configNUMBER_OF_CORES ];
#else
UBaseType_t PRIVILEGED_DATA uxCriticalNesting = 0;
UBaseType_t PRIVILEGED_DATA uxInterruptNested = 0;
extern TaskHandle_t pxCurrentTCB;
#endif

extern void vUpdateTimer(void);

void vPortYield( void ) PRIVILEGED_FUNCTION;

#if ( ( configENABLE_MPU == 1 ) && ( configUSE_MPU_WRAPPERS_V1 == 0 ) )
int vSystemCallEnter(uint32_t lr) PRIVILEGED_FUNCTION;
void vSystemCallExit(void)__attribute__ (( naked )) FREERTOS_SYSTEM_CALL;
#if ( configNUMBER_OF_CORES > 1 )
int vSystemCallEnter(uint32_t lr)
{
    extern TaskHandle_t pxCurrentTCBs[configNUMBER_OF_CORES];
    int retVal = -1;
    xMPU_SETTINGS * pxMpuSettings = xTaskGetMPUSettings( pxCurrentTCBs[getCoreIndex()] );
    
    if (!pxMpuSettings->xSystemCallStackInfo.ulFlags)
    {
        pxMpuSettings->xSystemCallStackInfo.ulFlags = 1;
        pxMpuSettings->xSystemCallStackInfo.ulLr = lr;
        retVal = 0;
    }
    return retVal;
}

void vSystemCallExit(void)
{
    OSASM (" push {r0, r1, r2, r3}  \n"); /* store retvalue from syscall */

    OSASM(" mov r0, %0\t\n": : "i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY ): );
    OSASM(" mcr p15, 0, r0, c4, c6, 0                       \t\n"); /* Write r0 into ICC_PMR, disable OS interrupts */
    OSASM(" dsb                                             \t\n"); /* OS interrupt disabled */

    OSASM(" mrc p15, 0, r1, c0, c0, 5 \n"); /* 3.3.78 Multiprocessor Affinity Register */
    OSASM(" lsr r2, r1, #7          \n");
    OSASM(" add r1, r1, r2          \n");
    OSASM(" and r1, r1, #0xf        \n"); /* r1 <-- core ID */
    OSASM (" ldr r0, =pxCurrentTCBs \n");
    OSASM(" lsl r1, r1, #2          \n");
    OSASM(" add r0, r0, r1          \n");
    OSASM (" ldr r0,[r0]            \n"); /* Get current TCB ofset CoreID */
    OSASM (" bl xTaskGetMPUSettings \n");
    OSASM (" ldr r1, [r0, %0]       \n" : : "i"((sizeof(MPURegionSettings_t) * portTOTAL_NUM_REGIONS) + sizeof(uint32_t)) : );
    OSASM (" cmp r1, #0             \n");
    OSASM (" beq 1f                 \n");
    OSASM (" eor r1, r1             \n");
    OSASM (" str r1, [r0, %0]       \n" : : "i"((sizeof(MPURegionSettings_t) * portTOTAL_NUM_REGIONS) + sizeof(uint32_t)) : );
    OSASM (" ldr lr, [r0, %0]       \n" : : "i"((sizeof(MPURegionSettings_t) * portTOTAL_NUM_REGIONS) + (sizeof(uint32_t) * 2)) : );

    OSASM(" mov r0, %0\t\n": : "i" ( portINTC_GIC_CONVERT_PRIO_SET( portINTC_GIC_MIN_PRIO ) ): );
    OSASM(" mcr p15, 0, r0, c4, c6, 0                       \t\n"); /* Write r0 into ICC_PMR, enable OS interrupts */
    OSASM(" dsb                                             \t\n"); /* OS interrupt enabled */

    OSASM (" pop {r0, r1, r2, r3}   \n"); /* restore retvalue from syscall */
    OSASM ("cps #16                 \n");
    OSASM ("bx lr                   \n");
    OSASM (" 1:\n");
    OSASM (" b .                    \n");
}
#else
int vSystemCallEnter(uint32_t lr)
{
    extern TaskHandle_t pxCurrentTCB;
    int retVal = -1;
    xMPU_SETTINGS * pxMpuSettings = xTaskGetMPUSettings( pxCurrentTCB );
    
    if (!pxMpuSettings->xSystemCallStackInfo.ulFlags)
    {
        pxMpuSettings->xSystemCallStackInfo.ulFlags = 1;
        pxMpuSettings->xSystemCallStackInfo.ulLr = lr;
        retVal = 0;
    }
    return retVal;
}

void vSystemCallExit(void)
{
    OSASM (" push {r0, r1}          \n"); /* store retvalue from syscall */
    OSASM (" ldr r0, =pxCurrentTCB  \n");
    OSASM (" ldr r0,[r0]            \n");
    OSASM (" bl xTaskGetMPUSettings \n");
    OSASM (" ldr r1, [r0, %0]       \n" : : "i"((sizeof(MPURegionSettings_t) * portTOTAL_NUM_REGIONS) + sizeof(uint32_t)) : );
    OSASM (" cmp r1, #0             \n");
    OSASM (" beq 1f                 \n");
    OSASM (" eor r1, r1             \n");
    OSASM (" str r1, [r0, %0]       \n" : : "i"((sizeof(MPURegionSettings_t) * portTOTAL_NUM_REGIONS) + sizeof(uint32_t)) : );

    OSASM (" ldr lr, [r0, %0]       \n" : : "i"((sizeof(MPURegionSettings_t) * portTOTAL_NUM_REGIONS) + (sizeof(uint32_t) * 2)) : );
    OSASM (" pop {r0, r1}           \n"); /* restore retvalue from syscall */
    OSASM ("cps #16                 \n");
    OSASM ("bx lr                   \n");
    OSASM (" 1:\n");
    OSASM (" b .                    \n");
}
#endif
#endif

#if ( configNUMBER_OF_CORES > 1 )
extern void MRU_IRQ_Handler(void);
extern void vInitInterruptTable(int, int, void*);
extern void vGicEnableInterrupt(int index, int prio);

void vPortYield(void)
{
    /** do not save R0, R1, R2, R3, R12 */
    OSASM(" mov r2, %0\t\n": : "i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY ): );
    OSASM(" mrc p15, 0, r3, c4, c6, 0                       \t\n"); /* get the ICC_PMR in r3 */
    OSASM(" mcr p15, 0, r2, c4, c6, 0                       \t\n"); /* Write r2 into ICC_PMR, disable OS interrupts */
    OSASM(" dsb                                             \t\n"); /* OS interrupt disabled */
    OSASM(" mrc p15, 0, r0, c0, c0, 5                       \t\n"); /* 3.3.78 Multiprocessor Affinity Register */
    OSASM(" lsr r1, r0, #7                                  \t\n");
    OSASM(" add r0, r0, r1                                  \t\n");
    OSASM(" and r0, r0, #0xf                                \t\n"); /* r0 <-- core ID */

    /* enter to svc mode */
    OSASM(" cps #0x13                                       \t\n"); /* enter to svc mode, R14 becomes available */
    OSASM(" ldr r1, =pxCurrentTCBs                          \t\n"); /* Get current TCB */
    OSASM(" lsl r2, r0, #2                                  \t\n");
    OSASM(" add r0, r1, r2                                  \t\n"); /* Get current TCB ofset CoreID */
    OSASM(" ldr r1, [r0]                                    \t\n");
    /* solicited frame
        00 <- cpsr / type == 0
        04 <- critical nesting
        08 <- ICC_PMR
        0c <- r4
        10 <- r5
        14 <- r6
        18 <- r7
        1c <- r8
        20 <- r9
        24 <- r10
        28 <- r11
        2c <- lr
    */
    OSASM(" mrs r0, SP_usr                                  \t\n"); /* r0 == task stack pointer */
    OSASM(" sub r12, r0, #48                                \t\n"); /* reserve space */
    OSASM(" str r12, [r1]                                   \t\n"); /* save task top of stack */
    OSASM(" mrs r12, LR_usr                                 \t\n"); /* r12 == user link register */
    OSASM(" stmfd r0!, {r3-r12}                             \t\n"); /* interrupt priority mask and callee-save registers using task stack */
    OSASM(" ldr r12, =uxCriticalNesting                     \t\n");

    OSASM(" add r12, r2                                     \t\n"); /* indexed uxCriticalNesting */
    OSASM(" mov r1, #0                                      \t\n"); /* solicited = 0, to fix arm/thumb interwork */
    OSASM(" ldr r3, [r12]                                   \t\n");
    OSASM(" stmfd r0!, {r1,r3}                              \t\n"); /* solicited (0, CPSR) context switched and uxCriticalNesting*/

#if configENABLE_FPU == 1
    OSASM(" mov r0, #0                                      \t\n"); /* first parameter */
    OSASM(" mov r1, #0                                      \t\n"); /* Second parameter */
    OSASM(" bl pvTaskGetThreadLocalStoragePointer           \t\n"); /* Call function */
    OSASM(" ldr r1, =prevFpuCtx                             \t\n");
    OSASM(" ldr r1, [r1]                                    \t\n"); /* Get prevFpuCtx's value */
    OSASM(" cmp r1, r0                                      \t\n"); /* Compare newFpuCtx with prevFpuCtx */
    OSASM(" bne 2f                                          \t\n");
    /* newFpuCtx == prevFpuCtx : solicited for switched out task,
     * buffer to save registers status, d8-d15 only */
    OSASM(" ldr r0, =b_SwitchByIRQ                          \t\n");
    OSASM(" ldr r1, [r0]                                    \t\n"); /* get boolean check switch by IRQ or Yield: b_SwitchByIRQ */
    OSASM(" mov r1, #0                                      \t\n");
    OSASM(" str r1, [r0]                                    \t\n"); /* b_SwitchByIRQ = 0 */
    OSASM(" 2:                                              \t\n");
    OSASM(" fmrx r0, FPEXC                                  \t\n"); /* Disable FPU coprocessor - for switched in task */
    OSASM(" and r0, r0, #~(1 << 30)                         \t\n");
    OSASM(" fmxr FPEXC, r0                                  \t\n");
#endif /* configENABLE_FPU */
    OSASM(" lsr r0, r2, #2                                  \t\n"); /* core id */
    OSASM(" bl vTaskSwitchContext                           \t\n");
    OSASM(" mrc p15, 0, r3, c4, c6, 0                       \t\n"); /* get the ICC_PMR in r3 */
    OSASM(" mov r2, %0 \t\n" : :"i"(configMAX_SYSCALL_INTERRUPT_PRIORITY) : );

    OSASM(" mrc p15, 0, r0, c0, c0, 5                       \t\n"); /* 3.3.78 Multiprocessor Affinity Register */
    OSASM(" lsr r1, r0, #7                                  \t\n");
    OSASM(" add r0, r0, r1                                  \t\n");
    OSASM(" and r0, r0, #0xf                                \t\n"); /* r0 <-- core ID */
    OSASM(" lsl r11, r0, #2                                 \t\n");

    OSASM(" ldr r0, =pxCurrentTCBs                          \t\n"); /* Get current TCB */
    OSASM(" add r0, r0, r11                                 \t\n"); /* Get current TCB ofset CoreID */
    OSASM(" ldr r0, [r0]                                    \t\n");

#if (configENABLE_MPU == 1)
    OSASM(" ldr r1, [r0, %0]                                \t\n" : : "i"((sizeof(MPURegionSettings_t) << 2) + 4) : ); /* R1 xMPU_SETTINGS->ulTaskFlags */
    OSASM(" mov r2, #4                                      \t\n");
    OSASM(" mov r3, %0                                      \t\n" : : "i"(portTOTAL_NUM_REGIONS) : );
    OSASM("3:");
    OSASM(" ldr r1, [r0, r2]                                \t\n");
    OSASM(" cmp r1, #0                                      \t\n");
    OSASM(" beq 2f                                          \t\n");
    OSASM(" mcr p15, 0, r1, c6, c2, 1                       \t\n"); /* select the descriptor */
    OSASM(" mov r5, #0                                      \t\n");
    OSASM(" mcr p15, 0, r5, c6, c3, 1                       \t\n"); /* write the limit, this disable the descriptor */
    OSASM(" dsb                                             \t\n");
    OSASM(" isb                                             \t\n");
    OSASM(" add r12, r2, #4                                 \t\n");
    OSASM(" ldr r4, [r12, r0]                               \t\n");
    OSASM(" add r12, r2, #8                                 \t\n");
    OSASM(" ldr r5, [r12, r0]                               \t\n");
    OSASM(" b 4f                                            \t\n");
    OSASM("2:");
    OSASM(" mov r5, r1                                      \t\n");
    OSASM(" mov r4, r1                                      \t\n");
    OSASM(" add r1, r1, %0                                  \t\n" : : "i"(portTOTAL_NUM_REGIONS + portMPU_TASKSTACK_DSC_IDX) : );
    OSASM(" sub r1, r1, r3                                  \t\n");
    OSASM("4:");
    OSASM(" mcr p15, 0, r1, c6, c2, 1                       \t\n"); /* select the descriptor */
    OSASM(" mcr p15, 0, r4, c6, c3, 0                       \t\n"); /* write the base */
    OSASM(" mcr p15, 0, r5, c6, c3, 1                       \t\n"); /* write the limit, this enable the descriptor */
    OSASM(" dsb                                             \t\n");
    OSASM(" isb                                             \t\n");

    OSASM(" add r2, r2, %0                                  \t\n" : : "i"(sizeof(MPURegionSettings_t)): );
    OSASM(" subs r3, r3, #1                                 \t\n");
    OSASM(" bne 3b                                          \t\n");

#endif

    OSASM(" ldr r12, [r0]                                   \t\n"); /* r12, SP_usr */
    OSASM(" ldmfd r12!, {r1-r10}                            \t\n"); /* r1 - sol = 0/unsol = CPSR, r2 critical, r3 int mask */
    OSASM(" ldr r0, =uxCriticalNesting                      \t\n");
    OSASM(" add r0, r11                                     \t\n");
    OSASM(" str r2, [r0]                                    \t\n"); /* restore critical nesting */
    OSASM(" mov r0, r12                                     \t\n"); /* R0 == new SP_usr */
    OSASM(" ldmfd r0!, {r11, r12}                           \t\n"); /* restore R11, R12 == LR_usr */
    OSASM(" msr LR_usr, r12                                 \t\n"); /* update user LR */
    OSASM(" cmp r1, #0                                      \t\n");
    OSASM(" beq 1f                                          \t\n"); /* go to solicited, 1: */
    /* unsolicited */
    OSASM(" cpsid i                                         \t\n"); /* disable interrupts */
    OSASM(" mcr p15, 0, r3, c4, c6, 0                       \t\n"); /* restore r3 into ICC_PMR, task mask level */
    OSASM(" msr spsr_irq, r1                                \t\n"); /* unsolicited (MOV R1 to SPSR), OK not IRQ mode */
    OSASM(" cps #0x12                                       \t\n"); /* enter in IRQ mode */
    OSASM(" add r1, r0, #24                                 \t\n"); /* prepare user stack unload */
    OSASM(" msr SP_usr, r1                                  \t\n"); /* unsolicited (MOV R1 to USR SP) */
    OSASM(" ldm r0, {r0-r3, r12, r14}                       \t\n"); /* get registers from user stack, additional unsolicited frame */
    OSASM(" subs pc, lr, #4                                 \t\n"); /* return from interrupt */
    OSASM(" 1:                                              \t\n"); /* return solicited */
    OSASM(" msr SP_usr, r0                                  \t\n"); /* update user SP */
    OSASM(" cps #0x1f                                       \t\n"); /* enter in System mode */
    OSASM(" mcr p15, 0, r3, c4, c6, 0                       \t\n"); /* restore r3 into ICC_PMR, task mask level */
    OSASM(" mov pc, lr                                      \t\n"); /* return solicited */
}
#else /* configNUMBER_OF_CORES > 1 */
void vPortYield(void)
{
    /** do not save R0, R1, R2, R3, R12 */
    OSASM(" mrc p15, 0, r3, c4, c6, 0                       \t\n"); /* get the ICC_PMR */
    OSASM(" mov r2, %0\t\n": : "i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY ): );
    OSASM(" mcr p15, 0, r2, c4, c6, 0                       \t\n"); /* Write r2 into ICC_PMR, disable OS interrupts */

    OSASM(" ldr r0, =pxCurrentTCB                           \t\n"); /* Get current TCB */
    OSASM(" ldr r1, [r0]                                    \t\n");

    OSASM(" stmfd sp!, {r3-r11, lr}                         \t\n"); /* interrupt priority mask and callee-save registers using task stack */
    OSASM(" mov r2, #0                                      \t\n"); /* solicited = 0 */
    OSASM(" ldr r0, =uxCriticalNesting                      \t\n");
    OSASM(" ldr r3, [r0]                                    \t\n");
    OSASM(" stmfd sp!, {r2,r3}                              \t\n"); /* solicited (0, CPSR) context switched and uxCriticalNesting*/

    OSASM(" str sp, [r1]                                    \t\n"); /* save task top of stack */

#if configENABLE_FPU == 1
    OSASM(" mov r0, #0                                      \t\n"); /* first parameter */
    OSASM(" mov r1, #0                                      \t\n"); /* Second parameter */
    OSASM(" bl pvTaskGetThreadLocalStoragePointer           \t\n"); /* Call function */
    OSASM(" ldr r1, =prevFpuCtx                             \t\n");
    OSASM(" ldr r1, [r1]                                    \t\n"); /* Get prevFpuCtx's value */
    OSASM(" cmp r1, r0                                      \t\n"); /* Compare newFpuCtx with prevFpuCtx */
    OSASM(" bne 2f                                          \t\n");
    /* newFpuCtx == prevFpuCtx : solicited for switched out task,
     * buffer to save registers status, d8-d15 only */
    OSASM(" ldr r0, =b_SwitchByIRQ                          \t\n");
    OSASM(" ldr r1, [r0]                                    \t\n"); /* get boolean check switch by IRQ or Yield: b_SwitchByIRQ */
    OSASM(" mov r1, #0                                      \t\n");
    OSASM(" str r1, [r0]                                    \t\n"); /* b_SwitchByIRQ = 0 */
    OSASM(" 2:                                              \t\n");
    OSASM(" fmrx r0, FPEXC                                  \t\n"); /* Disable FPU coprocessor - for switched in task */
    OSASM(" and r0, r0, #~(1 << 30)                         \t\n");
    OSASM(" fmxr FPEXC, r0                                  \t\n");
#endif /* configENABLE_FPU */
    OSASM(" bl vTaskSwitchContext                           \t\n");
    OSASM(" ldr r0, =pxCurrentTCB                           \t\n");
    OSASM(" ldr r0, [r0]                                    \t\n");
#if (configENABLE_MPU == 1)
    OSASM(" ldr r1, [r0, %0]                                \t\n" : : "i"((sizeof(MPURegionSettings_t) << 2) + 4) : ); /* R1 xMPU_SETTINGS->ulTaskFlags */
    OSASM(" mov r2, #4                                      \t\n");
    OSASM(" mov r3, %0                                      \t\n" : : "i"(portTOTAL_NUM_REGIONS) : );
    OSASM("3:");
    OSASM(" ldr r1, [r0, r2]                                \t\n");
    OSASM(" cmp r1, #0                                      \t\n");
    OSASM(" beq 2f                                          \t\n");
    OSASM(" mcr p15, 0, r1, c6, c2, 1                       \t\n"); /* select the descriptor */
    OSASM(" mov r5, #0                                      \t\n");
    OSASM(" mcr p15, 0, r5, c6, c3, 1                       \t\n"); /* write the limit, this disable the descriptor */
    OSASM(" dsb                                             \t\n");
    OSASM(" isb                                             \t\n");
    OSASM(" add r12, r2, #4                                 \t\n");
    OSASM(" ldr r4, [r12, r0]                               \t\n");
    OSASM(" add r12, r2, #8                                 \t\n");
    OSASM(" ldr r5, [r12, r0]                               \t\n");
    OSASM(" b 4f                                            \t\n");
    OSASM("2:");
    OSASM(" mov r5, r1                                      \t\n");
    OSASM(" mov r4, r1                                      \t\n");
    OSASM(" add r1, r1, %0                                  \t\n" : : "i"(portTOTAL_NUM_REGIONS + portMPU_TASKSTACK_DSC_IDX) : );
    OSASM(" sub r1, r1, r3                                  \t\n");
    OSASM("4:");
    OSASM(" mcr p15, 0, r1, c6, c2, 1                       \t\n"); /* select the descriptor */
    OSASM(" mcr p15, 0, r4, c6, c3, 0                       \t\n"); /* write the base */
    OSASM(" mcr p15, 0, r5, c6, c3, 1                       \t\n"); /* write the limit, this enable the descriptor */
    OSASM(" dsb                                             \t\n");
    OSASM(" isb                                             \t\n");

    OSASM(" add r2, r2, %0                                  \t\n" : : "i"(sizeof(MPURegionSettings_t)): );
    OSASM(" subs r3, r3, #1                                 \t\n");
    OSASM(" bne 3b                                          \t\n");

#endif
    OSASM(" ldr sp, [r0]                                    \t\n"); /* get new stack of the task */
    OSASM(" ldmfd sp!, {r1-r11,lr}                          \t\n"); /* r1 - sol = 0/unsol = CPSR, r2 critical, r3 int mask */
    OSASM(" ldr r12, =uxCriticalNesting                     \t\n");
    OSASM(" mcr p15, 0, r3, c4, c6, 0                       \t\n"); /* restore r3 into ICC_PMR, task mask level */
    OSASM(" str r2, [r12]                                   \t\n"); /* restore critical nesting */
    OSASM(" cmp r1, #0                                      \t\n");
    OSASM(" beq 1f                                          \t\n"); /* go to solicited, 1: */
    OSASM(" cpsid i                                         \t\n"); /* disable interrupts */
    OSASM(" mov r0, sp                                      \t\n"); /* prepare exit from interrupt, r0 contains user SP */
    OSASM(" add sp, sp, #24                                 \t\n"); /* user stack unload r0 - r3, r12, r14 */
    OSASM(" msr spsr_irq, r1                                \t\n"); /* unsolicited (MOV R1 to SPSR), OK not IRQ mode */
    OSASM(" cps #0x12                                       \t\n"); /* enter in IRQ mode */
    OSASM(" ldm r0, {r0-r3, r12, r14}                       \t\n"); /* get registers from user stack */
    OSASM(" subs pc, lr, #4                                 \t\n"); /* return from interrupt */
    OSASM(" 1:                                              \t\n");
    OSASM(" mov pc, lr                                      \t\n"); /* return unsolicited */
}
#endif
void vPortStartFirstTask(void) __attribute__ (( naked )) PRIVILEGED_FUNCTION;
#if ( configNUMBER_OF_CORES > 1 )
void vPortStartFirstTask(void)
{
    /** do not save R0, R1, R2, R3, R12 */
    OSASM(" cpsid ia                                        \t\n"); /* disable interrupts */
    OSASM(" bl getCoreIndex                                 \t\n"); /* r0 <-- core ID */
    OSASM(" ldr r1, =pxCurrentTCBs                          \t\n"); /* Get current TCB */
    OSASM(" lsl r2, r0, #2                                  \t\n");
    OSASM(" add r0, r1, r2                                  \t\n"); /* Get current TCB ofset CoreID */
    OSASM(" ldr r0, [r0]                                    \t\n");
#if (configENABLE_MPU == 1)
    OSASM(" ldr r1, [r0, %0]                                \t\n" : : "i"((sizeof(MPURegionSettings_t) << 2) + 4) : ); /* R1 xMPU_SETTINGS->ulTaskFlags */
    OSASM(" ands r1, r1, %0                                 \t\n" : : "i"(portTASK_IS_PRIVILEGED_NORESTRICTED): );
    OSASM(" bne 1f                                          \t\n");
    OSASM(" mov r2, #4                                      \t\n");
    OSASM(" mov r3, %0                                      \t\n" : : "i"(portTOTAL_NUM_REGIONS) : );
    OSASM("3:");
    OSASM(" ldr r1, [r0, r2]                                \t\n");
    OSASM(" cmp r1, #0                                      \t\n");
    OSASM(" beq 2f                                          \t\n");

    OSASM("add r12, r2, #4                                  \t\n");
    OSASM("ldr r4, [r12, r0]                                \t\n");
    OSASM("add r12, r2, #8                                  \t\n");
    OSASM("ldr r5, [r12, r0]                                \t\n");
    OSASM(" mcr p15, 0, r1, c6, c2, 1                       \t\n"); /* select the descriptor */
    OSASM(" mcr p15, 0, r4, c6, c3, 0                       \t\n"); /* write the base */
    OSASM(" mcr p15, 0, r5, c6, c3, 1                       \t\n"); /* write the limit, this enable the descriptor */
    OSASM(" dsb                                             \t\n");
    OSASM(" isb                                             \t\n");
    OSASM("2:");
    OSASM("add r2, r2, %0                                   \t\n" : : "i"(sizeof(MPURegionSettings_t)): );
    OSASM("subs r3, r3, #1                                  \t\n");
    OSASM(" bne 3b                                          \t\n");
    OSASM("1:");
#endif
    OSASM(" ldr sp, [r0]                                    \t\n"); /* get new stack of the task */
    OSASM(" mov r0, r2                                      \t\n"); /* r0 <-- core ID << 2 */
#if configENABLE_FPU == 1
    OSASM(" fmrx r2, FPEXC                                  \t\n"); /* Disable FPU coprocessor for the task */
    OSASM(" and r2, r2, #~(1 << 30)                         \t\n");
    OSASM(" fmxr FPEXC, r2                                  \t\n");
#endif /* configENABLE_FPU */
    OSASM(" ldmfd sp!, {r1-r11,lr}                          \t\n"); /* r1 - sol(0)/unsol(CPSR), r2 critical, r3 int mask*/
    OSASM(" ldr r12, =uxCriticalNesting                     \t\n");
    OSASM(" mcr p15, 0, r3, c4, c6, 0                       \t\n"); /* Write r3 into ICC_PMR, task mask level */
    OSASM(" add r12, r12, r0                                \t\n"); /* indexed by core ID */
    OSASM(" str r2, [r12]                                   \t\n"); /* save critical */
    OSASM(" mov r0, sp                                      \t\n"); /* r0 contains user SP */
    OSASM(" add sp, sp, #24                                 \t\n"); /* unload user stack */
    OSASM(" msr spsr_irq, r1                               \t\n"); /* unsolicited (MOV R1 to spsr) , OK not IRQ mode */
    OSASM(" cps #0x12                                       \t\n"); /* enter to irq mode */
    OSASM(" ldm r0, {r0-r3, r12, r14}                       \t\n");
    OSASM(" subs pc, lr, #4                                 \t\n"); /* return from interrupt */
}
#else /* configNUMBER_OF_CORES > 1 */
void vPortStartFirstTask(void)
{
    OSASM(" cpsid ia                                        \t\n"); /* disable interrupts */
    OSASM(" ldr r0, =pxCurrentTCB                           \t\n");
    OSASM(" ldr r0, [r0]                                    \t\n"); /* R0 crt TCB */
#if (configENABLE_MPU == 1)
    OSASM(" ldr r1, [r0, %0]                                \t\n" : : "i"((sizeof(MPURegionSettings_t) << 2) + 4) : ); /* R1 xMPU_SETTINGS->ulTaskFlags */
    OSASM(" ands r1, r1, %0                                 \t\n" : : "i"(portTASK_IS_PRIVILEGED_NORESTRICTED): );
    OSASM(" bne 1f                                          \t\n");
    OSASM(" mov r2, #4                                      \t\n");
    OSASM(" mov r3, %0                                      \t\n" : : "i"(portTOTAL_NUM_REGIONS) : );
    OSASM("3:");
    OSASM(" ldr r1, [r0, r2]                                \t\n");
    OSASM(" cmp r1, #0                                      \t\n");
    OSASM(" beq 2f                                          \t\n");

    OSASM("add r12, r2, #4                                  \t\n");
    OSASM("ldr r4, [r12, r0]                                \t\n");
    OSASM("add r12, r2, #8                                  \t\n");
    OSASM("ldr r5, [r12, r0]                                \t\n");
    OSASM(" mcr p15, 0, r1, c6, c2, 1                       \t\n"); /* select the descriptor */
    OSASM(" mcr p15, 0, r4, c6, c3, 0                       \t\n"); /* write the base */
    OSASM(" mcr p15, 0, r5, c6, c3, 1                       \t\n"); /* write the limit, this enable the descriptor */
    OSASM(" dsb                                             \t\n");
    OSASM(" isb                                             \t\n");
    OSASM("2:");
    OSASM("add r2, r2, %0                                   \t\n" : : "i"(sizeof(MPURegionSettings_t)): );
    OSASM("subs r3, r3, #1                                  \t\n");
    OSASM(" bne 3b                                          \t\n");
    OSASM("1:");
#endif
    OSASM(" ldr sp, [r0]                                    \t\n"); /* get new stack of the task */
#if configENABLE_FPU == 1
    OSASM(" fmrx r2, FPEXC                                  \t\n"); /* Disable FPU coprocessor for the task */
    OSASM(" and r2, r2, #~(1 << 30)                         \t\n");
    OSASM(" fmxr FPEXC, r2                                  \t\n");
#endif /* configENABLE_FPU */
    OSASM(" ldmfd sp!, {r1-r11,lr}                          \t\n"); /* r1 - sol(0)/unsol(CPSR), r2 critical, r3 int mask*/
    OSASM(" ldr r12, =uxCriticalNesting                     \t\n");
    OSASM(" mcr p15, 0, r3, c4, c6, 0                       \t\n"); /* Write r3 into ICC_PMR, task mask level */
    OSASM(" str r2, [r12]                                   \t\n"); /* save critical */
    OSASM(" mov r0, sp                                      \t\n"); /* r0 contains user SP */
    OSASM(" add sp, sp, #24                                 \t\n"); /* unload user stack */
    OSASM(" msr spsr_irq, r1                                \t\n"); /* unsolicited (MOV R1 to spsr), OK not IRQ mode*/
    OSASM(" cps #0x12                                       \t\n"); /* enter to irq mode */
    OSASM(" ldm r0, {r0-r3, r12, r14}                       \t\n");
    OSASM(" subs pc, lr, #4                                 \t\n"); /* return from interrupt */
}
#endif
/*
 * Initialise the stack
 */
uint32_t PRIVILEGED_DATA task_debug_pattern = 0;
#if ( configENABLE_MPU == 1 )
#if ( configUSE_MPU_WRAPPERS_V1 == 0 )

/* Max value that fits in a uint32_t type. */
#define portUINT32_MAX    ( ~( ( uint32_t ) 0 ) )
/* Check if adding a and b will result in overflow. */
#define portADD_UINT32_WILL_OVERFLOW( a, b )    ( ( a ) > ( portUINT32_MAX - ( b ) ) )
#define portEXTRACT_ADDRESS_FROM_BAR( rbar ) \
    ( ( rbar ) & portMPU_ADDRESS_BITMASK )
/* Extract size of the MPU region as encoded in the
 * RASR (Region Attribute and Size Register) value. */
#define portEXTRACT_ADDRESS_FROM_LAR( rlar ) \
    ( ((rlar) & portMPU_ADDRESS_BITMASK) + 0x3f )
/* Does addr lies within [start, end] address range? */
#define portIS_ADDRESS_WITHIN_RANGE( addr, start, end ) \
    ( ( ( addr ) >= ( start ) ) && ( ( addr ) <= ( end ) ) )

#define portMPU_BAR_AP_BITMASK                      ( 0x3UL << 1UL )
#define portMPU_RASR_SIZE_BITMASK                   ( 0x1FUL << 1UL )
/* MPU Region Base Address Register (RBAR) bitmasks. */
#define portMPU_ADDRESS_BITMASK                ( 0xFFFFFFC0UL )

/**
 * @brief This variable is set to pdTRUE when the scheduler is started.
 */
PRIVILEGED_DATA static BaseType_t xSchedulerRunning = pdFALSE;

#if ( configENABLE_ACCESS_CONTROL_LIST == 1 )

    BaseType_t xPortIsAuthorizedToAccessKernelObject( int32_t lInternalIndexOfKernelObject ) /* PRIVILEGED_FUNCTION */
    {
        uint32_t ulAccessControlListEntryIndex, ulAccessControlListEntryBit;
        BaseType_t xAccessGranted = pdFALSE;
        const xMPU_SETTINGS * xTaskMpuSettings;

        if( xSchedulerRunning == pdFALSE )
        {
            /* Grant access to all the kernel objects before the scheduler
             * is started. It is necessary because there is no task running
             * yet and therefore, we cannot use the permissions of any
             * task. */
            xAccessGranted = pdTRUE;
        }
        else
        {
            xTaskMpuSettings = xTaskGetMPUSettings( NULL ); /* Calling task's MPU settings. */

            ulAccessControlListEntryIndex = ( ( uint32_t ) lInternalIndexOfKernelObject / portACL_ENTRY_SIZE_BITS );
            ulAccessControlListEntryBit = ( ( uint32_t ) lInternalIndexOfKernelObject % portACL_ENTRY_SIZE_BITS );

            if( ( xTaskMpuSettings->ulTaskFlags & portTASK_IS_PRIVILEGED_FLAG ) == portTASK_IS_PRIVILEGED_FLAG )
            {
                xAccessGranted = pdTRUE;
            }
            else
            {
                if( ( xTaskMpuSettings->ulAccessControlList[ ulAccessControlListEntryIndex ] & ( 1U << ulAccessControlListEntryBit ) ) != 0 )
                {
                    xAccessGranted = pdTRUE;
                }
            }
        }

        return xAccessGranted;
    }

#else /* #if ( configENABLE_ACCESS_CONTROL_LIST == 1 ) */

    BaseType_t xPortIsAuthorizedToAccessKernelObject( int32_t lInternalIndexOfKernelObject ) /* PRIVILEGED_FUNCTION */
    {
        ( void ) lInternalIndexOfKernelObject;

        /* If Access Control List feature is not used, all the tasks have
         * access to all the kernel objects. */
        return pdTRUE;
    }

#endif /* #if ( configENABLE_ACCESS_CONTROL_LIST == 1 ) */


BaseType_t xPortIsAuthorizedToAccessBuffer( const void * pvBuffer,
                                            uint32_t ulBufferLength,
                                            uint32_t ulAccessRequested ) /* PRIVILEGED_FUNCTION */

{
    uint32_t i, ulBufferStartAddress, ulBufferEndAddress;
    uint32_t ulRegionStart, ulRegionEnd;
    uint32_t ulMPURegionAccessPermissions;
    BaseType_t xAccessGranted = pdFALSE;
    const xMPU_SETTINGS * xTaskMpuSettings = xTaskGetMPUSettings( NULL ); /* Calling task's MPU settings. */

    if( xSchedulerRunning == pdFALSE )
    {
        /* Grant access to all the kernel objects before the scheduler
         * is started. It is necessary because there is no task running
         * yet and therefore, we cannot use the permissions of any
         * task. */
        xAccessGranted = pdTRUE;
    }
    else if( ( xTaskMpuSettings->ulTaskFlags & portTASK_IS_PRIVILEGED_FLAG ) == portTASK_IS_PRIVILEGED_FLAG )
    {
        xAccessGranted = pdTRUE;
    }
    else
    {
        if( portADD_UINT32_WILL_OVERFLOW( ( ( uint32_t ) pvBuffer ), ( ulBufferLength - 1UL ) ) == pdFALSE )
        {
            ulBufferStartAddress = ( uint32_t ) pvBuffer;
            ulBufferEndAddress = ( ( ( uint32_t ) pvBuffer ) + ulBufferLength - 1UL );

            for( i = 0; i < portTOTAL_NUM_REGIONS; i++ )
            {
                /* Is the MPU region enabled? */
                if( ( xTaskMpuSettings->xRegionsSettings[ i ].ulLAR &
                      portMPU_EN ) == portMPU_EN )
                {
                    ulRegionStart = portEXTRACT_ADDRESS_FROM_BAR( xTaskMpuSettings->xRegionsSettings[ i ].ulBAR );
                    ulRegionEnd = portEXTRACT_ADDRESS_FROM_LAR( xTaskMpuSettings->xRegionsSettings[ i ].ulLAR );

                    if( portIS_ADDRESS_WITHIN_RANGE( ulBufferStartAddress,
                                                     ulRegionStart,
                                                     ulRegionEnd ) &&
                        portIS_ADDRESS_WITHIN_RANGE( ulBufferEndAddress,
                                                     ulRegionStart,
                                                     ulRegionEnd ) )
                    {
                        ulMPURegionAccessPermissions = xTaskMpuSettings->xRegionsSettings[ i ].ulBAR &
                                                       portMPU_BAR_AP_BITMASK;

                        if( ulAccessRequested == tskMPU_READ_PERMISSION ) /* RO. */
                        {
                            if( ( ulMPURegionAccessPermissions == portMPU_RW_EL1_EL0 ) ||
                                ( ulMPURegionAccessPermissions == portMPU_RO_EL1 ) ||
                                ( ulMPURegionAccessPermissions == portMPU_RW_EL1 ) ||
                                ( ulMPURegionAccessPermissions == portMPU_RO_EL1_EL0 ))
                            {
                                xAccessGranted = pdTRUE;
                                break;
                            }
                        }
                        else if( ( ulAccessRequested & tskMPU_WRITE_PERMISSION ) != 0UL ) /* W or RW. */
                        {
                            if( (ulMPURegionAccessPermissions ==  portMPU_RW_EL1_EL0) ||
                            (ulMPURegionAccessPermissions ==  portMPU_RW_EL1))
                            {
                                xAccessGranted = pdTRUE;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    return xAccessGranted;
}
#endif
void vPortStoreTaskMPUSettings( xMPU_SETTINGS * xMPUSettings,
                                    const struct xMEMORY_REGION * const xRegions,
                                    StackType_t * pxBottomOfStack,
                                    configSTACK_DEPTH_TYPE uxStackDepth )
{
    if( xRegions == NULL )
    {
        xMPUSettings->ulTaskFlags = portTASK_IS_PRIVILEGED_NORESTRICTED;
    }
    else
    {
        int ul;
        int index = 0;
        if( uxStackDepth > 0 )
        {
            /* Define the region that allows access to the stack. */
            xMPUSettings->xRegionsSettings[ 0 ].ulIndex = portMPU_TASKSTACK_DSC_IDX;
            xMPUSettings->xRegionsSettings[ 0 ].ulBAR = ( ( uint32_t ) pxBottomOfStack ) | portMPU_XN;
            xMPUSettings->xRegionsSettings[ 0 ].ulLAR = (( ( uint32_t ) pxBottomOfStack + (uxStackDepth * sizeof( StackType_t )) - 1) & 0xFFFFFFC0) | (1UL << 1UL) | portMPU_EN; /* attr index == 1 + enabled */
        }
        for( ul = 1UL; ul <= portNUM_CONFIGURABLE_REGIONS; ul++ )
        {
            if( xRegions[ index ].ulLengthInBytes > 0UL )
            {
                xMPUSettings->xRegionsSettings[ ul ].ulIndex  = ul + portMPU_TASKSTACK_DSC_IDX;
                xMPUSettings->xRegionsSettings[ ul ].ulBAR = (( uint32_t)xRegions[index].pvBaseAddress);
                if (portMPU_REGION_EXECUTE_NEVER & xRegions[ index ].ulParameters)
                    xMPUSettings->xRegionsSettings[ ul ].ulBAR |= portMPU_XN;
                if (xRegions[ index ].ulParameters & portMPU_REGION_READ_WRITE)
                    xMPUSettings->xRegionsSettings[ ul ].ulBAR |= portMPU_RW_EL1_EL0;
                if (xRegions[ index ].ulParameters & portMPU_REGION_READ_ONLY)
                    xMPUSettings->xRegionsSettings[ ul ].ulBAR |= portMPU_RO_EL1_EL0;
                if (xRegions[ index ].ulParameters & portMPU_REGION_PRIVILEGED_READ_WRITE)
                    xMPUSettings->xRegionsSettings[ ul ].ulBAR |= portMPU_RW_EL1;
                if (xRegions[ index ].ulParameters & portMPU_REGION_PRIVILEGED_READ_ONLY)
                    xMPUSettings->xRegionsSettings[ ul ].ulBAR |= portMPU_RO_EL1;

                xMPUSettings->xRegionsSettings[ ul ].ulLAR = ((((uint32_t)xRegions[index].pvBaseAddress) + xRegions[index].ulLengthInBytes - 1) & 0xFFFFFFC0);
                if (xRegions[ index ].ulParameters & portMPU_REGION_CACHEABLE_BUFFERABLE)
                {
                    if ((xRegions[ index ].ulParameters & portMPU_REGION_READ_ONLY) || (xRegions[ index ].ulParameters & portMPU_REGION_PRIVILEGED_READ_ONLY))
                        xMPUSettings->xRegionsSettings[ ul ].ulLAR |= portMPU_EN;
                    else
                        xMPUSettings->xRegionsSettings[ ul ].ulLAR |= ((1 << 1) | portMPU_EN);
                }
                else
                {
                    xMPUSettings->xRegionsSettings[ ul ].ulLAR |= ((3 << 1) | portMPU_EN);
                }
            }
            else
            {
                /* Invalidate the region. */
                xMPUSettings->xRegionsSettings[ ul ].ulIndex  = ul + portMPU_TASKSTACK_DSC_IDX;
                xMPUSettings->xRegionsSettings[ ul ].ulBAR = 0UL;
                xMPUSettings->xRegionsSettings[ ul ].ulLAR = 0UL;
            }
            index++;
        }
    }
}
BaseType_t xPortIsTaskPrivileged( void ) PRIVILEGED_FUNCTION;
BaseType_t xPortIsTaskPrivileged( void )
{
    BaseType_t xTaskIsPrivileged = pdFALSE;
    const xMPU_SETTINGS * xTaskMpuSettings = xTaskGetMPUSettings( NULL ); /* Calling task's MPU settings. */

    if( ( xTaskMpuSettings->ulTaskFlags & portTASK_IS_PRIVILEGED_FLAG ) == portTASK_IS_PRIVILEGED_FLAG )
    {
        xTaskIsPrivileged = pdTRUE;
    }

    return xTaskIsPrivileged;
}

StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                         TaskFunction_t pxCode,
                                         void * pvParameters,
                                         BaseType_t xRunPrivileged,
                                         xMPU_SETTINGS * xMPUSettings ) /* PRIVILEGED_FUNCTION */
{
#if configENABLE_FPU == 1
    if (task_use_fpu == 1)
    {
        pxTopOfStack -= portFPU_STACK_DEEP_SIZE;               /* 8 bytes aligned, D0-D31, FPSCR, FPEXC */
        memset( pxTopOfStack, 0x00, portSIZE_OF_STACK_FPU );
    }
    task_use_fpu = 0;
#endif /* configENABLE_FPU */

    pxTopOfStack --;
    *pxTopOfStack -- = (StackType_t)pxCode + 4;                                 /* R14_IRQ */
    *pxTopOfStack -- = (StackType_t)0x00121212 + (task_debug_pattern << 24);    /* R12 */
    *pxTopOfStack -- = (StackType_t)0x00030303 + (task_debug_pattern << 24);    /* R3 */
    *pxTopOfStack -- = (StackType_t)0x00020202 + (task_debug_pattern << 24);    /* R2 */
    *pxTopOfStack -- = (StackType_t)0x00010101 + (task_debug_pattern << 24);    /* R1 */
    *pxTopOfStack -- = (StackType_t)pvParameters;                               /* R0 */

    *pxTopOfStack -- = (StackType_t)0xFFFFFFFF;                                 /* R14_USR, invalid return */
    *pxTopOfStack -- = (StackType_t)0x00111111 + (task_debug_pattern << 24);    /* R11 */
    *pxTopOfStack -- = (StackType_t)0x00101010 + (task_debug_pattern << 24);    /* R10 */
    *pxTopOfStack -- = (StackType_t)0x00090909 + (task_debug_pattern << 24);    /* R9 */
    *pxTopOfStack -- = (StackType_t)0x00080808 + (task_debug_pattern << 24);    /* R8 */
    *pxTopOfStack -- = (StackType_t)0x00070707 + (task_debug_pattern << 24);    /* R7 */
    *pxTopOfStack -- = (StackType_t)0x00060606 + (task_debug_pattern << 24);    /* R6 */
    *pxTopOfStack -- = (StackType_t)0x00050505 + (task_debug_pattern << 24);    /* R5 */
    *pxTopOfStack -- = (StackType_t)0x00040404 + (task_debug_pattern << 24);    /* R4 */

    *pxTopOfStack -- = (StackType_t)0x000000f8;                                 /* Task interrupt mask */
    *pxTopOfStack -- = (StackType_t)0;                                          /* critical */
    if( xRunPrivileged == pdTRUE )
    {
        xMPUSettings->ulTaskFlags |= portTASK_IS_PRIVILEGED_FLAG;
#if defined(__thumb__)
        *pxTopOfStack = (StackType_t)0x0000013f;                                /* CPSR mode system, T32 (unsolicited)*/
#else
        *pxTopOfStack = (StackType_t)0x0000011f;                                /* CPSR mode system, A32 (unsolicited)*/
#endif
    }
    else
    {
        xMPUSettings->xRegionsSettings[ 0 ].ulBAR |= portMPU_RW_EL1_EL0;
#if defined(__thumb__)
        *pxTopOfStack = (StackType_t)0x00000130;                                /* CPSR mode user, T32 (unsolicited)*/
#else
        *pxTopOfStack = (StackType_t)0x00000110;                                /* CPSR mode user, A32 (unsolicited)*/
#endif
    }
    task_debug_pattern ++;

    #if ( configUSE_MPU_WRAPPERS_V1 == 0 )
    {
        /* This is not NULL only for the duration of a system call. */
        xMPUSettings->xSystemCallStackInfo.ulFlags = 0;
        xMPUSettings->xSystemCallStackInfo.ulLr = 0;
    }
    #endif /* configUSE_MPU_WRAPPERS_V1 == 0 */

    return pxTopOfStack;
}
#else
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
#if configENABLE_FPU == 1
    if (task_use_fpu == 1)
    {
        pxTopOfStack -= portFPU_STACK_DEEP_SIZE;               /* 8 bytes aligned, D0-D31, FPSCR, FPEXC */
        memset( pxTopOfStack, 0x00, portSIZE_OF_STACK_FPU );
    }
    task_use_fpu = 0;
#endif /* configENABLE_FPU */
    pxTopOfStack --;
    *pxTopOfStack -- = (StackType_t)pxCode + 4;                                 /* R14_IRQ */
    *pxTopOfStack -- = (StackType_t)0x00121212 + (task_debug_pattern << 24);    /* R12 */
    *pxTopOfStack -- = (StackType_t)0x00030303 + (task_debug_pattern << 24);    /* R3 */
    *pxTopOfStack -- = (StackType_t)0x00020202 + (task_debug_pattern << 24);    /* R2 */
    *pxTopOfStack -- = (StackType_t)0x00010101 + (task_debug_pattern << 24);    /* R1 */
    *pxTopOfStack -- = (StackType_t)pvParameters;                               /* R0 */

    *pxTopOfStack -- = (StackType_t)0xFFFFFFFF;                                 /* R14_USR, invalid return */
    *pxTopOfStack -- = (StackType_t)0x00111111 + (task_debug_pattern << 24);    /* R11 */
    *pxTopOfStack -- = (StackType_t)0x00101010 + (task_debug_pattern << 24);    /* R10 */
    *pxTopOfStack -- = (StackType_t)0x00090909 + (task_debug_pattern << 24);    /* R9 */
    *pxTopOfStack -- = (StackType_t)0x00080808 + (task_debug_pattern << 24);    /* R8 */
    *pxTopOfStack -- = (StackType_t)0x00070707 + (task_debug_pattern << 24);    /* R7 */
    *pxTopOfStack -- = (StackType_t)0x00060606 + (task_debug_pattern << 24);    /* R6 */
    *pxTopOfStack -- = (StackType_t)0x00050505 + (task_debug_pattern << 24);    /* R5 */
    *pxTopOfStack -- = (StackType_t)0x00040404 + (task_debug_pattern << 24);    /* R4 */

    *pxTopOfStack -- = (StackType_t)0x000000f8;                /* Task interrupt mask */
    *pxTopOfStack -- = (StackType_t)0;                         /* critical */
#if defined(__thumb__)
    *pxTopOfStack = (StackType_t)0x0000013f;                   /* CPSR mode system, T32 (unsolicited)*/
#else
    *pxTopOfStack = (StackType_t)0x0000011f;                   /* CPSR mode system, A32 (unsolicited)*/
#endif

    task_debug_pattern ++;

    return pxTopOfStack;
}
#endif
/*-----------------------------------------------------------*/
extern void prvSetupTimerInterrupt(void);
#if ( configNUMBER_OF_CORES > 1 )
volatile uint32_t syncPoint[configNUMBER_OF_CORES] = {0};

void vPortSyncPoint(uint32_t coreId)
{
    syncPoint[coreId] ++;

    while (1)
    {
        int i;
        uint32_t mask = 0;

        for (i = 0; i < configNUMBER_OF_CORES; i ++)
            mask = mask | ((syncPoint[i] & 1) << i);
        /* Wait until all enabled CPU cores are synchronized */
        if ((mask & RUNNING_CPUS_MASK) == RUNNING_CPUS_MASK)
            break;
    }
}

#endif
BaseType_t xPortStartScheduler( void )
{
#if ( configNUMBER_OF_CORES > 1 )
    uint32_t coreId = portGET_CORE_ID();

    MRUInit(CORE_ID_2_MRU_ID(coreId), CFG_MRU_CHANNEL_SMP, CFG_MRU_MB_ID_SMP, CFG_MRU_INTERRUPT_SMP);

#if !defined(_S32DS_FREERTOS_)
    vInitInterruptTable(coreId, MRU_INT_ID(CFG_MRU_INTERRUPT_SMP), MRU_IRQ_Handler);
    vGicEnableInterrupt(MRU_INT_ID(CFG_MRU_INTERRUPT_SMP), 2);
#endif

    if (configCORE_MASTER == coreId) {
        MRURegisterMBCB(CFG_MRU_INTERRUPT_SMP, (void*)vCoreSignal);
#endif
        /* Start the timer that generates the tick ISR.  Interrupts are disabled here already. */
        prvSetupTimerInterrupt();
#if ( configNUMBER_OF_CORES > 1 )
    }

    vPortSyncPoint(coreId);
#endif
    /* Start the first task. */
    vPortStartFirstTask();

    /* Should not get here! */
    return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* It is unlikely that the ARM port will require this function as there
    is nothing to return to.  */
}
#if ( configNUMBER_OF_CORES > 1 )
#else
/*-----------------------------------------------------------*/
void vPortEnterCritical( void ) PRIVILEGED_FUNCTION;
void vPortEnterCritical( void )
{
    portDISABLE_INTERRUPTS();
    uxCriticalNesting++;
}
/*-----------------------------------------------------------*/
void vPortExitCritical( void )  PRIVILEGED_FUNCTION;
void vPortExitCritical( void )
{
    configASSERT( uxCriticalNesting );
    uxCriticalNesting--;
    if( uxCriticalNesting == 0 )
    {
        portENABLE_INTERRUPTS();
    }
}
/*-----------------------------------------------------------*/
#endif
/*-----------------------------------------------------------*/

#if ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
BaseType_t xTaskCreateFpu(  TaskFunction_t pxTaskCode,
                                const char * const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                                const configSTACK_DEPTH_TYPE usStackDepth,
                                void * const pvParameters,
                                UBaseType_t uxPriority,
                                pTaskHandle_t * const pxCreatedTask ) PRIVILEGED_FUNCTION;
    /*
     * This function is called to create a task that uses the FPU instead of xCreateTask
     */
    BaseType_t xTaskCreateFpu(  TaskFunction_t pxTaskCode,
                                const char * const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                                const configSTACK_DEPTH_TYPE usStackDepth,
                                void * const pvParameters,
                                UBaseType_t uxPriority,
                                pTaskHandle_t * const pxCreatedTask )
    {
        BaseType_t xReturn;
        TaskHandle_t vTaskToSet;
        TaskHandle_t * vCreatedTask;

        #if ( configENABLE_FPU == 1 )
            {
                if(pxCreatedTask != NULL)
                {
                    vCreatedTask = pxCreatedTask;
                }
                else
                {
                    vCreatedTask = &vTaskToSet;
                }
                task_use_fpu = 1;
                xReturn = xTaskCreate( pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, vCreatedTask );
                vTaskSetThreadLocalStoragePointer((*vCreatedTask), 0, (void *)(*(StackType_t *)(*(StackType_t *)vCreatedTask) + portSIZE_OF_TASK_STACK_USE_FPU));
            }
        #else /* configENABLE_FPU */
            {
                xReturn = xTaskCreate( pxTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask );
            }
        #endif /* configENABLE_FPU */

        return xReturn;
    }
#endif /* configSUPPORT_DYNAMIC_ALLOCATION */

/*-----------------------------------------------------------*/

#if ( configSUPPORT_STATIC_ALLOCATION == 1 )
    TaskHandle_t xTaskCreateStaticFpu(  TaskFunction_t pxTaskCode,
                                        const char * const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                                        const uint32_t ulStackDepth,
                                        void * const pvParameters,
                                        UBaseType_t uxPriority,
                                        StackType_t * const puxStackBuffer,
                                        StaticTask_t * const pxTaskBuffer ) PRIVILEGED_FUNCTION;
    /*
     * This function is called to create a task that uses the FPU instead of xCreateTaskStatic
     */
    TaskHandle_t xTaskCreateStaticFpu(  TaskFunction_t pxTaskCode,
                                        const char * const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                                        const uint32_t ulStackDepth,
                                        void * const pvParameters,
                                        UBaseType_t uxPriority,
                                        StackType_t * const puxStackBuffer,
                                        StaticTask_t * const pxTaskBuffer )
    {
        TaskHandle_t xReturn;

        #if ( configENABLE_FPU == 1 )
            {
                task_use_fpu = 1;
                xReturn = xTaskCreateStatic( pxTaskCode, pcName, ulStackDepth,pvParameters, uxPriority, puxStackBuffer, pxTaskBuffer );
                vTaskSetThreadLocalStoragePointer(xReturn, 0, (void *)(*(StackType_t *)(*(StackType_t *)(&xReturn)) + portSIZE_OF_TASK_STACK_USE_FPU));
            }
        #else /* configENABLE_FPU */
            {
                xReturn = xTaskCreateStatic( pxTaskCode, pcName, ulStackDepth,pvParameters, uxPriority, puxStackBuffer, pxTaskBuffer );
            }
        #endif /* configENABLE_FPU */

        return xReturn;
    }
#endif /* configSUPPORT_STATIC_ALLOCATION */

/*-----------------------------------------------------------*/

void vPortUndefinedInstruction(void) __attribute__ (( naked ))
#if defined(_S32DS_FREERTOS_)
    __attribute__ ((section (".systeminit")));
#else
    __attribute__((section(".handlers")));
#endif

#if (configENABLE_FPU == 1) || (configENABLE_MPU == 1)

void vPortUndefinedInstruction(void)
{
    OSASM(" sub     lr, lr, #2                              \t\n"); /* update link register */
    OSASM(" stmfd   r13!, {r0-r5, r12, r14}                 \t\n"); /* save r0-r5, r12, r14 to und stack */

#if (configENABLE_MPU == 1)
    /* Check if this is a system call first (MPU feature) */
    OSASM(" ldr r0, =__syscalls_code_start__                \t\n");
    OSASM(" ldr r1, =__syscalls_code_end__                  \t\n");
    OSASM(" cmp lr, r0                                      \t\n");
    OSASM(" ble undefine_exc                                \t\n");
    OSASM(" cmp lr, r1                                      \t\n");
    OSASM(" bgt undefine_exc                                \t\n");

    /* Handle MPU system call */
    OSASM(" mrs r0, sp_usr                                  \t\n");
    OSASM(" ldr r4, [r0]                                    \t\n");
    OSASM(" add r0, r0, #4                                  \t\n");
    OSASM(" msr sp_usr, r0                                  \t\n"); /* unload user stack */

    OSASM(" mrs r0, lr_usr                                  \t\n");
    OSASM(" blx vSystemCallEnter                            \t\n");
    OSASM(" cmp r0, 0                                       \t\n");
    OSASM(" bne undefine_exc                                \t\n");

    OSASM(" ldr r0, =vSystemCallExit                        \t\n");
    OSASM(" msr lr_usr, r0                                  \t\n");

    OSASM(" mrs r0, spsr                                    \t\n");
    OSASM(" orr r0, r0, 15                                  \t\n"); /* return to supervisor */
    OSASM(" msr spsr, r0                                    \t\n");
    OSASM(" ldr r0, =uxSystemCallImplementations            \t\n");
    OSASM(" add r0, r0, r4, lsl #2                          \t\n");
    OSASM(" ldr r14, [r0]                                   \t\n");

    OSASM(" ldmfd r13!, {r0-r5, r12}                        \t\n"); /* restore r0-r5, r12 from und stack */
    OSASM(" add sp, sp, #4                                  \t\n");
    OSASM(" movs pc, r14                                    \t\n");
#endif /* configENABLE_MPU */

#if (configENABLE_FPU == 1)
    /* Check if this is an FPU exception */
    OSASM(" fmrx    r0, FPEXC                               \t\n"); /* Check FPEXC.EN is on */
    OSASM(" tst     r0, #(1 << 30)                          \t\n");
#if defined(__thumb__)
    OSASM(" it      ne                                      \t\n");
#endif /* __thumb__ */
    OSASM(" bne     undefine_exc                            \t\n");

    /* FPEXC.EN == 0 : for fpu exception */
    OSASM(" orr     r0, r0, #(1 << 30)                      \t\n");
    OSASM(" vmsr    FPEXC, r0                               \t\n"); /* Enable FPU */

    OSASM(" mov     r0, #0                                  \t\n");
    OSASM(" mov     r1, #0                                  \t\n");
    OSASM(" bl      pvTaskGetThreadLocalStoragePointer      \t\n");
    OSASM(" ldr     r2, =newFpuCtx                          \t\n");
    OSASM(" str     r0, [r2]                                \t\n"); /* get pointer & solicited/unsolicited (size) */

    OSASM(" mrs     r1, spsr                                \t\n"); /* Read SPSR into r1 to find previous program's mode */
    OSASM(" and     r1, r1, #0x1F                           \t\n");
    OSASM(" cmp     r1, #0x1F                               \t\n"); /* check if undefine from task(sys) or from interrupt(svc) */
    OSASM(" bne     svc_mode                                \t\n");

    /* spsr.M == 0x1F : An task which uses FPU instruction */
    OSASM(" ldr     r2, =prevFpuCtx                         \t\n");
    OSASM(" ldr     r2, [r2]                                \t\n"); /* get FPU stack pointer of prev task: prevFpuCtx */
    OSASM(" ldr     r1, =b_SwitchByIRQ                      \t\n");
    OSASM(" ldr     r1, [r1]                                \t\n"); /* get boolean check switch by IRQ or Yield: b_SwitchByIRQ */
    OSASM(" cmp     r2, #0                                  \t\n"); /* compare prevFpuCtx with NULL */
    OSASM(" beq     1f                                      \t\n");

    /* prevFpuCtx != 0 : prev task has used fpu */
    OSASM(" mov     r3, #0                                  \t\n");
    OSASM(" cmp     r0, r2                                  \t\n"); /* compare newFpuCtx with prevFpuCtx */
    OSASM(" beq     OR                                      \t\n");
    OSASM(" mov     r3, #1                                  \t\n");

    OSASM(" OR:                                             \t\n");
    OSASM(" cmp     r1, #1                                  \t\n"); /* compare b_SwitchByIRQ with 1 */
    OSASM(" bne     END_OR                                  \t\n");
    OSASM(" mov     r3, #1                                  \t\n");

    OSASM(" END_OR:                                         \t\n");

    OSASM(" cmp     r3, #1                                  \t\n"); /* compare R3 for (newFpuCtx != prevFpuCtx) ||  (b_SwitchByIRQ == 1) */
    OSASM(" bne     2f                                      \t\n");

    /* newFpuCtx != prevFpuCtx : the previous task is other than
     * current task else do nothing, same task was scheduled */
    /* save context from fpu to prevFpuCtx (fpu_buffer, sizetask) */
    OSASM(" cmp     r1, #0x1                                \t\n"); /* compare b_SwitchByIRQ with 1 */
    OSASM(" beq     3f                                      \t\n");

    /* bit0 == 0 : switch by yeild function
     * save (D8-D15), set first stack position = 0(8 bytes) */
    OSASM(" mov     r1, #0                                  \t\n");
    OSASM(" stmdb   r2!, {r1}                               \t\n");
    OSASM(" vstmdb  r2!, {d8-d15}                           \t\n");
    OSASM(" b       4f                                      \t\n");

    /* bit0 == 1 : switch by IRQ function
     * save (D0-D31), set first stack position = 1(8 bytes) */
    OSASM(" 3:                                              \t\n");
    OSASM(" mov     r1, #1LL                                \t\n");
    OSASM(" stmdb   r2!, {r1}                               \t\n");
    OSASM(" vstmdb  r2!, {d0-d15}                           \t\n");
    OSASM(" vstmdb  r2!, {d16-d31}                          \t\n");

    OSASM(" 4:                                              \t\n");
    OSASM(" fmrx    r1, FPSCR                               \t\n"); /* save: FPSCR, FPEXC */
    OSASM(" stmdb   r2!, {r1}                               \t\n");
    OSASM(" fmrx    r1, FPEXC                               \t\n");
    OSASM(" stmdb   r2!, {r1}                               \t\n");

    /* restore fpu from newFpuCtx(fpu_buffer, size) */
    OSASM(" ldmdb   r0!, {r1}                               \t\n"); /* restore fpu from newFpuCtx(fpu_buffer, size) to FPU based on first 8 bytes for size */
    OSASM(" cmp     r1, #1                                  \t\n"); /* compare first stack position with 1 */
    OSASM(" beq     3f                                      \t\n");

    /* first stack position == 0 : switch by yeild function, restore (D8-D15)*/
    OSASM(" vldmdb  r0!, {d8-d15}                           \t\n");
    OSASM(" b       4f                                      \t\n");

    /* first stack position == 1 : switch by irq, restore (D0-D31)*/
    OSASM(" 3:                                              \t\n");
    OSASM(" vldmdb  r0!, {d0-d15}                           \t\n");
    OSASM(" vldmdb  r0!, {d16-d31}                          \t\n");

    OSASM(" 4:                                              \t\n");
    OSASM(" ldmdb   r0!, {r1}                               \t\n"); /* restore: FPSCR, FPEXC */
    OSASM(" fmxr    FPSCR, r1                               \t\n");
    OSASM(" ldmdb   r0!, {r1}                               \t\n");
    OSASM(" fmxr    FPEXC, r1                               \t\n");
    OSASM(" b       2f                                      \t\n");

    /* prevFpuCtx == 0: there is no previous task using fpu,
     * so the crt can load its own registers */
    OSASM(" 1:                                              \t\n");
    OSASM(" ldmdb   r0!, {r1}                               \t\n"); /* restore fpu from newFpuCtx */
    OSASM(" vldmdb  r0!, {d0-d15}                           \t\n");
    OSASM(" vldmdb  r0!, {d16-d31}                          \t\n");
    OSASM(" ldmdb   r0!, {r1}                               \t\n");
    OSASM(" fmxr    FPSCR, r1                               \t\n"); /* restore: FPSCR, FPEXC */
    OSASM(" ldmdb   r0!, {r1}                               \t\n");
    OSASM(" fmxr    FPEXC, r1                               \t\n");

    /* exit FPU execution for a task using the FPU instruction */
    OSASM(" 2:                                              \t\n");
    OSASM(" ldr     r0, =newFpuCtx                          \t\n");
    OSASM(" ldr     r0, [r0]                                \t\n");
    OSASM(" ldr     r1, =prevFpuCtx                         \t\n");
    OSASM(" str     r0, [r1]                                \t\n"); /* prevFpuCtx <= newFpuCtx */
    OSASM(" b       exit                                    \t\n");

    /* spsr.M == 0x13 : An interrupt which uses FPU instruction */
    OSASM(" svc_mode:                                       \t\n");
    OSASM(" ldr     r0, =ulPortInterruptHasFPUContext       \t\n");
    OSASM(" ldr     r0, [r0]                                \t\n");
    OSASM(" cmp     r0, #0                                  \t\n"); /* check for task or interrupt was interrupted */
    OSASM(" bgt     1f                                      \t\n");

    OSASM(" ldr     r0, =prevFpuCtx                         \t\n");
    OSASM(" ldr     r2, [r0]                                \t\n");
    OSASM(" ldr     r1, =b_SwitchByIRQ                      \t\n");
    OSASM(" ldr     r1, [r1]                                \t\n"); /* get boolean check switch by IRQ or Yield: b_SwitchByIRQ */
    OSASM(" cmp     r1, #1                                  \t\n"); /* compare b_SwitchByIRQ with 1 */
    OSASM(" beq     2f                                      \t\n");

    /* ulPortInterruptHasFPUContext < 1 : task was interrupted */
    OSASM(" cmp     r2, #0                                  \t\n"); /* compare prevFpuCtx with NULL */
    OSASM(" beq     2f                                      \t\n");

    /* prevFpuCtx != 0 : prev task has used fpu.
     * Don't need to save fpu if previous task didn't use fpu*/
    OSASM(" mov     r1, #1LL                                \t\n"); /* save context from fpu to prevFpuCtx (fpu_buffer, sizetask) */
    OSASM(" stmdb   r2!, {r1}                               \t\n"); /* save 8 bytes 1 */
    OSASM(" vstmdb  r2!, {d0-d15}                           \t\n");
    OSASM(" vstmdb  r2!, {d16-d31}                          \t\n");
    OSASM(" fmrx    r1, FPSCR                               \t\n"); /* save: FPSCR, FPEXC */
    OSASM(" stmdb   r2!, {r1}                               \t\n");
    OSASM(" fmrx    r1, FPEXC                               \t\n");
    OSASM(" stmdb   r2!, {r1}                               \t\n");

    OSASM(" 2:                                              \t\n");
    OSASM(" mov     r1, #0                                  \t\n");
    OSASM(" str     r1, [r0]                                \t\n"); /* prevFpuCtx = 0, save was done */
    OSASM(" ldr     r2, =b_SwitchByIRQ                      \t\n");
    OSASM(" str     r1, [r2]                                \t\n"); /* get boolean check switch by IRQ or Yield: b_SwitchByIRQ */
    OSASM(" mrs     r0, sp_irq                              \t\n"); /* set first new_global_irq_fpu_stack's value is start of irq stack */
    OSASM(" ldr     r1, =new_global_irq_fpu_stack           \t\n");
    OSASM(" str     r0, [r1]                                \t\n");
    OSASM(" ldr     r1, =global_irq_fpu_stack               \t\n");
    OSASM(" str     r0, [r1]                                \t\n"); /* global_irq_fpu_stack = new_global_irq_fpu_stack */

    OSASM(" ldr     r1, =ulPortInterruptHasFPUContext       \t\n");
    OSASM(" ldr     r0, [r1]                                \t\n");
    OSASM(" add     r0, r0, #1                              \t\n");
    OSASM(" str     r0, [r1]                                \t\n"); /* ulPortInterruptHasFPUContext++ */
    OSASM(" b       exit                                    \t\n");

    /* ulPortInterruptHasFPUContext >= 1 : interrupt was interrupted */
    OSASM(" 1:                                              \t\n");
    OSASM(" ldr     r1, =uxInterruptNested                  \t\n");
    OSASM(" ldr     r1, [r1]                                \t\n");
    OSASM(" cmp     r0, r1                                  \t\n"); /* ulPortInterruptHasFPUContext < uxInterruptNested */
    OSASM(" bge     exit                                    \t\n");

    /* only if the number of tasks using the fpu is greater than
     * the number of nested interrupts.Avoid the case that in nested
     * interrupts an interrupt uses fpu before and after the other
     * interrupt takes precedence. Next time we don't need to save
     * the fpu because we saved it last time */
    /* IRQ1( FPU Instruction ) --> IRQ2 --> IRQ1( FPU Instruction ) */
    OSASM(" ldr     r0, =new_global_irq_fpu_stack           \t\n");
    OSASM(" ldr     r2, [r0]                                \t\n");
    OSASM(" ldr     r3, =global_irq_fpu_stack               \t\n");
    OSASM(" ldr     r1, [r3]                                \t\n");
    OSASM(" cmp     r1, r2                                  \t\n"); /* new_global_irq_fpu_stack != global_irq_fpu_stack */
    OSASM(" beq     3f                                      \t\n");

    /* only if the new_global_irq_fpu_stack != global_irq_fpu_stack
     * Avoid the case that in nested interrupts, in interrupt(IRQ1)
     * occurs more than 2 other interrupts (these interrupts use fpu)
     * (IRQ2,IRQ3,..). Then in the next interrupt (IRQ3/IRQ2(second
     * times)), we don't need to save the fpu because the previous
     * interrupt(IRQ2) is already saved */
    /* IRQ1 --> IRQ2( FPU Instruction ) --> IRQ1 --> IRQ2/IRQ3( FPU Instruction ) --> IRQ1 */
    OSASM(" str     r2, [r3]                                \t\n"); /* global_irq_fpu_stack = new_global_irq_fpu_stack */
    OSASM(" mov     r1, #1LL                                \t\n"); /* save context from fpu to new_global_irq_fpu_stack (fpu_buffer, size_interrupt) */
    OSASM(" stmdb   r2!, {r1}                               \t\n"); /* save 8 bytes 1 */
    OSASM(" vstmdb  r2!, {d0-d15}                           \t\n");
    OSASM(" vstmdb  r2!, {d16-d31}                          \t\n");
    OSASM(" fmrx    r1, FPSCR                               \t\n"); /* save: FPSCR, FPEXC */
    OSASM(" stmdb   r2!, {r1}                               \t\n");
    OSASM(" fmrx    r1, FPEXC                               \t\n");
    OSASM(" stmdb   r2!, {r1}                               \t\n");

    OSASM(" 3:                                              \t\n");
    OSASM(" ldr     r1, =ulPortInterruptHasFPUContext       \t\n");
    OSASM(" ldr     r0, [r1]                                \t\n");
    OSASM(" add     r0, r0, #1                              \t\n");
    OSASM(" str     r0, [r1]                                \t\n"); /* ulPortInterruptHasFPUContext++ */
#endif /* configENABLE_FPU */

    /* return from undefine instruction */
    OSASM(" exit:                                           \t\n");
#if defined(__thumb__)
    OSASM(" ldmfd   r13!, {r0-r5, r12, r14}                 \t\n"); /* restore r0-r5, r12 from und stack */
    OSASM(" movs    pc, lr                                  \t\n");
#else
    OSASM(" ldmfd   r13!, {r0-r5, r12, pc}^                 \t\n"); /* restore r0-r5, r12, r15 from und stack */
#endif

    /* Handle undefined exception */
    OSASM(" undefine_exc:                                   \t\n");
    OSASM(" b .                                             \t\n"); /* Loop */
}
#else
void vPortUndefinedInstruction(void)
{
    OSASM(" b .                                             \t\n"); /* Loop */
}
#endif /* configENABLE_MPU */
/*-----------------------------------------------------------*/

extern void* __VECTOR_RAM[];
/*-----------------------------------------------------------*/
void vPortInterruptDispatcher(void) __attribute__ (( naked ))
#if defined(_S32DS_FREERTOS_)
    __attribute__ ((section (".systeminit")));
#else
    __attribute__((section(".handlers")));
#endif

#if ( configNUMBER_OF_CORES > 1 )
void vPortInterruptDispatcher(void)
{
    OSASM(" clrex ");
    OSASM(" push {r0, r1, r2, r3, r12, r14}                 \t\n");
    OSASM(" blx getCoreIndex                                \t\n"); /* r0 <-- core ID */
    OSASM(" mrc p15, 0, r14, c12, c12, 0                    \t\n"); /* get interrupt id, r14 */
    OSASM(" mrc p15, 0, r12, c12, c11, 3                    \t\n"); /* get interrupt priority, r12 */
    OSASM(" cmp r12,%0\t\n"::"i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY ):);
    OSASM(" bge osInt                                       \t\n"); /* OS interrupt */
    OSASM(" mrs r1, LR_svc                                  \t\n");
    OSASM(" mrs r2, SPSR_svc                                \t\n");
    OSASM(" mrs r3, SPSR                                    \t\n");
    OSASM(" push {r1, r2, r3, r14}                          \t\n"); /* save: LR_svc, SPSR_svc, SPSR_irq and the interrupt ID */
#if configENABLE_FPU == 1
    OSASM(" fmrx    r2, FPEXC                               \t\n");
    OSASM(" and     r2, r2, #~(1 << 30)                     \t\n");
    OSASM(" fmxr    FPEXC, r2                               \t\n"); /* Disable FPU */
    OSASM(" ldr     r3, =new_global_irq_fpu_stack           \t\n");
    OSASM(" ldr     r1, [r3]                                \t\n"); /* get value of new_global_irq_fpu_stack */
    OSASM(" ldr     r2, =ulPortInterruptHasFPUContext       \t\n");
    OSASM(" ldr     r2, [r2]                                \t\n"); /* get value of ulPortInterruptHasFPUContext */
    OSASM(" push    {r1, r2}                                \t\n"); /* save these to IRQ stack */
    OSASM(" sub     r1, r1, %0\t\n"::"i" (portSIZE_OF_STACK_FPU):);
    OSASM(" str     r1, [r3]                                \t\n"); /* set new irq_fpu stack */
#endif /* configENABLE_FPU */

#if defined(_S32DS_FREERTOS_)
    OSASM("ldr  r1, =R52VtableRefArray                      \t\n"); /* Get VTABLE list */
    OSASM("add  r1, r1, r0, lsl #2                          \t\n"); /* Add core offset R0 = coreID*/
    OSASM("ldr  r0, [r1]                                    \t\n"); /* Get VTABLE */
#else
    OSASM(" cmp r14, #32                                    \t\n"); /* PPI, SGI */
    OSASM(" bcs __spi_irq                                   \t\n");
    OSASM(" cmp r0, #0                                      \t\n");
    OSASM(" beq __spi_irq                                   \t\n");
    OSASM(" sub r0, r0, #1                                  \t\n");
    OSASM(" ldr r1, =__VECTOR_RAM                           \t\n");
    OSASM(" lsl r0, r0, %0                       \t\n" : : "i"(7));
    OSASM(" add r1, r1, %0     \t\n" : : "i"(configGIC_INTERRUPTS << 2));
    OSASM(" add r0, r0, r1                                  \t\n");
    OSASM(" b __call_irq                                    \t\n");
    OSASM(" __spi_irq:                                      \t\n");
    OSASM(" ldr r0, =__VECTOR_RAM                           \t\n");
    OSASM(" __call_irq:                                     \t\n");
#endif

    OSASM(" ldr r2, [r0, r14, lsl #2]                       \t\n"); /* get user interrupt vector */
    OSASM(" cpsie i, #0x13                                  \t\n"); /* enable interrupts and enter to svc mode */
    OSASM(" cmp r2, #0                                      \t\n");
#if defined(__thumb__)
    OSASM(" it ne                                           \t\n");
#endif
    OSASM(" blxne r2                                        \t\n"); /* call user interrupt handler */
    OSASM(" cpsid i, 0x12                                   \t\n"); /* disable interrupts and enter to irq mode */
#if configENABLE_FPU == 1
    OSASM(" ldr     r1, =ulPortInterruptHasFPUContext       \t\n");
    OSASM(" ldr     r2, [r1]                                \t\n");
    OSASM(" pop     {r3, r14}                               \t\n"); /* restore: new_global_irq_fpu_stack, local_ulPortInterruptHasFPUContext */
    OSASM(" cmp     r14, r2                                 \t\n"); /* compare local_ulPortInterruptHasFPUContext with ulPortInterruptHasFPUContext */
    OSASM(" bge     1f                                      \t\n");
    /* local_ulPortInterruptHasFPUContext < ulPortInterruptHasFPUContext :
     * the irq uses fpu*/
    OSASM(" sub     r2, r2, #1                              \t\n"); /* ulPortInterruptHasFPUContext -- */
    OSASM(" str     r2, [r1]                                \t\n");
    OSASM(" cmp     r2, #0                                  \t\n"); /* compare ulPortInterruptHasFPUContext with NULL */
    OSASM(" beq     2f                                      \t\n");
    /* ulPortInterruptHasFPUContext != 0 : this is not the last IRQ
     * in the nested interrupt */
    OSASM(" ldr     r2, =new_global_irq_fpu_stack           \t\n");
    OSASM(" ldr     r2, [r2]                                \t\n");
    OSASM(" ldr     r1, =global_irq_fpu_stack               \t\n");
    OSASM(" str     r2, [r1]                                \t\n"); /* global_irq_fpu_stack <= new_global_irq_fpu_stack */
    OSASM(" ldmdb   r2!, {r1}                               \t\n");  /* restore fpu from new_global_irq_fpu_stack */
    OSASM(" vldmdb  r2!, {d0-d15}                           \t\n");
    OSASM(" vldmdb  r2!, {d16-d31}                          \t\n");
    OSASM(" ldmdb   r2!, {r1}                               \t\n"); /* restore: FPSCR, FPEXC */
    OSASM(" fmxr    FPSCR, r1                               \t\n");
    OSASM(" ldmdb   r2!, {r1}                               \t\n");
    OSASM(" fmxr    FPEXC, r1                               \t\n");
    OSASM(" ldr     r2, =new_global_irq_fpu_stack           \t\n");
    OSASM(" str     r3, [r2]                                \t\n"); /* restore: new_global_irq_fpu_stack */
    /* ulPortInterruptHasFPUContext == 0 : this is the last IRQ use fpu */
    OSASM(" 2:                                              \t\n");
    OSASM(" fmrx    r2, FPEXC                               \t\n"); /* Disable FPU */
    OSASM(" and     r2, r2, #~(1 << 30)                     \t\n");
    OSASM(" fmxr    FPEXC, r2                               \t\n");
    /* local_ulPortInterruptHasFPUContext == ulPortInterruptHasFPUContext :
     * the irq not uses fpu */
    OSASM(" 1:                                              \t\n");
    OSASM(" ldr     r1, =uxInterruptNested                  \t\n");
    OSASM(" ldr     r1, [r1]                                \t\n");
    OSASM(" cmp     r1, #1                                  \t\n"); /* compare uxInterruptNested with 1 */
    OSASM(" bne     3f                                      \t\n");
    /* uxInterruptNested == 1 : This is the last IRQ
     * reset before return to task. Prepare for another interrupt */
    OSASM(" ldr     r1, =global_irq_fpu_stack               \t\n");
    OSASM(" ldr     r2, =new_global_irq_fpu_stack           \t\n");
    OSASM(" mov     r3, #0                                  \t\n");
    OSASM(" str     r3, [r1]                                \t\n"); /* reset global irq_fpu stack */
    OSASM(" str     r3, [r2]                                \t\n"); /* reset new irq_fpu stack */
    OSASM(" mov     r0, #0                                  \t\n"); /* first parameter 0 */
    OSASM(" mov     r1, #0                                  \t\n"); /* Second parameter 1 */
    OSASM(" bl      pvTaskGetThreadLocalStoragePointer      \t\n"); /* Call function */
    OSASM(" mov     r3, r0                                  \t\n"); /* Save result to R3 */
    OSASM(" ldr     r0, =prevFpuCtx                         \t\n");
    OSASM(" ldr     r1, [r0]                                \t\n"); /* Get prevFpuCtx */
    OSASM(" cmp     r1, r3                                  \t\n"); /* Solicited for switched out task */
    OSASM(" beq     3f                                      \t\n");
    /* newFpuCtx != prevFpuCtx : the switching occurs */
    /* unsolicited for switched out task,
     * buffer to save status, registers d0-d31 */
    OSASM(" ldr     r2, =b_SwitchByIRQ                      \t\n");
    OSASM(" ldr     r1, [r2]                                \t\n"); /* get boolean check switch by IRQ or Yield: b_SwitchByIRQ */
    OSASM(" mov     r1, #1                                  \t\n"); /* b_SwitchByIRQ = 1 */
    OSASM(" str     r1, [r2]                                \t\n");
    OSASM(" 3:                                              \t\n");
#endif /* configENABLE_FPU */
    OSASM(" pop {r1, r2, r3, r14}                           \t\n"); /* restore: LR_svc, SPSR_svc, SPSR_irq and the interrupt ID */
    OSASM(" msr LR_svc, r1                                  \t\n");
    OSASM(" msr SPSR_svc, r2                                \t\n");
    OSASM(" msr SPSR_fsxc, r3                               \t\n");
    OSASM(" mcr p15, 0, r14, c12, c12, 1                    \t\n"); /* End Of Interrupt */
    OSASM(" dsb                                             \t\n");
    OSASM(" ldmfd sp!, {r0-r3, r12, r14}                    \t\n"); /* restore user registers */
    OSASM(" subs pc, lr, #4                                 \t\n"); /* return from irq */
    OSASM(" osInt:                                          \t\n"); /* OS interrupt handler */
    OSASM(" ldr r12, =uxInterruptNested                     \t\n");
    OSASM(" lsl r2, r0, #2                                  \t\n");
    OSASM(" add r12, r12, r2                                \t\n"); /* Get current TCB ofset CoreID */
    OSASM(" ldr r3, [r12]                                   \t\n");
    OSASM(" cmp r3, #0                                      \t\n");
    OSASM(" add r3, r3, #1                                  \t\n"); /* inc uxInterruptNested */
    OSASM(" str r3, [r12]                                   \t\n"); /* save uxInterruptNested */
    /* should save core ID */
#if defined(__thumb__)
    OSASM(" beq 2f                                          \t\n"); /* jmp uxInterruptNested == 1 */
    OSASM(" sub sp, sp, #4                                  \t\n"); /* reserve to stack uxInterruptNested > 1 */
    OSASM(" str r2, [sp]                                    \t\n"); /* save coreid uxInterruptNested > 1 */
    OSASM(" b 1f                                            \t\n"); /* jump to "prepare Call user handler" */
    OSASM(" 2:                                              \t\n");
#else
    OSASM(" subne sp, sp, #4                                \t\n"); /* reserve to stack uxInterruptNested > 1 */
    OSASM(" strne r2, [sp]                                  \t\n"); /* save coreid uxInterruptNested > 1 */
    OSASM(" bne 1f                                          \t\n"); /* jump to "prepare Call user handler" */
#endif
    OSASM(" mrs r12, SP_usr                                 \t\n"); /* get user stack to r12 */
    OSASM(" ldmfd sp!, {r0, r1, r3}                         \t\n"); /* unload irq stack, use: r0, r1, r3. r2 -- core id */
    OSASM(" sub r12, r12, #24                               \t\n");
    OSASM(" stm r12!, {r0, r1, r3}                          \t\n"); /* save users regs to user stack r0, r1, r2 */
    OSASM(" ldmfd sp!, {r0, r1, r3}                         \t\n"); /* unload irq stack */
    OSASM(" stm r12!, {r0, r1, r3}                          \t\n"); /* save users regs to user stack r3, r12, r14 */
    OSASM(" sub r12, r12, #24                               \t\n");
    OSASM(" ldr r1, =pxCurrentTCBs                          \t\n"); /* Get current TCB */
    OSASM(" add r1, r1, r2                                  \t\n"); /* Get current TCB ofset CoreID */
    OSASM(" ldr r1, [r1]                                    \t\n");
    OSASM(" mrc p15, 0, r3, c4, c6, 0                       \t\n"); /* get interrupt mask, r3 */
#if defined(__thumb__)
    OSASM(" sub r12, #4                                     \t\n"); /* Space for LR_USR */
    OSASM(" stmfd r12, {r3-r11}                             \t\n"); /* int_mask + save registers r14_usr, old task */
    OSASM(" mrs r0, LR_usr                                  \t\n"); /* get user LINK to r2 */
    OSASM(" str r0, [r12]                                   \t\n"); /* save to user stack */
    OSASM(" sub r12, r12, #36                               \t\n"); /* update r12 */
#else
    OSASM(" stmfd r12, {r3-r11, r14}^                       \t\n"); /* int_mask + save registers r14_usr, old task */
    OSASM(" sub r12, r12, #40                               \t\n"); /* update r12 */
#endif
    OSASM(" ldr r3, =uxCriticalNesting                      \t\n");
    OSASM(" add r3, r3, r2                                  \t\n");
    OSASM(" mrs r0, spsr                                    \t\n"); /* unsolicited (SPSR) to R2 */
    OSASM(" ldr r3, [r3]                                    \t\n"); /* get critical */
    OSASM(" stmfd r12!, {r0, r3}                            \t\n"); /* write type of yield, and critical to user task stack */
    OSASM(" str r12, [r1]                                   \t\n"); /* save task stack pointer to old task TCB */
    OSASM(" push {r2}                                       \t\n"); /* core ID */
    OSASM(" 1:                                              \t\n"); /* prepare Call user handler */
#if !defined(_S32DS_FREERTOS_)
    OSASM(" ldr r0, =__VECTOR_RAM                           \t\n");
#endif
    OSASM(" mrs r1, LR_svc                                  \t\n");
    OSASM(" mrs r12, SPSR_svc                               \t\n");
    OSASM(" mrs r3, SPSR                                    \t\n");
    OSASM(" push {r1, r3, r12, r14}                         \t\n"); /* save: LR_svc, SPSR_irq, SPSR_svc and the interrupt ID */
#if configENABLE_FPU == 1
    OSASM(" fmrx    r2, FPEXC                               \t\n");
    OSASM(" and     r2, r2, #~(1 << 30)                     \t\n");
    OSASM(" fmxr    FPEXC, r2                               \t\n"); /* Disable FPU */
    OSASM(" ldr     r3, =new_global_irq_fpu_stack           \t\n");
    OSASM(" ldr     r1, [r3]                                \t\n"); /* get value of new_global_irq_fpu_stack */
    OSASM(" ldr     r2, =ulPortInterruptHasFPUContext       \t\n");
    OSASM(" ldr     r2, [r2]                                \t\n"); /* get value of ulPortInterruptHasFPUContext */
    OSASM(" push    {r1, r2}                                \t\n"); /* save these to IRQ stack */
    OSASM(" sub     r1, r1, %0\t\n"::"i" (portSIZE_OF_STACK_FPU):);
    OSASM(" str     r1, [r3]                                \t\n"); /* set new irq_fpu stack */
#endif /* configENABLE_FPU */
#if defined(_S32DS_FREERTOS_)
    OSASM("ldr  r1, =R52VtableRefArray                      \t\n"); /* Get VTABLE list */
    OSASM("add  r1, r1, r2                                  \t\n"); /* Add core offset r2 = coreID << 2 */
    OSASM("ldr  r0, [r1]                                    \t\n"); /* Get VTABLE */
#else
    OSASM(" cmp r14, #32                                    \t\n"); /* PPI, SGI */
    OSASM(" bcs __spi_irq_os                                \t\n");
    OSASM(" cmp r2, #0                                      \t\n"); /* r2 == (coreID << 2) */
    OSASM(" beq __spi_irq_os                                \t\n");
    OSASM(" sub r2, r2, #4                                  \t\n");

    OSASM(" lsl r2, r2, #5                                  \t\n");
    OSASM(" add r0, r0, %0                                  \t\n" : : "i"(configGIC_INTERRUPTS << 2));
    OSASM(" add r0, r0, r2                                  \t\n");
    OSASM(" __spi_irq_os:                                   \t\n");
#endif
    OSASM(" ldr r2, [r0, r14, lsl #2]                       \t\n");
    OSASM(" cpsie i, #0x13                                  \t\n"); /* enable interrupts, go to svc mode */
    OSASM(" cmp r2, #0                                      \t\n");
#if defined(__thumb__)
    OSASM(" it ne                                           \t\n");
#endif
    OSASM(" blxne r2                                        \t\n"); /* call user interrupt handler */
    OSASM(" cpsid i, #0x12                                  \t\n"); /* disable interrupts, enter in IRQ mode */
#if configENABLE_FPU == 1
    OSASM(" ldr     r1, =ulPortInterruptHasFPUContext       \t\n");
    OSASM(" ldr     r2, [r1]                                \t\n");
    OSASM(" pop     {r3, r14}                               \t\n"); /* restore: new_global_irq_fpu_stack, local_ulPortInterruptHasFPUContext */
    OSASM(" cmp     r14, r2                                 \t\n"); /* compare local_ulPortInterruptHasFPUContext with ulPortInterruptHasFPUContext */
    OSASM(" bge     1f                                      \t\n");
    /* local_ulPortInterruptHasFPUContext < ulPortInterruptHasFPUContext :
     * the irq uses fpu*/
    OSASM(" sub     r2, r2, #1                              \t\n"); /* ulPortInterruptHasFPUContext -- */
    OSASM(" str     r2, [r1]                                \t\n");
    OSASM(" cmp     r2, #0                                  \t\n"); /* compare ulPortInterruptHasFPUContext with NULL */
    OSASM(" beq     2f                                      \t\n");
    /* ulPortInterruptHasFPUContext != 0 : this is not the last IRQ
     * in the nested interrupt */
    OSASM(" ldr     r2, =new_global_irq_fpu_stack           \t\n");
    OSASM(" ldr     r2, [r2]                                \t\n");
    OSASM(" ldr     r1, =global_irq_fpu_stack               \t\n");
    OSASM(" str     r2, [r1]                                \t\n"); /* global_irq_fpu_stack <= new_global_irq_fpu_stack */
    OSASM(" ldmdb   r2!, {r1}                               \t\n");  /* restore fpu from new_global_irq_fpu_stack */
    OSASM(" vldmdb  r2!, {d0-d15}                           \t\n");
    OSASM(" vldmdb  r2!, {d16-d31}                          \t\n");
    OSASM(" ldmdb   r2!, {r1}                               \t\n"); /* restore: FPSCR, FPEXC */
    OSASM(" fmxr    FPSCR, r1                               \t\n");
    OSASM(" ldmdb   r2!, {r1}                               \t\n");
    OSASM(" fmxr    FPEXC, r1                               \t\n");
    OSASM(" ldr     r2, =new_global_irq_fpu_stack           \t\n");
    OSASM(" str     r3, [r2]                                \t\n"); /* restore: new_global_irq_fpu_stack */
    /* ulPortInterruptHasFPUContext == 0 : this is the last IRQ use fpu */
    OSASM(" 2:                                              \t\n");
    OSASM(" fmrx    r2, FPEXC                               \t\n"); /* Disable FPU */
    OSASM(" and     r2, r2, #~(1 << 30)                     \t\n");
    OSASM(" fmxr    FPEXC, r2                               \t\n");
    /* local_ulPortInterruptHasFPUContext == ulPortInterruptHasFPUContext :
     * the irq not uses fpu */
    OSASM(" 1:                                              \t\n");
    OSASM(" ldr     r1, =uxInterruptNested                  \t\n");
    OSASM(" ldr     r1, [r1]                                \t\n");
    OSASM(" cmp     r1, #1                                  \t\n"); /* compare uxInterruptNested with 1 */
    OSASM(" bne     3f                                      \t\n");
    /* uxInterruptNested == 1 : This is the last IRQ
     * reset before return to task. Prepare for another interrupt */
    OSASM(" ldr     r1, =global_irq_fpu_stack               \t\n");
    OSASM(" ldr     r2, =new_global_irq_fpu_stack           \t\n");
    OSASM(" mov     r3, #0                                  \t\n");
    OSASM(" str     r3, [r1]                                \t\n"); /* reset global irq_fpu stack */
    OSASM(" str     r3, [r2]                                \t\n"); /* reset new irq_fpu stack */
    OSASM(" mov     r0, #0                                  \t\n"); /* first parameter 0 */
    OSASM(" mov     r1, #0                                  \t\n"); /* Second parameter 1 */
    OSASM(" bl      pvTaskGetThreadLocalStoragePointer      \t\n"); /* Call function */
    OSASM(" mov     r3, r0                                  \t\n"); /* Save result to R3 */
    OSASM(" ldr     r0, =prevFpuCtx                         \t\n");
    OSASM(" ldr     r1, [r0]                                \t\n"); /* Get prevFpuCtx */
    OSASM(" cmp     r1, r3                                  \t\n"); /* Solicited for switched out task */
    OSASM(" beq     3f                                      \t\n");
    /* newFpuCtx != prevFpuCtx : the switching occurs */
    /* unsolicited for switched out task,
     * buffer to save status, registers d0-d31 */
    OSASM(" ldr     r2, =b_SwitchByIRQ                      \t\n");
    OSASM(" ldr     r1, [r2]                                \t\n"); /* get boolean check switch by IRQ or Yield: b_SwitchByIRQ */
    OSASM(" mov     r1, #1                                  \t\n"); /* b_SwitchByIRQ = 1 */
    OSASM(" str     r1, [r2]                                \t\n");
    OSASM(" 3:                                              \t\n");
#endif /* configENABLE_FPU */
    OSASM(" pop {r1, r2, r3, r14}                           \t\n"); /* restore: LR_svc, SPSR_irq, SPSR_svc and the interrupt ID */
    OSASM(" ldr r12, =uxInterruptNested                     \t\n");
    OSASM(" msr LR_svc, r1                                  \t\n");
    OSASM(" msr SPSR_svc, r3                                \t\n");
    OSASM(" msr SPSR_fsxc, r2                               \t\n");
    OSASM(" mcr p15, 0, r14, c12, c12, 1                    \t\n"); /* End Of Interrupt */
    OSASM(" dsb                                             \t\n");
    OSASM(" pop {r0}                                        \t\n"); /* get core id from stack */
    OSASM(" add r12, r12, r0                                \t\n"); /* uxInterruptNested indexed by core ID */
    OSASM(" ldr r3, [r12]                                   \t\n");
    OSASM(" subs r3, r3, #1                                 \t\n"); /* dec uxInterruptNested */
    OSASM(" str r3, [r12]                                   \t\n"); /* save uxInterruptNested */
#if defined(__thumb__)
    OSASM(" beq 2f                                          \t\n");
    OSASM(" ldmfd sp!, {r0-r3, r12, r14}                    \t\n"); /* return from irq uxInterruptNested != 0 */
    OSASM(" subs pc, lr, #4                                 \t\n"); /* return from irq uxInterruptNested != 0 */
    OSASM(" 2:                                              \t\n");
#else
    OSASM(" ldmnefd sp!, {r0-r3, r12, r14}                  \t\n"); /* return from irq uxInterruptNested != 0 */
    OSASM(" subnes pc, lr, #4                               \t\n"); /* return from irq uxInterruptNested != 0 */
#endif
    OSASM(" ldr r14, =pxCurrentTCBs                         \t\n");
    OSASM(" add r14, r14, r0                                \t\n"); /* indexed by core ID */
    OSASM(" ldr r14, [r14]                                  \t\n");
#if (configENABLE_MPU == 1)
    OSASM(" ldr r1, [r14, %0]                               \t\n" : : "i"((sizeof(MPURegionSettings_t) << 2) + 4) : ); /* R1 xMPU_SETTINGS->ulTaskFlags */
    OSASM(" mov r2, #4                                      \t\n");
    OSASM(" mov r3, %0                                      \t\n" : : "i"(portTOTAL_NUM_REGIONS) : );
    OSASM("3:");
    OSASM(" ldr r1, [r14, r2]                               \t\n");
    OSASM(" cmp r1, #0                                      \t\n");
    OSASM(" beq 2f                                          \t\n");
    OSASM(" mcr p15, 0, r1, c6, c2, 1                       \t\n"); /* select the descriptor */
    OSASM(" mov r5, #0                                      \t\n");
    OSASM(" mcr p15, 0, r5, c6, c3, 1                       \t\n"); /* write the limit, this disable the descriptor */
    OSASM(" dsb                                             \t\n");
    OSASM(" isb                                             \t\n");
    OSASM("add r12, r2, #4                                  \t\n");
    OSASM("ldr r4, [r12, r14]                               \t\n");
    OSASM("add r12, r2, #8                                  \t\n");
    OSASM("ldr r5, [r12, r14]                               \t\n");
    OSASM(" b 4f                                            \t\n");
    OSASM("2:");
    OSASM(" mov r5, r1                                      \t\n");
    OSASM(" mov r4, r1                                      \t\n");
    OSASM(" add r1, r3, %0                                  \t\n" : : "i"(portMPU_TASKSTACK_DSC_IDX + portTOTAL_NUM_REGIONS) : );
    OSASM(" sub r1, r1, r3                                  \t\n");
    OSASM("4:");
    OSASM(" mcr p15, 0, r1, c6, c2, 1                       \t\n"); /* select the descriptor */
    OSASM(" mcr p15, 0, r4, c6, c3, 0                       \t\n"); /* write the base */
    OSASM(" mcr p15, 0, r5, c6, c3, 1                       \t\n"); /* write the limit, this enable the descriptor */
    OSASM(" dsb                                             \t\n");
    OSASM(" isb                                             \t\n");

    OSASM("add r2, r2, %0                                   \t\n" : : "i"(sizeof(MPURegionSettings_t)): );
    OSASM("subs r3, r3, #1                                  \t\n");
    OSASM(" bne 3b                                          \t\n");

#endif
    OSASM(" ldr r1, [r14]                                   \t\n"); /* get new task stack */
    OSASM(" ldmfd r1!, {r2-r4}                              \t\n"); /* get switch type (0 or CPSR), critical, intmask */
    OSASM(" mcr p15, 0, r4, c4, c6, 0                       \t\n"); /* Write r4 into ICC_PMR, task mask level */
    OSASM(" ldr r4, =uxCriticalNesting                      \t\n");
    OSASM(" add r4, r4, r0                                  \t\n");
    OSASM(" str r3, [r4]                                    \t\n"); /* store critical */
    OSASM(" ldmfd r1!, {r4-r11, r14}                        \t\n"); /* Get saved regs + r14_usr to r14_irq */
    OSASM(" cmp r2, #0                                      \t\n"); /* is solicited? solicited yield does not need a specific CPSR so, it can use the previous CPSR/SPSR */
#if defined(__thumb__)
    OSASM(" bne 2f                                          \t\n");
    OSASM(" msr SP_usr, r1                                  \t\n"); /* solicited, update user stack */
    OSASM(" movw r2, 0x13f                                  \t\n");
    OSASM(" msr spsr_fsxc, r2                               \t\n"); /* in IRQ mode */
    OSASM(" movs pc, lr                                     \t\n"); /* exit from interrupt, solicited return, no need to subs */
    OSASM(" 2:                                              \t\n");
#else
    OSASM(" msreq SP_usr, r1                                \t\n"); /* solicited, update user stack */
    OSASM(" moveqs pc, lr                                   \t\n"); /* exit from interrupt, solicited return, no need to subs */
#endif
    OSASM(" msr spsr_fsxc, r2                               \t\n"); /* save spsr, unsolicited */
    OSASM(" mov r0, r1                                      \t\n");
    OSASM(" add r1, r1, #24                                 \t\n");
    OSASM(" msr SP_usr, r1                                  \t\n"); /* update user stack register */
    OSASM(" msr LR_usr, r14                                 \t\n"); /* update user link register */
    OSASM(" add r0, r0, #4                                  \t\n");
    OSASM(" ldmfd    r0!, {r1-r3, r12, r14}                 \t\n"); /* save r14_irq to user stack, */
    OSASM(" ldr r0, [r0, #-24]                              \t\n");
    OSASM(" subs pc, lr, #4                                 \t\n"); /* exit from interrupt */
}
#else /* configNUMBER_OF_CORES > 1 */
void vPortInterruptDispatcher(void)
{
    OSASM(" push {r3, r12, r14}                             \t\n"); /* save r12, r14 to interrupt stack */
    OSASM(" mrc p15, 0, r14, c12, c12, 0                    \t\n"); /* get interrupt id, r14 */
    OSASM(" dsb                                             \t\n");
    OSASM(" movw r12, #1020                                 \t\n"); /* check spurious */
    OSASM(" cmp r14, r12                                    \t\n"); /* check spurious */
    OSASM(" blt 1f                                          \t\n");
    OSASM(" mcr p15, 0, r14, c12, c12, 1                    \t\n"); /* End Of Interrupt */
    OSASM(" dsb                                             \t\n");
    OSASM(" pop {r3, r12, r14}                              \t\n"); /* restore r12, r14 to interrupt stack */
    OSASM(" subs pc, lr, #4                                 \t\n"); /* return from irq */
    OSASM(" 1:");
    OSASM(" mrc p15, 0, r12, c12, c11, 3                    \t\n"); /* get interrupt priority, r12 */
    OSASM(" dsb                                             \t\n");
    OSASM(" cmp r12,%0\t\n"::"i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY ):);
    OSASM(" bge osInt                                       \t\n"); /* OS interrupt */
    OSASM(" stmfd sp!, {r0-r2}                              \t\n"); /* save volatiles to IRQ stack, r12 & r14 already saved */
#if defined(_S32DS_FREERTOS_)
#if defined (S32Z2XX) || defined (S32E2XX)
    OSASM("ldr  r1, =0x419A0004                            \t\n");  /* GetCoreID MSCM_CPXNUM_ADDR */
    OSASM("ldr  r2, [r1]                                   \t\n");
    OSASM("ldr  r1, =VTABLE                                \t\n");  /* Get VTABLE list */
	OSASM("add  r1, r1, r2, lsl #2                         \t\n");  /* Add core offset */
	OSASM("ldr  r0, [r1]                                   \t\n");  /* Get VTABLE */
#elif defined (S32N55) || defined (CPU_S32K566) || defined (CPU_S32N79)
    OSASM(" push {r1,r2,r4}                                \t\n");
    OSASM("mrc  p15, 0, r4, c0, c0, 5                      \t\n"); /* Corresponds to MPIDR. Each RTU has two clusters and each cluster has 2 cores */
    OSASM("and  r2, r4, #0xFF                              \t\n");/* Aff0 */
    OSASM("lsr  r4, r4, #8                                 \t\n"); /* Aff1 */
    OSASM("and  r4, r4, #0xFF                              \t\n");
    OSASM("add  r4, r2, r4, lsl #1                         \t\n"); /* CoreID = Aff0 + (Aff1*2) */
    OSASM("mov  r2,r4                                      \t\n");
    OSASM("ldr  r1, =R52VtableRefArray                     \t\n"); /* Get VTABLE list */
    OSASM("add  r1, r1, r2, lsl #2                         \t\n"); /* Add core offset */
    OSASM("ldr  r0, [r1]                                   \t\n"); /* Get VTABLE */
    OSASM("pop {r1,r2,r4}                                  \t\n");
#else
    OSASM(" ldr r0, =VTABLE                                 \t\n");
#endif
#else
    OSASM(" ldr r0, =__VECTOR_RAM                           \t\n");
#endif
    OSASM(" mrs r1, LR_svc                                  \t\n");
    OSASM(" mrs r2, SPSR_svc                                \t\n");
    OSASM(" mrs r3, SPSR                                    \t\n");
    OSASM(" push {r1, r2, r3, r14}                          \t\n"); /* save: LR_svc, SPSR_svc, SPSR_irq and the interrupt ID */

#if configENABLE_FPU == 1
    OSASM(" fmrx    r2, FPEXC                               \t\n");
    OSASM(" and     r2, r2, #~(1 << 30)                     \t\n");
    OSASM(" fmxr    FPEXC, r2                               \t\n"); /* Disable FPU */
    OSASM(" ldr     r3, =new_global_irq_fpu_stack           \t\n");
    OSASM(" ldr     r1, [r3]                                \t\n"); /* get value of new_global_irq_fpu_stack */
    OSASM(" ldr     r2, =ulPortInterruptHasFPUContext       \t\n");
    OSASM(" ldr     r2, [r2]                                \t\n"); /* get value of ulPortInterruptHasFPUContext */
    OSASM(" push    {r1, r2}                                \t\n"); /* save these to IRQ stack */
    OSASM(" sub     r1, r1, %0\t\n"::"i" (portSIZE_OF_STACK_FPU):);
    OSASM(" str     r1, [r3]                                \t\n"); /* set new irq_fpu stack */
#endif /* configENABLE_FPU */

    OSASM(" ldr r2, [r0, r14, lsl #2]                       \t\n"); /* get user interrupt vector */
    OSASM(" cpsie i, #0x13                                  \t\n"); /* enable interrupts and enter to svc mode */
    OSASM(" cmp r2, #0                                      \t\n");
#if defined(__thumb__)
    OSASM(" it ne                                           \t\n");
#endif
    OSASM(" blxne r2                                        \t\n"); /* call user interrupt handler */
    OSASM(" cpsid i, 0x12                                   \t\n"); /* disable interrupts and enter to irq mode */

#if configENABLE_FPU == 1
    OSASM(" ldr     r1, =ulPortInterruptHasFPUContext       \t\n");
    OSASM(" ldr     r2, [r1]                                \t\n");
    OSASM(" pop     {r3, r14}                               \t\n"); /* restore: new_global_irq_fpu_stack, local_ulPortInterruptHasFPUContext */
    OSASM(" cmp     r14, r2                                 \t\n"); /* compare local_ulPortInterruptHasFPUContext with ulPortInterruptHasFPUContext */
    OSASM(" bge     1f                                      \t\n");
    /* local_ulPortInterruptHasFPUContext < ulPortInterruptHasFPUContext :
     * the irq uses fpu*/
    OSASM(" sub     r2, r2, #1                              \t\n"); /* ulPortInterruptHasFPUContext -- */
    OSASM(" str     r2, [r1]                                \t\n");
    OSASM(" cmp     r2, #0                                  \t\n"); /* compare ulPortInterruptHasFPUContext with NULL */
    OSASM(" beq     2f                                      \t\n");
    /* ulPortInterruptHasFPUContext != 0 : this is not the last IRQ
     * in the nested interrupt */
    OSASM(" ldr     r2, =new_global_irq_fpu_stack           \t\n");
    OSASM(" ldr     r2, [r2]                                \t\n");
    OSASM(" ldr     r1, =global_irq_fpu_stack               \t\n");
    OSASM(" str     r2, [r1]                                \t\n"); /* global_irq_fpu_stack <= new_global_irq_fpu_stack */

    OSASM(" ldmdb   r2!, {r1}                               \t\n");  /* restore fpu from new_global_irq_fpu_stack */
    OSASM(" vldmdb  r2!, {d0-d15}                           \t\n");
    OSASM(" vldmdb  r2!, {d16-d31}                          \t\n");
    OSASM(" ldmdb   r2!, {r1}                               \t\n"); /* restore: FPSCR, FPEXC */
    OSASM(" fmxr    FPSCR, r1                               \t\n");
    OSASM(" ldmdb   r2!, {r1}                               \t\n");
    OSASM(" fmxr    FPEXC, r1                               \t\n");

    OSASM(" ldr     r2, =new_global_irq_fpu_stack           \t\n");
    OSASM(" str     r3, [r2]                                \t\n"); /* restore: new_global_irq_fpu_stack */
    /* ulPortInterruptHasFPUContext == 0 : this is the last IRQ use fpu */
    OSASM(" 2:                                              \t\n");
    OSASM(" fmrx    r2, FPEXC                               \t\n"); /* Disable FPU */
    OSASM(" and     r2, r2, #~(1 << 30)                     \t\n");
    OSASM(" fmxr    FPEXC, r2                               \t\n");
    /* local_ulPortInterruptHasFPUContext == ulPortInterruptHasFPUContext :
     * the irq not uses fpu */
    OSASM(" 1:                                              \t\n");
    OSASM(" ldr     r1, =uxInterruptNested                  \t\n");
    OSASM(" ldr     r1, [r1]                                \t\n");
    OSASM(" cmp     r1, #1                                  \t\n"); /* compare uxInterruptNested with 1 */
    OSASM(" bne     3f                                      \t\n");
    /* uxInterruptNested == 1 : This is the last IRQ
     * reset before return to task. Prepare for another interrupt */
    OSASM(" ldr     r1, =global_irq_fpu_stack               \t\n");
    OSASM(" ldr     r2, =new_global_irq_fpu_stack           \t\n");
    OSASM(" mov     r3, #0                                  \t\n");
    OSASM(" str     r3, [r1]                                \t\n"); /* reset global irq_fpu stack */
    OSASM(" str     r3, [r2]                                \t\n"); /* reset new irq_fpu stack */

    OSASM(" mov     r0, #0                                  \t\n"); /* first parameter 0 */
    OSASM(" mov     r1, #0                                  \t\n"); /* Second parameter 1 */
    OSASM(" bl      pvTaskGetThreadLocalStoragePointer      \t\n"); /* Call function */
    OSASM(" mov     r3, r0                                  \t\n"); /* Save result to R3 */
    OSASM(" ldr     r0, =prevFpuCtx                         \t\n");
    OSASM(" ldr     r1, [r0]                                \t\n"); /* Get prevFpuCtx */
    OSASM(" cmp     r1, r3                                  \t\n"); /* Solicited for switched out task */
    OSASM(" beq     3f                                      \t\n");
    /* newFpuCtx != prevFpuCtx : the switching occurs */
    /* unsolicited for switched out task,
     * buffer to save status, registers d0-d31 */
    OSASM(" ldr     r2, =b_SwitchByIRQ                      \t\n");
    OSASM(" ldr     r1, [r2]                                \t\n"); /* get boolean check switch by IRQ or Yield: b_SwitchByIRQ */
    OSASM(" mov     r1, #1                                  \t\n"); /* b_SwitchByIRQ = 1 */
    OSASM(" str     r1, [r2]                                \t\n");
    OSASM(" 3:                                              \t\n");
#endif /* configENABLE_FPU */
    OSASM(" pop {r1, r2, r3, r14}                           \t\n"); /* restore: LR_svc, SPSR_svc, SPSR_irq and the interrupt ID */
    OSASM(" msr LR_svc, r1                                  \t\n");
    OSASM(" msr SPSR_svc, r2                                \t\n");
    OSASM(" msr SPSR_fsxc, r3                               \t\n");
    OSASM(" mcr p15, 0, r14, c12, c12, 1                    \t\n"); /* End Of Interrupt */
    OSASM(" dsb                                             \t\n");
    OSASM(" ldmfd sp!, {r0-r3, r12, r14}                    \t\n"); /* restore user registers */
    OSASM(" subs pc, lr, #4                                 \t\n"); /* return from irq */
    OSASM(" osInt:                                          \t\n"); /* OS interrupt handler */
    OSASM(" ldr r12, =uxInterruptNested                     \t\n");
    OSASM(" ldr r3, [r12]                                   \t\n");
    OSASM(" cmp r3, #0                                      \t\n");
    OSASM(" add r3, r3, #1                                  \t\n"); /* inc uxInterruptNested */
    OSASM(" str r3, [r12]                                   \t\n"); /* save uxInterruptNested */
#if defined(__thumb__)
    OSASM(" beq 2f                                          \t\n");
    OSASM(" stmdb sp!, {r0-r2}                              \t\n"); /* uxInterruptNested != 0, using irq stack */
    OSASM(" b 1f                                            \t\n"); /* jump to "prepare Call user handler" */
    OSASM(" 2:                                              \t\n");
#else
    OSASM(" stmnedb sp!, {r0-r2}                            \t\n"); /* uxInterruptNested != 0, using irq stack */
    OSASM(" bne 1f                                          \t\n"); /* jump to "prepare Call user handler" */
#endif

    OSASM(" mrs r12, SP_usr                                 \t\n"); /* get user stack to r12 */
    OSASM(" sub r12, r12, #12                               \t\n"); /* space for r3, r12, r14 */
    OSASM(" stmdb    r12!, {r0-r2}                          \t\n"); /* save users regs to user stack, */
    OSASM(" ldmfd sp!, {r0-r2}                              \t\n"); /* get r3 -> r0, r12 -> r1, r14-> r2, unload irq stack */
    OSASM(" add r3, r12, #24                                \t\n");
    OSASM(" stmdb r3, {r0-r2}                               \t\n");
    OSASM(" msr SP_usr, r12                                 \t\n"); /* update user stack, save r12*/
    OSASM(" ldr r1, =pxCurrentTCB                           \t\n");
    OSASM(" ldr r1, [r1]                                    \t\n");
    OSASM(" push {r1}                                       \t\n"); /* current task TCB to IRQ stack */
    OSASM(" 1:                                              \t\n"); /* prepare Call user handler */
#if defined(_S32DS_FREERTOS_)
#if defined (S32Z2XX) || defined (S32E2XX)
    OSASM("ldr  r1, =0x419A0004                            \t\n");  /* GetCoreID MSCM_CPXNUM_ADDR */
    OSASM("ldr  r2, [r1]                                   \t\n");
    OSASM("ldr  r1, =VTABLE                                \t\n");  /* Get VTABLE list */
	OSASM("add  r1, r1, r2, lsl #2                         \t\n");  /* Add core offset */
	OSASM("ldr  r0, [r1]                                   \t\n");  /* Get VTABLE */
#elif defined (S32N55) || defined (CPU_S32K566) || defined (CPU_S32N79)
    OSASM("push {r1,r2,r4}                                 \t\n");
    OSASM("mrc  p15, 0, r4, c0, c0, 5                      \t\n"); /* Corresponds to MPIDR. Each RTU has two clusters and each cluster has 2 cores */
    OSASM("and  r2, r4, #0xFF                              \t\n");/* Aff0 */
    OSASM("lsr  r4, r4, #8                                 \t\n"); /* Aff1 */
    OSASM("and  r4, r4, #0xFF                              \t\n");
    OSASM("add  r4, r2, r4, lsl #1                         \t\n"); /* CoreID = Aff0 + (Aff1*2) */
    OSASM("mov  r2,r4                                      \t\n");
    OSASM("ldr  r1, =R52VtableRefArray                     \t\n"); /* Get VTABLE list */
    OSASM("add  r1, r1, r2, lsl #2                         \t\n"); /* Add core offset */
    OSASM("ldr  r0, [r1]                                   \t\n"); /* Get VTABLE */
    OSASM("pop {r1,r2,r4}                                  \t\n");
#else
    OSASM(" ldr r0, =VTABLE                                 \t\n");
#endif
#else
    OSASM(" ldr r0, =__VECTOR_RAM                           \t\n");
#endif
    OSASM(" mrs r1, LR_svc                                  \t\n");
    OSASM(" mrs r2, SPSR_svc                                \t\n");
    OSASM(" mrs r3, SPSR                                    \t\n");
    OSASM(" push {r1, r2, r3, r14}                          \t\n"); /* save: LR_svc, SPSR_svc, SPSR_irq and the interrupt ID */

#if configENABLE_FPU == 1
    OSASM(" fmrx    r2, FPEXC                               \t\n");
    OSASM(" and     r2, r2, #~(1 << 30)                     \t\n");
    OSASM(" fmxr    FPEXC, r2                               \t\n"); /* Disable FPU */
    OSASM(" ldr     r3, =new_global_irq_fpu_stack           \t\n");
    OSASM(" ldr     r1, [r3]                                \t\n"); /* get value of new_global_irq_fpu_stack */
    OSASM(" ldr     r2, =ulPortInterruptHasFPUContext       \t\n");
    OSASM(" ldr     r2, [r2]                                \t\n"); /* get value of ulPortInterruptHasFPUContext */
    OSASM(" push    {r1, r2}                                \t\n"); /* save these to IRQ stack */
    OSASM(" sub     r1, r1, %0\t\n"::"i" (portSIZE_OF_STACK_FPU):);
    OSASM(" str     r1, [r3]                                \t\n"); /* set new irq_fpu stack */
#endif /* configENABLE_FPU */

    OSASM(" ldr r2, [r0, r14, lsl #2]                       \t\n");
    OSASM(" cpsie i, #0x13                                  \t\n"); /* enable interrupts, go to svc mode */
    OSASM(" cmp r2, #0                                      \t\n");
#if defined(__thumb__)
    OSASM(" it ne                                           \t\n");
#endif
    OSASM(" blxne r2                                        \t\n"); /* call user interrupt handler */

    OSASM(" cpsid i, #0x12                                  \t\n"); /* disable interrupts, enter in IRQ mode */

#if configENABLE_FPU == 1
    OSASM(" ldr     r1, =ulPortInterruptHasFPUContext       \t\n");
    OSASM(" ldr     r2, [r1]                                \t\n");
    OSASM(" pop     {r3, r14}                               \t\n"); /* restore: new_global_irq_fpu_stack, local_ulPortInterruptHasFPUContext */
    OSASM(" cmp     r14, r2                                 \t\n"); /* compare local_ulPortInterruptHasFPUContext with ulPortInterruptHasFPUContext */
    OSASM(" bge     1f                                      \t\n");
    /* local_ulPortInterruptHasFPUContext < ulPortInterruptHasFPUContext :
     * the irq uses fpu*/
    OSASM(" sub     r2, r2, #1                              \t\n"); /* ulPortInterruptHasFPUContext -- */
    OSASM(" str     r2, [r1]                                \t\n");
    OSASM(" cmp     r2, #0                                  \t\n"); /* compare ulPortInterruptHasFPUContext with NULL */
    OSASM(" beq     2f                                      \t\n");
    /* ulPortInterruptHasFPUContext != 0 : this is not the last IRQ
     * in the nested interrupt */
    OSASM(" ldr     r2, =new_global_irq_fpu_stack           \t\n");
    OSASM(" ldr     r2, [r2]                                \t\n");
    OSASM(" ldr     r1, =global_irq_fpu_stack               \t\n");
    OSASM(" str     r2, [r1]                                \t\n"); /* global_irq_fpu_stack <= new_global_irq_fpu_stack */

    OSASM(" ldmdb   r2!, {r1}                               \t\n");  /* restore fpu from new_global_irq_fpu_stack */
    OSASM(" vldmdb  r2!, {d0-d15}                           \t\n");
    OSASM(" vldmdb  r2!, {d16-d31}                          \t\n");
    OSASM(" ldmdb   r2!, {r1}                               \t\n"); /* restore: FPSCR, FPEXC */
    OSASM(" fmxr    FPSCR, r1                               \t\n");
    OSASM(" ldmdb   r2!, {r1}                               \t\n");
    OSASM(" fmxr    FPEXC, r1                               \t\n");

    OSASM(" ldr     r2, =new_global_irq_fpu_stack           \t\n");
    OSASM(" str     r3, [r2]                                \t\n"); /* restore: new_global_irq_fpu_stack */
    /* ulPortInterruptHasFPUContext == 0 : this is the last IRQ use fpu */
    OSASM(" 2:                                              \t\n");
    OSASM(" fmrx    r2, FPEXC                               \t\n"); /* Disable FPU */
    OSASM(" and     r2, r2, #~(1 << 30)                     \t\n");
    OSASM(" fmxr    FPEXC, r2                               \t\n");

    /* local_ulPortInterruptHasFPUContext == ulPortInterruptHasFPUContext :
     * the irq not uses fpu */
    OSASM(" 1:                                              \t\n");
    OSASM(" ldr     r1, =uxInterruptNested                  \t\n");
    OSASM(" ldr     r1, [r1]                                \t\n");
    OSASM(" cmp     r1, #1                                  \t\n"); /* compare uxInterruptNested with 1 */
    OSASM(" bne     3f                                      \t\n");
    /* uxInterruptNested == 1 : This is the last IRQ
     * reset before return to task. Prepare for another interrupt */
    OSASM(" ldr     r1, =global_irq_fpu_stack               \t\n");
    OSASM(" ldr     r2, =new_global_irq_fpu_stack           \t\n");
    OSASM(" mov     r3, #0                                  \t\n");
    OSASM(" str     r3, [r1]                                \t\n"); /* reset global irq_fpu stack */
    OSASM(" str     r3, [r2]                                \t\n"); /* reset new irq_fpu stack */

    OSASM(" mov     r0, #0                                  \t\n"); /* first parameter 0 */
    OSASM(" mov     r1, #0                                  \t\n"); /* Second parameter 1 */
    OSASM(" bl      pvTaskGetThreadLocalStoragePointer      \t\n"); /* Call function */
    OSASM(" mov     r3, r0                                  \t\n"); /* Save result to R3 */
    OSASM(" ldr     r0, =prevFpuCtx                         \t\n");
    OSASM(" ldr     r1, [r0]                                \t\n"); /* Get prevFpuCtx */
    OSASM(" cmp     r1, r3                                  \t\n"); /* Solicited for switched out task */
    OSASM(" beq     3f                                      \t\n");
    /* newFpuCtx != prevFpuCtx : the switching occurs */
    /* unsolicited for switched out task,
     * buffer to save status, registers d0-d31 */
    OSASM(" ldr     r2, =b_SwitchByIRQ                      \t\n");
    OSASM(" ldr     r1, [r2]                                \t\n"); /* get boolean check switch by IRQ or Yield: b_SwitchByIRQ */
    OSASM(" mov     r1, #1                                  \t\n"); /* b_SwitchByIRQ = 1 */
    OSASM(" str     r1, [r2]                                \t\n");
    OSASM(" 3:                                              \t\n");
#endif /* configENABLE_FPU */

    OSASM(" pop {r1, r2, r3, r14}                           \t\n"); /* restore: LR_svc, SPSR_svc, SPSR_irq and the interrupt ID */
    OSASM(" ldr r12, =uxInterruptNested                     \t\n");
    OSASM(" msr LR_svc, r1                                  \t\n");
    OSASM(" msr SPSR_svc, r2                                \t\n");
    OSASM(" msr SPSR_fsxc, r3                               \t\n");
    OSASM(" mcr p15, 0, r14, c12, c12, 1                    \t\n"); /* End Of Interrupt */
    OSASM(" dsb                                             \t\n");
    OSASM(" ldr r3, [r12]                                   \t\n");
    OSASM(" subs r3, r3, #1                                 \t\n"); /* dec uxInterruptNested */
    OSASM(" str r3, [r12]                                   \t\n"); /* save uxInterruptNested */
#if defined(__thumb__)
    OSASM(" beq 2f                                          \t\n");
    OSASM(" ldmfd sp!, {r0-r3, r12, r14}                    \t\n"); /* return from irq uxInterruptNested != 0 */
    OSASM(" subs pc, lr, #4                                 \t\n"); /* return from irq uxInterruptNested != 0 */
    OSASM(" 2:                                              \t\n");
#else
    OSASM(" ldmnefd sp!, {r0-r3, r12, r14}                  \t\n"); /* return from irq uxInterruptNested != 0 */
    OSASM(" subnes pc, lr, #4                               \t\n"); /* return from irq uxInterruptNested != 0 */
#endif

    OSASM(" mrs r0, SP_usr                                  \t\n"); /* get user stack to r0 */
    OSASM(" ldr r14, =pxCurrentTCB                          \t\n");
    OSASM(" pop {r1}                                        \t\n"); /* get prev TCB from IRQ stack */
    OSASM(" ldr r14, [r14]                                  \t\n");
    OSASM(" cmp r14, r1                                     \t\n"); /* compare crt TCB with previous TCB */
#if defined(__thumb__)
    OSASM(" bne 2f                                          \t\n");
    OSASM(" add r1, r0, #24                                 \t\n"); /* unload user stack, no schedule */
    OSASM(" msr SP_usr, r1                                  \t\n"); /* update user stack, no schedule */
    OSASM(" b nothingToDo                                   \t\n");
    OSASM(" 2:                                              \t\n");
#else
    OSASM(" addeq r1, r0, #24                               \t\n"); /* unload user stack, no schedule */
    OSASM(" msreq SP_usr, r1                                \t\n"); /* update user stack, no schedule */
    OSASM(" beq nothingToDo                                 \t\n");
#endif
    OSASM(" mrc p15, 0, r3, c4, c6, 0                       \t\n"); /* get interrupt mask, r3 */

#if defined(__thumb__)
    OSASM(" sub r0, #4                                      \t\n"); /* Space for LR_USR */
    OSASM(" stmfd r0, {r3-r11}                              \t\n"); /* int_mask + save registers r14_usr, old task */
    OSASM(" mrs r2, LR_usr                                  \t\n"); /* get user LINK to r2 */
    OSASM(" str r2, [r0]                                    \t\n"); /* save to user stack */
    OSASM(" sub r0, r0, #36                                 \t\n"); /* update r0 */
#else
    OSASM(" stmfd r0, {r3-r11, r14}^                        \t\n"); /* int_mask + save registers r14_usr, old task */
    OSASM(" sub r0, r0, #40                                 \t\n"); /* update r0 */
#endif
    OSASM(" ldr r3, =uxCriticalNesting                      \t\n");
    OSASM(" mrs r2, spsr                                    \t\n"); /* unsolicited (SPSR) to R2 */
    OSASM(" ldr r3, [r3]                                    \t\n"); /* get critical */
    OSASM(" stmfd r0!, {r2, r3}                             \t\n"); /* write type of yield, and critical to user task stack */

    OSASM(" str r0, [r1]                                    \t\n"); /* save stack pointer to old task TCB */

#if (configENABLE_MPU == 1)
    OSASM(" ldr r1, [r14, %0]                               \t\n" : : "i"((sizeof(MPURegionSettings_t) << 2) + 4) : ); /* R1 xMPU_SETTINGS->ulTaskFlags */
    OSASM(" mov r2, #4                                      \t\n");
    OSASM(" mov r3, %0                                      \t\n" : : "i"(portTOTAL_NUM_REGIONS) : );
    OSASM("3:");
    OSASM(" ldr r1, [r14, r2]                               \t\n");
    OSASM(" cmp r1, #0                                      \t\n");
    OSASM(" beq 2f                                          \t\n");
    OSASM(" mcr p15, 0, r1, c6, c2, 1                       \t\n"); /* select the descriptor */
    OSASM(" mov r5, #0                                      \t\n");
    OSASM(" mcr p15, 0, r5, c6, c3, 1                       \t\n"); /* write the limit, this disable the descriptor */
    OSASM(" dsb                                             \t\n");
    OSASM(" isb                                             \t\n");
    OSASM("add r12, r2, #4                                  \t\n");
    OSASM("ldr r4, [r12, r14]                               \t\n");
    OSASM("add r12, r2, #8                                  \t\n");
    OSASM("ldr r5, [r12, r14]                               \t\n");
    OSASM(" b 4f                                            \t\n");
    OSASM("2:");
    OSASM(" mov r5, r1                                      \t\n");
    OSASM(" mov r4, r1                                      \t\n");
    OSASM(" add r1, r3, %0                                  \t\n" : : "i"(portMPU_TASKSTACK_DSC_IDX + portTOTAL_NUM_REGIONS) : );
    OSASM(" sub r1, r1, r3                                  \t\n");
    OSASM("4:");
    OSASM(" mcr p15, 0, r1, c6, c2, 1                       \t\n"); /* select the descriptor */
    OSASM(" mcr p15, 0, r4, c6, c3, 0                       \t\n"); /* write the base */
    OSASM(" mcr p15, 0, r5, c6, c3, 1                       \t\n"); /* write the limit, this enable the descriptor */
    OSASM(" dsb                                             \t\n");
    OSASM(" isb                                             \t\n");

    OSASM("add r2, r2, %0                                   \t\n" : : "i"(sizeof(MPURegionSettings_t)): );
    OSASM("subs r3, r3, #1                                  \t\n");
    OSASM(" bne 3b                                          \t\n");

#endif

    OSASM(" ldr r0, [r14]                                   \t\n"); /* get new task stack */

    OSASM(" ldmfd r0!, {r2-r4}                              \t\n"); /* get switch type (0 or CPSR), critical, intmask */
    OSASM(" mcr p15, 0, r4, c4, c6, 0                       \t\n"); /* Write r4 into ICC_PMR, task mask level */
    OSASM(" ldr r4, =uxCriticalNesting                      \t\n");
    OSASM(" str r3, [r4]                                    \t\n"); /* store critical */
    OSASM(" ldmfd r0!, {r4-r11, r14}                        \t\n"); /* Get saved regs + r14_usr to r14_irq */
    OSASM(" cmp r2, #0                                      \t\n"); /* is solicited? solicited yield does not need a specific CPSR so, it can use the previous CPSR/SPSR */
#if defined(__thumb__)
    OSASM(" bne 2f                                          \t\n");
    OSASM(" msr SP_usr, r0                                  \t\n"); /* solicited, update user stack */
    OSASM(" movw r2, 0x13f                                  \t\n");
    OSASM(" msr spsr_fsxc, r2                               \t\n"); /* in IRQ mode */
    OSASM(" movs pc, lr                                     \t\n"); /* exit from interrupt, solicited return, no need to subs */
    OSASM(" 2:                                              \t\n");
#else
    OSASM(" msreq SP_usr, r0                                \t\n"); /* solicited, update user stack */
    OSASM(" moveqs pc, lr                                   \t\n"); /* exit from interrupt, solicited return, no need to subs */
#endif
    OSASM(" msr spsr_fsxc, r2                               \t\n"); /* save spsr, unsolicited */
    OSASM(" add r1, r0, #24                                 \t\n");
    OSASM(" msr SP_usr, r1                                  \t\n"); /* update user stack register */
    OSASM(" msr LR_usr, r14                                 \t\n"); /* update user link register */
    OSASM(" nothingToDo:                                    \t\n");
    OSASM(" add r0, r0, #4                                  \t\n");
    OSASM(" ldmfd    r0!, {r1-r3, r12, r14}                 \t\n"); /* save r14_irq to user stack, */
    OSASM(" ldr r0, [r0, #-24]                              \t\n");
    OSASM(" subs pc, lr, #4                                 \t\n"); /* exit from interrupt */
}
#endif
void xPortSysTickHandler( void ) PRIVILEGED_FUNCTION;
void xPortSysTickHandler( void )
{
    BaseType_t doSwitch = pdFALSE;
#if ( configNUMBER_OF_CORES > 1 )
    uint32_t ulPreviousMask;
#endif
    vUpdateTimer();
#if ( configNUMBER_OF_CORES > 1 )
    ulPreviousMask = taskENTER_CRITICAL_FROM_ISR();
#else
    portDISABLE_INTERRUPTS();
#endif
    doSwitch = xTaskIncrementTick();
#if ( configNUMBER_OF_CORES > 1 )
    taskEXIT_CRITICAL_FROM_ISR( ulPreviousMask );
#else
    portENABLE_INTERRUPTS();
#endif
    portYIELD_FROM_ISR(doSwitch);
}

extern void SVC_Handler(unsigned long* info);
void vPortSVCDispatcher(void) __attribute__ (( naked ))
#if defined(_S32DS_FREERTOS_)
    __attribute__ ((section (".systeminit")));
#else
    __attribute__((section(".handlers")));
#endif

void vPortSVCDispatcher(void)
{
    OSASM(" stmfd r13!, {r0-r3, r12, r14}   \t\n"); /* save r0 - r3, r12, r14 to SVC stack */
    OSASM(" mrc p15, 0, r0, c4, c6, 0       \t\n"); /* get interrupt priority, r0 */
    OSASM(" mov r12,%0\t\n"::"i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY ):);
    OSASM(" push {r0}                       \t\n"); /* save current interrupt mask */
    OSASM(" mcr p15, 0, r12, c4, c6, 0      \t\n"); /* do not allow OS interrupts to execute during SVC */
    OSASM(" dsb                             \t\n");
    OSASM(" add r0, r13, #4                 \t\n");
    OSASM(" ldr r3, =SVC_Handler            \t\n");
    OSASM(" cmp r3, #0                      \t\n");
    OSASM(" cpsie i                         \t\n"); /* enable interrupts */
#if defined(__thumb__)
    OSASM(" it ne                           \t\n"); /* conditional prefix */
#endif
    OSASM(" blxne r3                        \t\n"); /* call user svc handler */
    OSASM(" cpsid i                         \t\n"); /* disable interrupts */
    OSASM(" pop {r0}                        \t\n"); /* get previous saved interrupt mask */
    OSASM(" mcr p15, 0, r0, c4, c6, 0       \t\n"); /* restore mask */
#if defined(__thumb__)
    OSASM(" ldmfd r13!, {r0-r3, r12, r14}   \t\n"); /* restore r0 - r3, r12 from SVC stack */
    OSASM(" movs pc, lr                     \t\n");
#else
    OSASM(" ldmfd r13!, {r0-r3, r12, pc}^   \t\n"); /* restore r0 - r3, r12, r15 from SVC stack */
#endif
}

#if ( configNUMBER_OF_CORES > 1 )

/* Index 0 is used for ISR lock and Index 1 is used for task lock */
__attribute__ ((aligned (4))) volatile uint32_t PRIVILEGED_DATA ulPortGateWord[ portRTOS_LOCK_COUNT ];
/* Which core owns the lock */
__attribute__ ((aligned (4))) volatile uint32_t PRIVILEGED_DATA ulPortOwnedByCore[ configNUMBER_OF_CORES ];
/* Lock count a core owns */
__attribute__ ((aligned (4))) volatile  uint32_t PRIVILEGED_DATA ulPortRecursionCountByLock[ portRTOS_LOCK_COUNT ];

__attribute__ ((aligned (4))) volatile uint32_t PRIVILEGED_DATA ulPortCntW[ configNUMBER_OF_CORES ];
__attribute__ ((aligned (4))) volatile uint32_t PRIVILEGED_DATA ulPortCntR[ configNUMBER_OF_CORES ];

void vYieldCore( int xCoreID ) PRIVILEGED_FUNCTION;
void vYieldCore( int xCoreID )
{
    ulPortCntW[xCoreID] ++;

    MRUMB_Set(CORE_ID_2_MRU_ID(xCoreID), CFG_MRU_CHANNEL_SMP, CFG_MRU_MB_ID_SMP, CFG_MRU_MB_VAL_SMP);
}

void vCoreSignal(uint32_t sigId, uint32_t coreId) PRIVILEGED_FUNCTION;
void vCoreSignal(uint32_t sigId, uint32_t coreId)
{
    if (CFG_MRU_MB_VAL_SMP == sigId)
    {
        while (ulPortCntW[coreId] != ulPortCntR[coreId])
        {
            ulPortCntR[coreId] ++;
            portYIELD_FROM_ISR(1);
        }
    }
}

#endif
