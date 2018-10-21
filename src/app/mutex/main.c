/*
 *  Copyright (C) : 2018
 *  File name     : main.c
 *  Description   : Bamboo OS mutex demo program
 *  Author        : Dang Minh Phuong
 *  Email         : kamejoko80@yahoo.com
 *
 *  This program is free software, you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <stdio.h>
#include "stm32f4_discovery.h"
#include "stm32f4xx_conf.h"
#include "bos.h"
#include "bos_shell.h"
#include "uart_debug.h"
#include "trace.h"

#define USE_MUTEX_PROTECTION

/*
 * Task create
 */
TASK_CREATE(task_01,        1024);
TASK_CREATE(task_02,        1024);
TASK_CREATE(task_sys_trace, 2048);
TASK_CREATE(task_idle,      1024);

#if defined(USE_MUTEX_PROTECTION)
/*
 * Mutex lock create
 */
MUTEX_CREATE(mutex);
#endif

/**
 * @brief         Counter
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void Counter(void)
{
    int i;
    task_t *task;

    /* Get current task object */
    task = BOS_GetCurrentTask();

#if defined(USE_MUTEX_PROTECTION)
    /* Mutex lock request */
    BOS_MutexLock(&mutex);

    SysTrace_Message("=======> Task ID %d has occupied the lock", task->task_id);
#endif

    for(i = 1; i <= 20; i++)
    {
       SysTrace_Message("Counter %d", i);
       BOS_Delay(2);
    }

#if defined(USE_MUTEX_PROTECTION)
    /* Mutex lock release */
    BOS_MutexUnLock(&mutex);
#endif

}

/**
 * @brief         task_01_func
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void task_01_func (void)
{
    Counter();
}

/**
 * @brief         task_02_func
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void task_02_func (void)
{
    BOS_Delay(5);
    Counter();
}

/**
 * @brief         task_sys_trace_func
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void task_sys_trace_func (void)
{
    while(1)
    {
        /* Manage log trace queue event */
        SysTrace_ManageMsgQueueEvent();
    }
}

/**
 * @brief         task_idle_func
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void task_idle_func (void)
{
    /* Important, system idle must be defined */
    while(1)
    {
        __asm("nop");
    }
}

/**
 * @brief         main
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
int main(void)
{
    uart_debug_init();
    SysTrace_Init();
    STM_EVAL_LEDInit(LED3);

    TASK_INIT(task_01, task_01_func, 3);
    TASK_INIT(task_02, task_02_func, 2);
    TASK_INIT(task_sys_trace, task_sys_trace_func, 4);
    TASK_INIT(task_idle, task_idle_func, 7);
    BOS_InitShell();
    BOS_Start(&task_idle);

    /* This should not be run */
    while(1)
    {

    }
}
