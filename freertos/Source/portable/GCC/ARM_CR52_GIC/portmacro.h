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

#ifndef PORTMACRO_H
#define PORTMACRO_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if (configENABLE_MPU == 1)
#define portUSING_MPU_WRAPPERS          1
#define portPRIVILEGE_BIT               (1 << 31)

#ifdef configMPU_NUM_CONFIGURABLE_REGIONS
#define portNUM_CONFIGURABLE_REGIONS configMPU_NUM_CONFIGURABLE_REGIONS
#else
#define portNUM_CONFIGURABLE_REGIONS 3
#endif

#define portTOTAL_NUM_REGIONS (portNUM_CONFIGURABLE_REGIONS + 1)

/* first dsc used by task */
#ifdef configMPU_TASK_STACK_DSC_IDX
#define portMPU_TASKSTACK_DSC_IDX configMPU_TASK_STACK_DSC_IDX
#else
#define portMPU_TASKSTACK_DSC_IDX 7
#endif

#define portMPU_XN                                  (1UL << 0UL)
#define portMPU_RW_EL1                              (0UL << 1UL)
#define portMPU_RW_EL1_EL0                          (1UL << 1UL)
#define portMPU_RO_EL1                              (2UL << 1UL)
#define portMPU_RO_EL1_EL0                          (3UL << 1UL)
#define portMPU_EN                                  (1UL << 0UL)

extern long xPortIsTaskPrivileged( void );

/**
 * @brief Checks whether or not the calling task is privileged.
 *
 * @return pdTRUE if the calling task is privileged, pdFALSE otherwise.
 */
#define portIS_TASK_PRIVILEGED()    xPortIsTaskPrivileged()

/**
 * @brief Settings to define an MPU region.
 */
typedef struct MPURegionSettings
{
    uint32_t ulIndex;
    uint32_t ulBAR; /**< MPU Region Base Address Register (RBAR) for the region. */
    uint32_t ulLAR; /**< MPU Region Limit Address Register the region. */
} MPURegionSettings_t;

#if ( configUSE_MPU_WRAPPERS_V1 == 0 )

    #ifndef configSYSTEM_CALL_STACK_SIZE
        #error configSYSTEM_CALL_STACK_SIZE must be defined to the desired size of the system call stack in words for using MPU wrappers v2.
    #endif

    /**
     * @brief System call stack.
     */
    typedef struct SYSTEM_CALL_STACK_INFO
    {
        uint32_t ulFlags;
        uint32_t ulLr;
    } xSYSTEM_CALL_STACK_INFO;

#endif /* configUSE_MPU_WRAPPERS_V1 == 0 */

#define portMPU_REGION_READ_WRITE               (1UL << 5)
#define portMPU_REGION_PRIVILEGED_READ_ONLY     (1UL << 6)
#define portMPU_REGION_READ_ONLY                (1UL << 7)
#define portMPU_REGION_PRIVILEGED_READ_WRITE    (1UL << 8)
#define portMPU_REGION_CACHEABLE_BUFFERABLE     (1UL << 9)
#define portMPU_REGION_EXECUTE_NEVER            (1UL << 10)

#define portTASK_IS_PRIVILEGED_NORESTRICTED ( 1UL << 0UL )
#define portTASK_IS_PRIVILEGED_FLAG         ( 1UL << 1UL )
typedef struct MPU_SETTINGS
{
    MPURegionSettings_t xRegionsSettings[ portTOTAL_NUM_REGIONS ]; /**< Settings for 4 per task regions. */
    uint32_t ulTaskFlags;

    #if ( configUSE_MPU_WRAPPERS_V1 == 0 )
        xSYSTEM_CALL_STACK_INFO xSystemCallStackInfo;
        #if ( configENABLE_ACCESS_CONTROL_LIST == 1 )
            uint32_t ulAccessControlList[ ( configPROTECTED_KERNEL_OBJECT_POOL_SIZE / portACL_ENTRY_SIZE_BITS ) + 1 ];
        #endif
    #endif
} xMPU_SETTINGS;

#endif

extern void vPortYield(void) __attribute__ (( naked ));
#if ( configNUMBER_OF_CORES > 1 )
extern uint32_t getCoreIndex(void);
#endif
#define OSASM __asm volatile

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR        char
#define portFLOAT       float
#define portDOUBLE      double
#define portLONG        long
#define portSHORT       short
#define portSTACK_TYPE  uint32_t
#define portBASE_TYPE   long
#ifndef configSTACK_DEPTH_TYPE

