/*
 *  Copyright (C) : 2015
 *  File name     : bos_mbx.c
 *  Description   : Bamboo OS mailbox module
 *  Author        : Dang Minh Phuong
 *  Email         : kamejoko80@yahoo.com
 *
 *  This program is free software, you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <stdlib.h>
#include "bos.h"
#include "bos_mbx.h"

volatile uint32_t bos_PostedMBXCount = 0;

bos_mbx_t bos_mbx_list;

/**
 * @brief         BOS_CreateMBX
 * @param[in]     uint32_t len
 * @param[in,out] void
 * @return        bos_mbx_t *
 */
void BOS_InitMBX(void)
{
    INIT_LIST_HEAD(&bos_mbx_list.list);
}

/**
 * @brief         BOS_CreateMBX
 * @param[in]     uint32_t len
 * @param[in,out] void
 * @return        bos_mbx_t *
 */
bos_mbx_t *BOS_AllocateMBX(uint32_t len)
{
    bos_mbx_t *mbx;

    /* allocate mbx data structure */
    mbx = (bos_mbx_t *)malloc(sizeof(bos_mbx_t));

    if(mbx == NULL)
    {
        return NULL;
    }

    /* allocate data buffer */
    mbx->data =(uint8_t *)malloc(len);
    mbx->len  =len;

    if(mbx->data == NULL)
    {
        /* free mbx object */
        free(mbx);
        return NULL;
    }

    return mbx;
}

/**
 * @brief         BOS_FreeMBX
 * @param[in]     bos_mbx_t *mbx
 * @param[in,out] void
 * @return        void
 */
void BOS_FreeMBX(bos_mbx_t *mbx)
{
    if(mbx != NULL)
    {
        if(mbx->data != NULL)
        {
            free(mbx->data);
        }
        free(mbx);
        mbx = NULL;
    }
}

/**
 * @brief         BOS_SendMBX
 * @param[in]     bos_mbx_t *mbx, task_t *rcv_task
 * @param[in,out] void
 * @return        void
 */
void BOS_SendMBX(bos_mbx_t *mbx, task_t *rcv_task)
{
    if(mbx == NULL)
    {
        return;
    }

    /* scheduler should not switch context here */
    BOS_EnterCritical();
    mbx->flag = MBX_POS; /* mask as sending mail */
    mbx->sender_id = BOS_GetCurrentTaskID();
    mbx->receiver_id = rcv_task->task_id;
    list_add(&(mbx->list), &(bos_mbx_list.list));
    bos_PostedMBXCount++;
    BOS_ExitCritical();
}

/**
 * @brief         BOS_ReceiveMBX
 * @param[in]     task_t *rcv_task
 * @param[in,out] void
 * @return        void
 */
void BOS_ReceiveMBX(void)
{
    task_t *task = BOS_GetCurrentTask();

    task->flags = STATE_WAIT; /* task wait */
    /* Trigger PendSV, switch to another task */
    SCB->ICSR |= (1 << 28);
}

/**
 * @brief         BOS_GetMBX
 * @param[in]     void
 * @param[in,out] void
 * @return        bos_mbx_t *
 */
bos_mbx_t *BOS_GetMBX(void)
{
    struct list_head *pos;
    bos_mbx_t *mbx;
    task_t *task = BOS_GetCurrentTask();

    /* check for each item */
    list_for_each(pos, &bos_mbx_list.list)
    {
        /* get entry from list */
        mbx = list_entry(pos, bos_mbx_t, list);

        /* check mail receive*/
        if(mbx != NULL)
        {
            if ((mbx->receiver_id == task->task_id) && (mbx->flag == MBX_POS))
            {
                list_del(&mbx->list);
                bos_PostedMBXCount--;
                return mbx;
            }
        }
    }

    return NULL;
}
