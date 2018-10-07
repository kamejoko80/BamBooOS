/*
**
**                           Main.c
**
**
**********************************************************************/
/*
   Last committed:     $Revision : 1.2
   Last changed by:    $Author   : Dang Minh Phuong
   Last changed date:  $Date     : 20160427
   ID:                 $Id       : 00

**********************************************************************/
#include <stdio.h>
#include "stm32f4_discovery.h"
#include "stm32f4xx_conf.h"
#include "bos.h"
#include "bos_shell.h"
#include "uart_debug.h"
#include "trace.h"

TASK_CREATE(task_01,        1024);
TASK_CREATE(task_02,        1024);
TASK_CREATE(task_03,        256);
TASK_CREATE(task_04,        256);
TASK_CREATE(task_06,        1024);
TASK_CREATE(task_07,        1024);
TASK_CREATE(task_sys_trace, 1024);
TASK_CREATE(task_idle,      256);

/**
 * @brief         task_01_func
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void task_01_func (void)
{
    while (1)
    {
        BOS_WaitEvent(EVT_MASK(1));
        BOS_ClearEvent(EVT_MASK(1));
    }
}

/**
 * @brief         task_02_func
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void task_02_func (void)
{
    while (1)
    {
        BOS_WaitEvent(EVT_MASK(2));
        BOS_ClearEvent(EVT_MASK(2));
        STM_EVAL_LEDToggle(LED3);
    }
}

/**
 * @brief         task_03_func
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void task_03_func (void)
{
    while(1)
    {
        BOS_Delay(50);
        BOS_SetEvent(EVT_MASK(1));
    }
}

/**
 * @brief         task_04_func
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void task_04_func (void)
{
#if 0
    while(1)
    {
        BOS_Delay(100);
        BOS_SetEvent(EVT_MASK(2));
    }
#endif
}

/**
 * @brief         task_06_func
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void task_06_func (void)
{
#if 1
    // Allocate mailbox
    bos_mbx_t *mbx = BOS_AllocateMBX(4);

    mbx->data[0] = 0x00;
    mbx->data[1] = 0x11;
    mbx->data[2] = 0x22;
    mbx->data[3] = 0x33;

    BOS_SendMBX(mbx, &task_07);

    SysTrace_Message("task_06 sent an email");

    /* Task 06 will be terminated after sending email */
#endif

}

/**
 * @brief         task_06_func
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void task_07_func (void)
{
    bos_mbx_t *mbx;
    uint32_t i;

    while(1)
    {
        /* wait for incoming mailbox */
        BOS_ReceiveMBX();

        SysTrace_Message("task_07 received email");

        /* Get email */
        mbx = BOS_GetMBX();

        for(i = 0; i < mbx->len; i++)
        {
            SysTrace_Message("     data[%d] = 0x%X", i, mbx->data[i]);
        }

        /* Free mailbox */
        BOS_FreeMBX(mbx);
    }
}

void timer_cb(void)
{
    SysTrace_Message("One shot timer callback func");
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
    bos_timer_t timer;
    bos_timer_t led_timer;

    BOS_TimerInit(&timer);
    BOS_TimerInit(&led_timer);

    BOS_OneShotTimerStart(&timer, 0, 100, 0, &timer_cb);
    BOS_CyclicTimerStart(&led_timer, 0, 500, EVT_MASK(2), NULL);

    /* Important, system idle must be defined */
    while(1)
    {
        __asm("nop");
    }
}


int main(void)
{
    uart_debug_init();
    SysTrace_Init();
    STM_EVAL_LEDInit(LED3);

    TASK_INIT(task_01, task_01_func, 1);
    TASK_INIT(task_02, task_02_func, 0);
    TASK_INIT(task_03, task_03_func, 2);
    TASK_INIT(task_04, task_04_func, 2);
    TASK_INIT(task_06, task_06_func, 4);
    TASK_INIT(task_07, task_07_func, 4);
    TASK_INIT(task_sys_trace, task_sys_trace_func, 4);
    TASK_INIT(task_idle, task_idle_func, 7);
    BOS_InitShell();
    BOS_Start(&task_idle);

    /* This should not be run */
    while(1)
    {

    }
}