/* Defaults to uint16_t for backward compatibility, but can be overridden
 * in FreeRTOSConfig.h if uint16_t is too restrictive. */
    #define configSTACK_DEPTH_TYPE    uint16_t
#endif

typedef portSTACK_TYPE StackType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;
typedef struct tskTaskControlBlock * pTaskHandle_t;

#if( configUSE_16_BIT_TICKS == 1 )
    typedef uint16_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffff
#else
    typedef uint32_t TickType_t;
    #define portMAX_DELAY ( TickType_t ) 0xffffffffUL

    /* 32-bit tick type on a 32-bit architecture, so reads of the tick count do
    not need to be guarded with a critical section. */
    #define portTICK_TYPE_IS_ATOMIC 1
#endif
/*-----------------------------------------------------------*/

/* Architecture specifics. */
#define portSTACK_GROWTH            ( -1 )
#define portTICK_PERIOD_MS          ( ( TickType_t ) 1000 / configTICK_RATE_HZ )
#define portBYTE_ALIGNMENT          8
/*-----------------------------------------------------------*/

#define portINLINE  __inline

#ifndef portFORCE_INLINE
    #define portFORCE_INLINE inline __attribute__(( always_inline))
#endif



#define portINTC_GIC_PRIO_BITS                  (5u)
#define portINTC_GIC_PRIO_SHIFT                 (8u - portINTC_GIC_PRIO_BITS)
#define portINTC_GIC_CONVERT_PRIO_SET(prio)     ((((prio) & 0x1Fu) ^ 0x1Fu) << portINTC_GIC_PRIO_SHIFT)
#ifndef portINTC_GIC_MIN_PRIO
    #define portINTC_GIC_MIN_PRIO                   (0)
#endif

#define portYIELD()                     vPortYield()
#define portSET_INTERRUPT_MASK_FROM_ISR()       ulPortRaiseBASEPRI()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x)    vPortSetBASEPRI( x )
#define portENABLE_INTERRUPTS()                 vPortSetBASEPRI(portINTC_GIC_CONVERT_PRIO_SET( portINTC_GIC_MIN_PRIO ))

#if ( configNUMBER_OF_CORES > 1 )
#define portCRITICAL_NESTING_IN_TCB             (1)

#define portDISABLE_INTERRUPTS()                ulPortRaiseBASEPRI()
#define portRESTORE_INTERRUPTS(x)               vPortSetBASEPRI( x )
#define portENTER_CRITICAL()                    vTaskEnterCritical()
#define portEXIT_CRITICAL()                     vTaskExitCritical()
#else
/* Critical section management. */
extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );
#define portDISABLE_INTERRUPTS()                vPortRaiseBASEPRI()
#define portENTER_CRITICAL()                    vPortEnterCritical()
#define portEXIT_CRITICAL()                     vPortExitCritical()
#endif
/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site.  These are
not necessary for to use this port.  They are defined so the common demo files
(which build with all the ports) will build. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

/* Architecture specific optimisations. */
#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
    #define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#endif

#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1

    /* Generic helper function. */
    __attribute__( ( always_inline ) ) static inline uint8_t ucPortCountLeadingZeros( uint32_t ulBitmap )
    {
    uint8_t ucReturn;

        __asm volatile ( "clz %0, %1" : "=r" ( ucReturn ) : "r" ( ulBitmap ) : "memory" );
        return ucReturn;
    }

    /* Check the configuration. */
    #if( configMAX_PRIORITIES > 32 )
        #error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 difference priorities as tasks that share a priority will time slice.
    #endif

    /* Store/clear the ready priorities in a bit map. */
    #define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )
    #define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities ) ( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

    /*-----------------------------------------------------------*/

    #define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities ) uxTopPriority = ( 31UL - ( uint32_t ) ucPortCountLeadingZeros( ( uxReadyPriorities ) ) )

#endif /* configUSE_PORT_OPTIMISED_TASK_SELECTION */

/*-----------------------------------------------------------*/

#if ( configNUMBER_OF_CORES > 1 )
#else
/*-----------------------------------------------------------*/

portFORCE_INLINE static void vPortRaiseBASEPRI( void )
{
uint32_t ulNewBASEPRI;

    __asm volatile
    (
        " mov %0, %1                                              \n" \
        " mcr p15, 0, %0, c4, c6, 0                               \n" \
        :"=r" (ulNewBASEPRI) : "i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY ) : "memory"
    );
    OSASM(" dsb                         \n" : : : );
}

