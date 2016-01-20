#ifndef __BOS_H__
#define __BOS_H__

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "bos_list.h"
#include "bos_common.h"
#include "bos_timer.h"
#include "bos_shell.h"
#include "bos_mbx.h"

/* BOS user config */
#define MAX_TASKS       10
#define MAX_NAME_SIZE   32 

/* Task state */
#define STATE_TERM      0x00000000 
#define STATE_READY     0x00000001 
#define STATE_RUN       0x00000002  
#define STATE_WAIT      0x00000003

/* Stack monitor option */
#define STACK_MONITOR       1 
#define STACK_PATTERN_BYTE  0xAA
#define STACK_PATTERN_WORD  0xAAAAAAAA

/* OS Version */
#define BOS_VERSION     "1.3"
#define COMPILE_DATE    __DATE__
#define COMPILE_TIME    __TIME__
#define CPU_VARIANT     "ARM Cortex M0+"
#define TARGET_NAME     "KL46Z256"
#define BOARD_NAME      "FRDM-KL46Z"

/* Event mask */
#define EVT_MASK(x)     (1 << (x))

#define LOWEST_PRIORITY 7 

#define CMXOS_OK        1
#define CMXOS_NOK       0 

#define BOS_EnterCritical() __disable_irq()
#define BOS_ExitCritical()  __enable_irq()

#define PRINT(...) (BOS_EnterCritical(),printf(__VA_ARGS__),BOS_ExitCritical())

#define TASK_CREATE(task, size) uint8_t task##_stack[size] __attribute__((aligned (4)));\
                                const uint32_t task##_stack_size = size;\
                                const char* task##name = #task;\
                                task_t task;\
                                task_t *p##task = &task;

#define TASK_INIT(task, func, pri) BOS_TaskInit(task##name, p##task, (void*)task##_stack, task##_stack_size, func, pri)

typedef struct
{
    /* sw stack frame */
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;

    /* hardware stack frame */
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;

} stack_frame_t;

typedef struct
{
    char    name[MAX_NAME_SIZE];
    void    *stack;
    void    *stack_start;
    uint32_t stack_size;     
    uint32_t flags;
    uint32_t wait_evt;
    uint32_t task_id;
    uint32_t start_time;
    uint32_t expire;
    uint32_t priority;
} task_t;

typedef struct
{
    task_t *task;
} task_table_t;

uint32_t BOS_GetCurrentTaskID(void);
task_t * BOS_GetCurrentTask(void);

void BOS_TaskInit(const char* name, task_t *task, void *stack, uint32_t stack_size, void *task_func, uint32_t priority);
void BOS_Start(task_t *task);
void BOS_Delay(uint32_t tick);

void BOS_WaitEvent (uint32_t evt_mask);
void BOS_ClearEvent (uint32_t evt_mask);
void BOS_SetEvent (uint32_t evt_mask);

void BOS_InitMBX(void);
bos_mbx_t *BOS_AllocateMBX(uint32_t len);
void BOS_FreeMBX(bos_mbx_t *mbx);
void BOS_SendMBX(bos_mbx_t *mbx, task_t *rcv_task);
void BOS_ReceiveMBX(void);
bos_mbx_t *BOS_GetMBX(void);

#endif /* __BOS_H__ */
