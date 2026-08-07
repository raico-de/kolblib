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

/* Defining MPU_WRAPPERS_INCLUDED_FROM_API_FILE prevents task.h from redefining
 * all the API functions to use the MPU wrappers.  That should only be done when
 * task.h is included from an application file. */
#define MPU_WRAPPERS_INCLUDED_FROM_API_FILE

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "event_groups.h"
#include "stream_buffer.h"
#include "mpu_prototypes.h"
#include "mpu_syscall_numbers.h"

#undef MPU_WRAPPERS_INCLUDED_FROM_API_FILE
/*-----------------------------------------------------------*/

#if ( ( configENABLE_MPU == 1 ) && ( configUSE_MPU_WRAPPERS_V1 == 0 ) )

    #if ( INCLUDE_xTaskDelayUntil == 1 )

        BaseType_t MPU_xTaskDelayUntil( TickType_t * const pxPreviousWakeTime,
                                        const TickType_t xTimeIncrement ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        BaseType_t MPU_xTaskDelayUntil( TickType_t * const pxPreviousWakeTime,
                                        const TickType_t xTimeIncrement ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTaskDelayUntil ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTaskDelayUntilImpl             \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( INCLUDE_xTaskDelayUntil == 1 ) */
/*-----------------------------------------------------------*/

    #if ( INCLUDE_xTaskAbortDelay == 1 )

        BaseType_t MPU_xTaskAbortDelay( TaskHandle_t xTask ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        BaseType_t MPU_xTaskAbortDelay( TaskHandle_t xTask ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTaskAbortDelay ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTaskAbortDelayImpl             \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( INCLUDE_xTaskAbortDelay == 1 ) */
/*-----------------------------------------------------------*/

    #if ( INCLUDE_vTaskDelay == 1 )

        void MPU_vTaskDelay( const TickType_t xTicksToDelay ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        void MPU_vTaskDelay( const TickType_t xTicksToDelay ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_vTaskDelay ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_vTaskDelayImpl                  \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( INCLUDE_vTaskDelay == 1 ) */
/*-----------------------------------------------------------*/

    #if ( INCLUDE_uxTaskPriorityGet == 1 )

        UBaseType_t MPU_uxTaskPriorityGet( const TaskHandle_t xTask ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        UBaseType_t MPU_uxTaskPriorityGet( const TaskHandle_t xTask ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_uxTaskPriorityGet ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_uxTaskPriorityGetImpl           \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( INCLUDE_uxTaskPriorityGet == 1 ) */
/*-----------------------------------------------------------*/

    #if ( INCLUDE_eTaskGetState == 1 )

        eTaskState MPU_eTaskGetState( TaskHandle_t xTask ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        eTaskState MPU_eTaskGetState( TaskHandle_t xTask ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_eTaskGetState ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_eTaskGetStateImpl               \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( INCLUDE_eTaskGetState == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TRACE_FACILITY == 1 )

        void MPU_vTaskGetInfo( TaskHandle_t xTask,
                               TaskStatus_t * pxTaskStatus,
                               BaseType_t xGetFreeStackSpace,
                               eTaskState eState ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        void MPU_vTaskGetInfo( TaskHandle_t xTask,
                               TaskStatus_t * pxTaskStatus,
                               BaseType_t xGetFreeStackSpace,
                               eTaskState eState ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_vTaskGetInfo ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_vTaskGetInfoImpl                \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TRACE_FACILITY == 1 ) */
/*-----------------------------------------------------------*/

    #if ( INCLUDE_xTaskGetIdleTaskHandle == 1 )

        TaskHandle_t MPU_xTaskGetIdleTaskHandle( void ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        TaskHandle_t MPU_xTaskGetIdleTaskHandle( void ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTaskGetIdleTaskHandle ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTaskGetIdleTaskHandleImpl      \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( INCLUDE_xTaskGetIdleTaskHandle == 1 ) */
/*-----------------------------------------------------------*/

    #if ( INCLUDE_vTaskSuspend == 1 )

        void MPU_vTaskSuspend( TaskHandle_t xTaskToSuspend ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        void MPU_vTaskSuspend( TaskHandle_t xTaskToSuspend ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_vTaskSuspend ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_vTaskSuspendImpl                \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( INCLUDE_vTaskSuspend == 1 ) */
/*-----------------------------------------------------------*/

    #if ( INCLUDE_vTaskSuspend == 1 )

        void MPU_vTaskResume( TaskHandle_t xTaskToResume ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        void MPU_vTaskResume( TaskHandle_t xTaskToResume ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_vTaskResume ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_vTaskResumeImpl                 \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( INCLUDE_vTaskSuspend == 1 ) */
/*-----------------------------------------------------------*/

    TickType_t MPU_xTaskGetTickCount( void ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    TickType_t MPU_xTaskGetTickCount( void ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTaskGetTickCount ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xTaskGetTickCountImpl           \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    UBaseType_t MPU_uxTaskGetNumberOfTasks( void ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    UBaseType_t MPU_uxTaskGetNumberOfTasks( void ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_uxTaskGetNumberOfTasks ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_uxTaskGetNumberOfTasksImpl      \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    #if ( configGENERATE_RUN_TIME_STATS == 1 )

        configRUN_TIME_COUNTER_TYPE MPU_ulTaskGetRunTimeCounter( const TaskHandle_t xTask ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        configRUN_TIME_COUNTER_TYPE MPU_ulTaskGetRunTimeCounter( const TaskHandle_t xTask ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_ulTaskGetRunTimeCounter ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_ulTaskGetRunTimeCounterImpl     \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( ( configGENERATE_RUN_TIME_STATS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configGENERATE_RUN_TIME_STATS == 1 )

        configRUN_TIME_COUNTER_TYPE MPU_ulTaskGetRunTimePercent( const TaskHandle_t xTask ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        configRUN_TIME_COUNTER_TYPE MPU_ulTaskGetRunTimePercent( const TaskHandle_t xTask ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_ulTaskGetRunTimePercent ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_ulTaskGetRunTimePercentImpl     \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( ( configGENERATE_RUN_TIME_STATS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( ( configGENERATE_RUN_TIME_STATS == 1 ) && ( INCLUDE_xTaskGetIdleTaskHandle == 1 ) )

        configRUN_TIME_COUNTER_TYPE MPU_ulTaskGetIdleRunTimePercent( void ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        configRUN_TIME_COUNTER_TYPE MPU_ulTaskGetIdleRunTimePercent( void ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_ulTaskGetIdleRunTimePercent ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_ulTaskGetIdleRunTimePercentImpl \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( ( configGENERATE_RUN_TIME_STATS == 1 ) && ( INCLUDE_xTaskGetIdleTaskHandle == 1 ) ) */
/*-----------------------------------------------------------*/

    #if ( ( configGENERATE_RUN_TIME_STATS == 1 ) && ( INCLUDE_xTaskGetIdleTaskHandle == 1 ) )

        configRUN_TIME_COUNTER_TYPE MPU_ulTaskGetIdleRunTimeCounter( void ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        configRUN_TIME_COUNTER_TYPE MPU_ulTaskGetIdleRunTimeCounter( void ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_ulTaskGetIdleRunTimeCounter ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_ulTaskGetIdleRunTimeCounterImpl \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( ( configGENERATE_RUN_TIME_STATS == 1 ) && ( INCLUDE_xTaskGetIdleTaskHandle == 1 ) ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_APPLICATION_TASK_TAG == 1 )

        void MPU_vTaskSetApplicationTaskTag( TaskHandle_t xTask,
                                             TaskHookFunction_t pxHookFunction ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        void MPU_vTaskSetApplicationTaskTag( TaskHandle_t xTask,
                                             TaskHookFunction_t pxHookFunction ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_vTaskSetApplicationTaskTag ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_vTaskSetApplicationTaskTagImpl  \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_APPLICATION_TASK_TAG == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_APPLICATION_TASK_TAG == 1 )

        TaskHookFunction_t MPU_xTaskGetApplicationTaskTag( TaskHandle_t xTask ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        TaskHookFunction_t MPU_xTaskGetApplicationTaskTag( TaskHandle_t xTask ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTaskGetApplicationTaskTag ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTaskGetApplicationTaskTagImpl  \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_APPLICATION_TASK_TAG == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configNUM_THREAD_LOCAL_STORAGE_POINTERS != 0 )

        void MPU_vTaskSetThreadLocalStoragePointer( TaskHandle_t xTaskToSet,
                                                    BaseType_t xIndex,
                                                    void * pvValue ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        void MPU_vTaskSetThreadLocalStoragePointer( TaskHandle_t xTaskToSet,
                                                    BaseType_t xIndex,
                                                    void * pvValue ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_vTaskSetThreadLocalStoragePointer ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_vTaskSetThreadLocalStoragePointerImpl  \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configNUM_THREAD_LOCAL_STORAGE_POINTERS != 0 ) */
/*-----------------------------------------------------------*/

    #if ( configNUM_THREAD_LOCAL_STORAGE_POINTERS != 0 )

        void * MPU_pvTaskGetThreadLocalStoragePointer( TaskHandle_t xTaskToQuery,
                                                       BaseType_t xIndex ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        void * MPU_pvTaskGetThreadLocalStoragePointer( TaskHandle_t xTaskToQuery,
                                                       BaseType_t xIndex ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_pvTaskGetThreadLocalStoragePointer ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_pvTaskGetThreadLocalStoragePointerImpl  \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configNUM_THREAD_LOCAL_STORAGE_POINTERS != 0 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TRACE_FACILITY == 1 )

        UBaseType_t MPU_uxTaskGetSystemState( TaskStatus_t * const pxTaskStatusArray,
                                              const UBaseType_t uxArraySize,
                                              configRUN_TIME_COUNTER_TYPE * const pulTotalRunTime ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        UBaseType_t MPU_uxTaskGetSystemState( TaskStatus_t * const pxTaskStatusArray,
                                              const UBaseType_t uxArraySize,
                                              configRUN_TIME_COUNTER_TYPE * const pulTotalRunTime ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_uxTaskGetSystemState ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_uxTaskGetSystemStateImpl        \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TRACE_FACILITY == 1 ) */
/*-----------------------------------------------------------*/

    #if ( INCLUDE_uxTaskGetStackHighWaterMark == 1 )

        UBaseType_t MPU_uxTaskGetStackHighWaterMark( TaskHandle_t xTask ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        UBaseType_t MPU_uxTaskGetStackHighWaterMark( TaskHandle_t xTask ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_uxTaskGetStackHighWaterMark ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_uxTaskGetStackHighWaterMarkImpl \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( INCLUDE_uxTaskGetStackHighWaterMark == 1 ) */
/*-----------------------------------------------------------*/

    #if ( INCLUDE_uxTaskGetStackHighWaterMark2 == 1 )

        configSTACK_DEPTH_TYPE MPU_uxTaskGetStackHighWaterMark2( TaskHandle_t xTask ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        configSTACK_DEPTH_TYPE MPU_uxTaskGetStackHighWaterMark2( TaskHandle_t xTask ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_uxTaskGetStackHighWaterMark2 ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_uxTaskGetStackHighWaterMark2Impl \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( INCLUDE_uxTaskGetStackHighWaterMark2 == 1 ) */
/*-----------------------------------------------------------*/

    #if ( ( INCLUDE_xTaskGetCurrentTaskHandle == 1 ) || ( configUSE_MUTEXES == 1 ) )

        TaskHandle_t MPU_xTaskGetCurrentTaskHandle( void ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        TaskHandle_t MPU_xTaskGetCurrentTaskHandle( void ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTaskGetCurrentTaskHandle ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTaskGetCurrentTaskHandleImpl   \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( ( INCLUDE_xTaskGetCurrentTaskHandle == 1 ) || ( configUSE_MUTEXES == 1 ) ) */
/*-----------------------------------------------------------*/

    #if ( INCLUDE_xTaskGetSchedulerState == 1 )

        BaseType_t MPU_xTaskGetSchedulerState( void ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        BaseType_t MPU_xTaskGetSchedulerState( void ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTaskGetSchedulerState ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTaskGetSchedulerStateImpl      \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( INCLUDE_xTaskGetSchedulerState == 1 ) */
/*-----------------------------------------------------------*/

    void MPU_vTaskSetTimeOutState( TimeOut_t * const pxTimeOut ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    void MPU_vTaskSetTimeOutState( TimeOut_t * const pxTimeOut ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_vTaskSetTimeOutState ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_vTaskSetTimeOutStateImpl        \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    BaseType_t MPU_xTaskCheckForTimeOut( TimeOut_t * const pxTimeOut,
                                         TickType_t * const pxTicksToWait ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    BaseType_t MPU_xTaskCheckForTimeOut( TimeOut_t * const pxTimeOut,
                                         TickType_t * const pxTicksToWait ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTaskCheckForTimeOut ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xTaskCheckForTimeOutImpl        \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    #if ( configUSE_TASK_NOTIFICATIONS == 1 )

        BaseType_t MPU_xTaskGenericNotifyEntry( const xTaskGenericNotifyParams_t * pxParams ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        BaseType_t MPU_xTaskGenericNotifyEntry( const xTaskGenericNotifyParams_t * pxParams ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTaskGenericNotify ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTaskGenericNotifyImpl          \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TASK_NOTIFICATIONS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TASK_NOTIFICATIONS == 1 )

        BaseType_t MPU_xTaskGenericNotifyWaitEntry( const xTaskGenericNotifyWaitParams_t * pxParams ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        BaseType_t MPU_xTaskGenericNotifyWaitEntry( const xTaskGenericNotifyWaitParams_t * pxParams ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTaskGenericNotifyWait ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTaskGenericNotifyWaitImpl      \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TASK_NOTIFICATIONS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TASK_NOTIFICATIONS == 1 )

        uint32_t MPU_ulTaskGenericNotifyTake( UBaseType_t uxIndexToWaitOn,
                                              BaseType_t xClearCountOnExit,
                                              TickType_t xTicksToWait ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        uint32_t MPU_ulTaskGenericNotifyTake( UBaseType_t uxIndexToWaitOn,
                                              BaseType_t xClearCountOnExit,
                                              TickType_t xTicksToWait ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_ulTaskGenericNotifyTake ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_ulTaskGenericNotifyTakeImpl     \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TASK_NOTIFICATIONS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TASK_NOTIFICATIONS == 1 )

        BaseType_t MPU_xTaskGenericNotifyStateClear( TaskHandle_t xTask,
                                                     UBaseType_t uxIndexToClear ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        BaseType_t MPU_xTaskGenericNotifyStateClear( TaskHandle_t xTask,
                                                     UBaseType_t uxIndexToClear ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTaskGenericNotifyStateClear ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTaskGenericNotifyStateClearImpl     \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TASK_NOTIFICATIONS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TASK_NOTIFICATIONS == 1 )

        uint32_t MPU_ulTaskGenericNotifyValueClear( TaskHandle_t xTask,
                                                    UBaseType_t uxIndexToClear,
                                                    uint32_t ulBitsToClear ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        uint32_t MPU_ulTaskGenericNotifyValueClear( TaskHandle_t xTask,
                                                    UBaseType_t uxIndexToClear,
                                                    uint32_t ulBitsToClear ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_ulTaskGenericNotifyValueClear ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_ulTaskGenericNotifyValueClearImpl     \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TASK_NOTIFICATIONS == 1 ) */
/*-----------------------------------------------------------*/

    BaseType_t MPU_xQueueGenericSend( QueueHandle_t xQueue,
                                      const void * const pvItemToQueue,
                                      TickType_t xTicksToWait,
                                      const BaseType_t xCopyPosition ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    BaseType_t MPU_xQueueGenericSend( QueueHandle_t xQueue,
                                      const void * const pvItemToQueue,
                                      TickType_t xTicksToWait,
                                      const BaseType_t xCopyPosition ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xQueueGenericSend ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xQueueGenericSendImpl           \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    UBaseType_t MPU_uxQueueMessagesWaiting( const QueueHandle_t xQueue ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    UBaseType_t MPU_uxQueueMessagesWaiting( const QueueHandle_t xQueue ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_uxQueueMessagesWaiting ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_uxQueueMessagesWaitingImpl      \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    UBaseType_t MPU_uxQueueSpacesAvailable( const QueueHandle_t xQueue ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    UBaseType_t MPU_uxQueueSpacesAvailable( const QueueHandle_t xQueue ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_uxQueueSpacesAvailable ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_uxQueueSpacesAvailableImpl      \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    BaseType_t MPU_xQueueReceive( QueueHandle_t xQueue,
                                  void * const pvBuffer,
                                  TickType_t xTicksToWait ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    BaseType_t MPU_xQueueReceive( QueueHandle_t xQueue,
                                  void * const pvBuffer,
                                  TickType_t xTicksToWait ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xQueueReceive ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xQueueReceiveImpl               \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    BaseType_t MPU_xQueuePeek( QueueHandle_t xQueue,
                               void * const pvBuffer,
                               TickType_t xTicksToWait ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    BaseType_t MPU_xQueuePeek( QueueHandle_t xQueue,
                               void * const pvBuffer,
                               TickType_t xTicksToWait ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xQueuePeek ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xQueuePeekImpl                  \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    BaseType_t MPU_xQueueSemaphoreTake( QueueHandle_t xQueue,
                                        TickType_t xTicksToWait ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    BaseType_t MPU_xQueueSemaphoreTake( QueueHandle_t xQueue,
                                        TickType_t xTicksToWait ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                                         \n" : : "i" ( SYSTEM_CALL_xQueueSemaphoreTake ) : );
        OSASM(" push {r12}                                          \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3                         \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3                         \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                                           \n");
        OSASM(" push {lr}                                           \n");
        OSASM(" blx MPU_xQueueSemaphoreTakeImpl                     \n");
        OSASM(" pop {pc}                                            \n");
    }
/*-----------------------------------------------------------*/

    #if ( ( configUSE_MUTEXES == 1 ) && ( INCLUDE_xSemaphoreGetMutexHolder == 1 ) )

        TaskHandle_t MPU_xQueueGetMutexHolder( QueueHandle_t xSemaphore ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        TaskHandle_t MPU_xQueueGetMutexHolder( QueueHandle_t xSemaphore ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xQueueGetMutexHolder ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xQueueGetMutexHolderImpl        \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( ( configUSE_MUTEXES == 1 ) && ( INCLUDE_xSemaphoreGetMutexHolder == 1 ) ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_RECURSIVE_MUTEXES == 1 )

        BaseType_t MPU_xQueueTakeMutexRecursive( QueueHandle_t xMutex,
                                                 TickType_t xTicksToWait ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        BaseType_t MPU_xQueueTakeMutexRecursive( QueueHandle_t xMutex,
                                                 TickType_t xTicksToWait ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xQueueTakeMutexRecursive ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xQueueTakeMutexRecursiveImpl    \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_RECURSIVE_MUTEXES == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_RECURSIVE_MUTEXES == 1 )

        BaseType_t MPU_xQueueGiveMutexRecursive( QueueHandle_t pxMutex ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        BaseType_t MPU_xQueueGiveMutexRecursive( QueueHandle_t pxMutex ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xQueueGiveMutexRecursive ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xQueueGiveMutexRecursiveImpl    \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_RECURSIVE_MUTEXES == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_QUEUE_SETS == 1 )

        QueueSetMemberHandle_t MPU_xQueueSelectFromSet( QueueSetHandle_t xQueueSet,
                                                        const TickType_t xTicksToWait ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        QueueSetMemberHandle_t MPU_xQueueSelectFromSet( QueueSetHandle_t xQueueSet,
                                                        const TickType_t xTicksToWait ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xQueueSelectFromSet ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xQueueSelectFromSetImpl         \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_QUEUE_SETS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_QUEUE_SETS == 1 )

        BaseType_t MPU_xQueueAddToSet( QueueSetMemberHandle_t xQueueOrSemaphore,
                                       QueueSetHandle_t xQueueSet ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        BaseType_t MPU_xQueueAddToSet( QueueSetMemberHandle_t xQueueOrSemaphore,
                                       QueueSetHandle_t xQueueSet ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xQueueAddToSet ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xQueueAddToSetImpl              \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_QUEUE_SETS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configQUEUE_REGISTRY_SIZE > 0 )

        void MPU_vQueueAddToRegistry( QueueHandle_t xQueue,
                                      const char * pcName ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        void MPU_vQueueAddToRegistry( QueueHandle_t xQueue,
                                      const char * pcName ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_vQueueAddToRegistry ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_vQueueAddToRegistryImpl         \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configQUEUE_REGISTRY_SIZE > 0 ) */
/*-----------------------------------------------------------*/

    #if ( configQUEUE_REGISTRY_SIZE > 0 )

        void MPU_vQueueUnregisterQueue( QueueHandle_t xQueue ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        void MPU_vQueueUnregisterQueue( QueueHandle_t xQueue ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_vQueueUnregisterQueue ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_vQueueUnregisterQueueImpl       \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configQUEUE_REGISTRY_SIZE > 0 ) */
/*-----------------------------------------------------------*/

    #if ( configQUEUE_REGISTRY_SIZE > 0 )

        const char * MPU_pcQueueGetName( QueueHandle_t xQueue ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        const char * MPU_pcQueueGetName( QueueHandle_t xQueue ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_pcQueueGetName ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_pcQueueGetNameImpl              \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configQUEUE_REGISTRY_SIZE > 0 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TIMERS == 1 )

        void * MPU_pvTimerGetTimerID( const TimerHandle_t xTimer ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        void * MPU_pvTimerGetTimerID( const TimerHandle_t xTimer ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_pvTimerGetTimerID ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_pvTimerGetTimerIDImpl           \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TIMERS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TIMERS == 1 )

        void MPU_vTimerSetTimerID( TimerHandle_t xTimer,
                                   void * pvNewID ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        void MPU_vTimerSetTimerID( TimerHandle_t xTimer,
                                   void * pvNewID ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_vTimerSetTimerID ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_vTimerSetTimerIDImpl            \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TIMERS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TIMERS == 1 )

        BaseType_t MPU_xTimerIsTimerActive( TimerHandle_t xTimer ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        BaseType_t MPU_xTimerIsTimerActive( TimerHandle_t xTimer ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTimerIsTimerActive ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTimerIsTimerActiveImpl         \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TIMERS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TIMERS == 1 )

        TaskHandle_t MPU_xTimerGetTimerDaemonTaskHandle( void ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        TaskHandle_t MPU_xTimerGetTimerDaemonTaskHandle( void ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTimerGetTimerDaemonTaskHandle ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTimerGetTimerDaemonTaskHandleImpl         \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TIMERS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TIMERS == 1 )

        BaseType_t MPU_xTimerGenericCommandFromTaskEntry( const xTimerGenericCommandFromTaskParams_t * pxParams ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        BaseType_t MPU_xTimerGenericCommandFromTaskEntry( const xTimerGenericCommandFromTaskParams_t * pxParams ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTimerGenericCommandFromTask ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTimerGenericCommandFromTaskImpl         \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TIMERS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TIMERS == 1 )

        const char * MPU_pcTimerGetName( TimerHandle_t xTimer ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        const char * MPU_pcTimerGetName( TimerHandle_t xTimer ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_pcTimerGetName ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_pcTimerGetNameImpl              \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TIMERS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TIMERS == 1 )

        void MPU_vTimerSetReloadMode( TimerHandle_t xTimer,
                                      const BaseType_t uxAutoReload ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        void MPU_vTimerSetReloadMode( TimerHandle_t xTimer,
                                      const BaseType_t uxAutoReload ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_vTimerSetReloadMode ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_vTimerSetReloadModeImpl         \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TIMERS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TIMERS == 1 )

        BaseType_t MPU_xTimerGetReloadMode( TimerHandle_t xTimer ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        BaseType_t MPU_xTimerGetReloadMode( TimerHandle_t xTimer ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTimerGetReloadMode ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTimerGetReloadModeImpl         \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TIMERS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TIMERS == 1 )

        UBaseType_t MPU_uxTimerGetReloadMode( TimerHandle_t xTimer ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        UBaseType_t MPU_uxTimerGetReloadMode( TimerHandle_t xTimer ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_uxTimerGetReloadMode ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_uxTimerGetReloadModeImpl        \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TIMERS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TIMERS == 1 )

        TickType_t MPU_xTimerGetPeriod( TimerHandle_t xTimer ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        TickType_t MPU_xTimerGetPeriod( TimerHandle_t xTimer ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTimerGetPeriod ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTimerGetPeriodImpl             \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TIMERS == 1 ) */
/*-----------------------------------------------------------*/

    #if ( configUSE_TIMERS == 1 )

        TickType_t MPU_xTimerGetExpiryTime( TimerHandle_t xTimer ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        TickType_t MPU_xTimerGetExpiryTime( TimerHandle_t xTimer ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xTimerGetExpiryTime ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_xTimerGetExpiryTimeImpl         \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /* if ( configUSE_TIMERS == 1 ) */
/*-----------------------------------------------------------*/

    EventBits_t MPU_xEventGroupWaitBitsEntry( const xEventGroupWaitBitsParams_t * pxParams ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    EventBits_t MPU_xEventGroupWaitBitsEntry( const xEventGroupWaitBitsParams_t * pxParams ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xEventGroupWaitBits ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xEventGroupWaitBitsImpl         \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    EventBits_t MPU_xEventGroupClearBits( EventGroupHandle_t xEventGroup,
                                          const EventBits_t uxBitsToClear ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    EventBits_t MPU_xEventGroupClearBits( EventGroupHandle_t xEventGroup,
                                          const EventBits_t uxBitsToClear ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xEventGroupClearBits ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xEventGroupClearBitsImpl        \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    EventBits_t MPU_xEventGroupSetBits( EventGroupHandle_t xEventGroup,
                                        const EventBits_t uxBitsToSet ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    EventBits_t MPU_xEventGroupSetBits( EventGroupHandle_t xEventGroup,
                                        const EventBits_t uxBitsToSet ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xEventGroupSetBits ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xEventGroupSetBitsImpl          \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    EventBits_t MPU_xEventGroupSync( EventGroupHandle_t xEventGroup,
                                     const EventBits_t uxBitsToSet,
                                     const EventBits_t uxBitsToWaitFor,
                                     TickType_t xTicksToWait ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    EventBits_t MPU_xEventGroupSync( EventGroupHandle_t xEventGroup,
                                     const EventBits_t uxBitsToSet,
                                     const EventBits_t uxBitsToWaitFor,
                                     TickType_t xTicksToWait ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xEventGroupSync ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xEventGroupSyncImpl             \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    #if ( configUSE_TRACE_FACILITY == 1 )

        UBaseType_t MPU_uxEventGroupGetNumber( void * xEventGroup ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        UBaseType_t MPU_uxEventGroupGetNumber( void * xEventGroup ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_uxEventGroupGetNumber ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_uxEventGroupGetNumberImpl       \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /*( configUSE_TRACE_FACILITY == 1 )*/
/*-----------------------------------------------------------*/

    #if ( configUSE_TRACE_FACILITY == 1 )

        void MPU_vEventGroupSetNumber( void * xEventGroup,
                                       UBaseType_t uxEventGroupNumber ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

        void MPU_vEventGroupSetNumber( void * xEventGroup,
                                       UBaseType_t uxEventGroupNumber ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
        {
            OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_vEventGroupSetNumber ) : );
            OSASM(" push {r12}                              \n"); /* save syscall to stack */
            OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
            OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
            OSASM(" pop {r12}                               \n");
            OSASM(" push {lr}                               \n");
            OSASM(" blx MPU_vEventGroupSetNumberImpl        \n");
            OSASM(" pop {pc}                                \n");
        }

    #endif /*( configUSE_TRACE_FACILITY == 1 )*/
/*-----------------------------------------------------------*/

    #if (configUSE_STREAM_BUFFERS == 1)
    size_t MPU_xStreamBufferSend( StreamBufferHandle_t xStreamBuffer,
                                  const void * pvTxData,
                                  size_t xDataLengthBytes,
                                  TickType_t xTicksToWait ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    size_t MPU_xStreamBufferSend( StreamBufferHandle_t xStreamBuffer,
                                  const void * pvTxData,
                                  size_t xDataLengthBytes,
                                  TickType_t xTicksToWait ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xStreamBufferSend ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xStreamBufferSendImpl           \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    size_t MPU_xStreamBufferReceive( StreamBufferHandle_t xStreamBuffer,
                                     void * pvRxData,
                                     size_t xBufferLengthBytes,
                                     TickType_t xTicksToWait ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    size_t MPU_xStreamBufferReceive( StreamBufferHandle_t xStreamBuffer,
                                     void * pvRxData,
                                     size_t xBufferLengthBytes,
                                     TickType_t xTicksToWait ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xStreamBufferReceive ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xStreamBufferReceiveImpl        \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    BaseType_t MPU_xStreamBufferIsFull( StreamBufferHandle_t xStreamBuffer ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    BaseType_t MPU_xStreamBufferIsFull( StreamBufferHandle_t xStreamBuffer ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xStreamBufferIsFull ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xStreamBufferIsFullImpl         \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    BaseType_t MPU_xStreamBufferIsEmpty( StreamBufferHandle_t xStreamBuffer ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    BaseType_t MPU_xStreamBufferIsEmpty( StreamBufferHandle_t xStreamBuffer ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xStreamBufferIsEmpty ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xStreamBufferIsEmpty_Unpriv     \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    size_t MPU_xStreamBufferSpacesAvailable( StreamBufferHandle_t xStreamBuffer ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    size_t MPU_xStreamBufferSpacesAvailable( StreamBufferHandle_t xStreamBuffer ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xStreamBufferSpacesAvailable ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xStreamBufferSpacesAvailableImpl     \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    size_t MPU_xStreamBufferBytesAvailable( StreamBufferHandle_t xStreamBuffer ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    size_t MPU_xStreamBufferBytesAvailable( StreamBufferHandle_t xStreamBuffer ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xStreamBufferBytesAvailable ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xStreamBufferBytesAvailableImpl \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    BaseType_t MPU_xStreamBufferSetTriggerLevel( StreamBufferHandle_t xStreamBuffer,
                                                 size_t xTriggerLevel ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    BaseType_t MPU_xStreamBufferSetTriggerLevel( StreamBufferHandle_t xStreamBuffer,
                                                 size_t xTriggerLevel ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xStreamBufferSetTriggerLevel ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xStreamBufferSetTriggerLevelImpl \n");
        OSASM(" pop {pc}                                \n");
    }
/*-----------------------------------------------------------*/

    size_t MPU_xStreamBufferNextMessageLengthBytes( StreamBufferHandle_t xStreamBuffer ) __attribute__( ( naked ) ) FREERTOS_SYSTEM_CALL;

    size_t MPU_xStreamBufferNextMessageLengthBytes( StreamBufferHandle_t xStreamBuffer ) /* __attribute__ (( naked )) FREERTOS_SYSTEM_CALL */
    {
        OSASM(" mov r12, %0                             \n" : : "i" ( SYSTEM_CALL_xStreamBufferNextMessageLengthBytes ) : );
        OSASM(" push {r12}                              \n"); /* save syscall to stack */
        OSASM(" mrc p15, 0, r12, c13, c0, 3             \n"); /* Read TPIDRURO into r12 */
        OSASM(" mcr p15, 0, r12, c13, c0, 3             \n"); /* Write TPIDRURO into r12, fault on EL0 */
        OSASM(" pop {r12}                               \n");
        OSASM(" push {lr}                               \n");
        OSASM(" blx MPU_xStreamBufferNextMessageLengthBytesImpl \n");
        OSASM(" pop {pc}                                \n");
    }
    #endif
/*-----------------------------------------------------------*/

#endif /* ( configENABLE_MPU == 1 ) && ( configUSE_MPU_WRAPPERS_V1 == 0 ) */