/*-----------------------------------------------------------*/
#endif
portFORCE_INLINE static uint32_t ulPortRaiseBASEPRI( void )
{
uint32_t ulOriginalBASEPRI, ulNewBASEPRI;

    OSASM
    (
        " mrc p15, 0, %0, c4, c6, 0                               \n" \
        " mov %1, %2                                              \n" \
        " mcr p15, 0, %1, c4, c6, 0                               \n" \
        :"=r" (ulOriginalBASEPRI), "=r" (ulNewBASEPRI) : "i" ( configMAX_SYSCALL_INTERRUPT_PRIORITY ) : "memory"
    );
    OSASM("dsb sy" ::: "memory");
    return ulOriginalBASEPRI;
}
/*-----------------------------------------------------------*/

portFORCE_INLINE static void vPortSetBASEPRI( uint32_t ulNewMaskValue )
{
    OSASM
    (
        " mcr p15, 0, %0, c4, c6, 0   " :: "r" ( ulNewMaskValue ) : "memory"
    );
    OSASM("dsb sy" ::: "memory");
}
/*-----------------------------------------------------------*/
#if ( configNUMBER_OF_CORES > 1 )
#define portYIELD_FROM_ISR(x)                if ((x)) {                                         \
                                                uint32_t ulStateISR = portDISABLE_INTERRUPTS(); \
                                                vTaskSwitchContext(portGET_CORE_ID());          \
                                                portRESTORE_INTERRUPTS(ulStateISR);             \
                                            }
#else
#define portYIELD_FROM_ISR(x)                if ((x)) {                                 \
                                                portDISABLE_INTERRUPTS();               \
                                                vTaskSwitchContext();                   \
                                                portENABLE_INTERRUPTS();                \
                                            }
#endif
/*-----------------------------------------------------------*/

#if ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
extern BaseType_t xTaskCreateFpu( TaskFunction_t pxTaskCode,
                                  const char * const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                                  const configSTACK_DEPTH_TYPE usStackDepth,
                                  void * const pvParameters,
                                  UBaseType_t uxPriority,
								  pTaskHandle_t * const pxCreatedTask );
#endif /* configSUPPORT_DYNAMIC_ALLOCATION */
/*-----------------------------------------------------------*/

