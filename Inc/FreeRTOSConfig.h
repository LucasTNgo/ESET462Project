// ************************
//  FreeRTOS Config
// ************************
// Required by RP2350 port

#pragma once

// Required by RP2350 port
#define configENABLE_MPU                        0
#define configENABLE_TRUSTZONE                  0
#define configRUN_FREERTOS_SECURE_ONLY          1
#define configENABLE_FPU                        1
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    16

// Hardware
#define configCPU_CLOCK_HZ                      150000000

// Required by FreeRTOS.h
#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_16_BIT_TICKS                  0
#define configMINIMAL_STACK_SIZE                256
#define configMAX_PRIORITIES                    5
#define configTICK_RATE_HZ                      1000
#define configTOTAL_HEAP_SIZE                   (128 * 1024)
#define configUSE_PASSIVE_IDLE_HOOK         0
#define configUSE_CORE_AFFINITY             1
#define configRUN_MULTIPLE_PRIORITIES       1

// Assert
#define configASSERT(x) if((x) == 0) { portDISABLE_INTERRUPTS(); for(;;); }

//#define configNUM_CORES                         2
#define configNUMBER_OF_CORES               2

// Timers
#define INCLUDE_xTimerPendFunctionCall  1
#define configUSE_TIMERS                1
#define configTIMER_TASK_PRIORITY       (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH        10
#define configTIMER_TASK_STACK_DEPTH    256

#define configSUPPORT_PICO_SYNC_INTEROP     1
#define configSUPPORT_PICO_TIME_INTEROP     1