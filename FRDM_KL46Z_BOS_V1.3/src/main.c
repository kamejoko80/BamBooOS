/*
 *  Copyright   : 2015 
 *  File name   : main.c
 *	Author      : Dang Minh Phuong
 *	Description : Bamboo OS demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MKL46Z4.h"
#include "uart.h"
#include "bos.h"
#include "board.h"
#include "i2c.h"
#include "mma8451.h"
#include "mag3110.h"


#define  EVT_UART_RECEIVE EVT_MASK(7)

TASK_CREATE(task_01,   256);
TASK_CREATE(task_02,   256);
TASK_CREATE(task_03,   256);
TASK_CREATE(task_04,   256);
TASK_CREATE(task_06,   256);
TASK_CREATE(task_07,   256);
TASK_CREATE(task_idle, 1024);

extern void Init_Led (void);

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
        FPTE->PTOR |= RED_LED_PIN;
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
        FPTD->PTOR |= GREEN_LED_PIN;
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
    // Allocate mailbox
    bos_mbx_t *mbx = BOS_AllocateMBX(4);

    mbx->data[0] = 0x00;
    mbx->data[1] = 0x11;
    mbx->data[2] = 0x22;
    mbx->data[3] = 0x33;

    BOS_SendMBX(mbx, &task_07);

    PRINT("task_06 sent an email\r\n");

    /* Task 06 will be terminated after sending email */
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
        /* wait for incomming mailbox */
        BOS_ReceiveMBX();

        PRINT("\r\n task_07 received email\r\n");

        /* Get email */
        mbx = BOS_GetMBX();
        
        for(i = 0; i < mbx->len; i++)
        {
            PRINT(" %X", mbx->data[i]);
        }
        
        PRINT("\r\n");
        
        /* Free mailbox */
        BOS_FreeMBX(mbx);
    }
}

#if 0
void timer_cb(void)
{
    printf("one shot timer callback func 01\r\n");
}
#endif

/**
 * @brief         task_idle_func
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void task_idle_func (void)
{
    //bos_timer_t timer;
    bos_timer_t led_timer;

    //BOS_TimerInit(&timer);
    BOS_TimerInit(&led_timer);
    
    //BOS_OneShotTimerStart(&timer, 0, 100, 0, &timer_cb);	
    BOS_CyclicTimerStart(&led_timer, 0, 500, EVT_MASK(2), NULL);
    
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
int main (void)
{
    SystemCoreClockUpdate();

    UART0_Init();
    Init_Led();
    i2c_init(I2C0);
    mma8451_init();
    mag3110_init();  

    TASK_INIT(task_01, task_01_func, 1);
    TASK_INIT(task_02, task_02_func, 0);
    TASK_INIT(task_03, task_03_func, 2);
    TASK_INIT(task_04, task_04_func, 2);
    TASK_INIT(task_06, task_06_func, 4);
    TASK_INIT(task_07, task_07_func, 4);
    TASK_INIT(task_idle, task_idle_func, 7);
    BOS_InitShell();
    BOS_Start(&task_idle);
    
    for(;;);
}