static inline BaseType_t xPortIsInsideInterrupt( void )
{
    uint32_t ulCurrentInterrupt = 0;
    BaseType_t xReturn = pdFALSE;

    OSASM(" mrc p15, 0, %0, c12, c11, 3 \n" : "=r"(ulCurrentInterrupt): : );

    if( ulCurrentInterrupt < 0xf8 )
    {
        xReturn = pdTRUE;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

#if ( configNUMBER_OF_CORES > 1 )

extern void vYieldCore(int xCoreID);
extern void vCoreSignal(uint32_t sigId, uint32_t coreId);

#define portRTOS_LOCK_COUNT                         (2UL)
#define portISR_LOCK                                (0UL)
#define portTASK_LOCK                               (1UL)
#define portGET_ISR_LOCK()                          vPortRecursiveLock(portISR_LOCK, pdTRUE)
#define portRELEASE_ISR_LOCK()                      vPortRecursiveLock(portISR_LOCK, pdFALSE)
#define portGET_TASK_LOCK()                         vPortRecursiveLock(portTASK_LOCK, pdTRUE)
#define portRELEASE_TASK_LOCK()                     vPortRecursiveLock(portTASK_LOCK, pdFALSE)
#define portGET_CORE_ID                             getCoreIndex
#define portYIELD_CORE( a )                         vYieldCore( a )
#define portSET_INTERRUPT_MASK()                    ulPortRaiseBASEPRI()
#define portCLEAR_INTERRUPT_MASK( ulState )         vPortSetBASEPRI(ulState)
#define portENTER_CRITICAL_FROM_ISR()               vTaskEnterCriticalFromISR()
#define portEXIT_CRITICAL_FROM_ISR(x)               vTaskExitCriticalFromISR( x )

#if defined(portCRITICAL_NESTING_IN_TCB) && (portCRITICAL_NESTING_IN_TCB == 0)
#define portGET_CRITICAL_NESTING_COUNT(  )          ( uxCriticalNesting[ ( portGET_CORE_ID() ) ] )
#define portSET_CRITICAL_NESTING_COUNT( x )         ( uxCriticalNesting[ ( portGET_CORE_ID() ) ] = ( x ) )
#define portINCREMENT_CRITICAL_NESTING_COUNT(  )    ( uxCriticalNesting[ ( portGET_CORE_ID() ) ]++ )
#define portDECREMENT_CRITICAL_NESTING_COUNT(  )    ( uxCriticalNesting[ ( portGET_CORE_ID() ) ]-- )
#endif

#define portCHECK_IF_IN_ISR()                       xPortIsInsideInterrupt()

extern UBaseType_t uxCriticalNesting[ configNUMBER_OF_CORES ];
extern volatile uint32_t ulPortGateWord[ portRTOS_LOCK_COUNT ];
extern volatile uint32_t ulPortOwnedByCore[ configNUMBER_OF_CORES ];
extern volatile uint32_t ulPortRecursionCountByLock[ portRTOS_LOCK_COUNT ];

static portFORCE_INLINE int32_t iPortTryLock(volatile uint32_t* addr)
{
    register int32_t tmp, ret;
    OSASM(" 1: " : : : );
    OSASM(" dsb " : : : );

    OSASM(" ldrex %0, [%1] " : "=r"(ret) : "r"(addr) : );
    OSASM(" cmp %0, #1 " : : "r"(ret) : );
    OSASM(" beq 1f " : : : );
    OSASM(" mov %0, #1 " : "=r"(tmp): : );
    OSASM(" strex %0, %1, [%2] " : "=&r"(ret) : "r"(tmp), "r"(addr) : );
    OSASM(" dsb " : : : );
    OSASM(" cmp %0, #0 " : : "r"(ret) : );
    OSASM(" bne 1b " : : : );
    OSASM(" 1: " : : : );
    return ret;
}
static portFORCE_INLINE void vPortUnLock(volatile uint32_t* addr)
{
    *addr = 0UL;
    OSASM("dsb sy" ::: "memory");
}

/*-----------------------------------------------------------*/
static portFORCE_INLINE void vPortRecursiveLock(uint32_t ulLockNum, BaseType_t uxAcquire)
{

    uint32_t ulCoreNum = portGET_CORE_ID();
    uint32_t ulLockBit = 1u << ulLockNum;

    /* Lock acquire */
    if (uxAcquire)
    {
        /* Check if spinlock is available */
        /* If spinlock is not available check if the core owns the lock */
        /* If the core owns the lock wait increment the lock count by the core */
        /* If core does not own the lock wait for the spinlock */
        if( iPortTryLock( &ulPortGateWord[ulLockNum] ) != 0)
        {
            /* Check if the core owns the spinlock */
            OSASM("dsb sy" ::: "memory");
            if( ulPortOwnedByCore[ulCoreNum] & ulLockBit )
            {
                configASSERT( ulPortRecursionCountByLock[ulLockNum] != 255UL );
                ulPortRecursionCountByLock[ulLockNum] ++;
                OSASM("dsb sy" ::: "memory");
                return;
            }
            /* Wait for spinlock */
            while( iPortTryLock(&ulPortGateWord[ulLockNum]) != 0);
        }

         /* Add barrier to ensure lock is taken before we proceed */
    	OSASM("dsb sy" ::: "memory");

        /* Assert the lock count is 0 when the spinlock is free and is acquired */
        configASSERT(ulPortRecursionCountByLock[ulLockNum] == 0);

        /* Set lock count as 1 */
        ulPortRecursionCountByLock[ulLockNum] = 1;
        /* Set ulPortOwnedByCore */
        ulPortOwnedByCore[ulCoreNum] |= ulLockBit;
        OSASM("dsb sy" ::: "memory");
    }
    /* Lock release */
    else
    {
        /* Assert the lock is not free already */
        configASSERT( (ulPortOwnedByCore[ulCoreNum] & ulLockBit) != 0 );
        configASSERT( ulPortRecursionCountByLock[ulLockNum] != 0 );

        /* Reduce ulPortRecursionCountByLock by 1 */
        ulPortRecursionCountByLock[ulLockNum] --;
        OSASM("dsb sy" ::: "memory");

        if( !ulPortRecursionCountByLock[ulLockNum] )
        {
            ulPortOwnedByCore[ulCoreNum] &= ~ulLockBit;
            OSASM("dsb sy" ::: "memory");
            vPortUnLock(&ulPortGateWord[ulLockNum]);
        }
    }
}
#endif /* ( configNUMBER_OF_CORES > 1 ) */


#ifdef __cplusplus
}
#endif
#endif /* PORTMACRO_H */
