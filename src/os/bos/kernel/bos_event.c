/*
 *  Copyright (C) : 2015
 *  File name     : bos_event.c
 *  Description   : Bamboo OS event module
 *  Author        : Dang Minh Phuong
 *  Email         : kamejoko80@yahoo.com
 *
 *  This program is free software, you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "bos.h"

volatile  uint32_t bos_TaskEvt = 0;


/**
 * @brief         BOS_WaitEvent
 * @param[in]     task_t *task, uint32_t evt_mask
 * @param[in,out] void
 * @return        void
 */
void BOS_WaitEvent (uint32_t evt_mask)
{
    task_t *task = BOS_GetCurrentTask();

    task->flags = STATE_WAIT; /* task wait */
    task->wait_evt = evt_mask;

    /* Trigger PendSV, switch to another task */
    SCB->ICSR |= (1 << 28);
}

/**
 * @brief         BOS_ClearEvent
 * @param[in]     uint32_t evt_mask
 * @param[in,out] void
 * @return        void
 */
void BOS_ClearEvent (uint32_t evt_mask)
{
    BOS_EnterCritical();
    bos_TaskEvt &= ~evt_mask;
    BOS_ExitCritical();
}

/**
 * @brief         BOS_SetEvent
 * @param[in]     uint32_t evt_mask
 * @param[in,out] void
 * @return        void
 */
void BOS_SetEvent (uint32_t evt_mask)
{
    BOS_EnterCritical();
    bos_TaskEvt |= evt_mask;
    BOS_ExitCritical();
}

