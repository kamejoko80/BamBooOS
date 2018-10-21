/*
 *  Copyright (C) : 2015
 *  File name     : bos_common.c
 *  Description   : Bamboo OS kernel
 *  Author        : Dang Minh Phuong
 *  Email         : kamejoko80@yahoo.com
 *
 *  This program is free software, you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <stdio.h>
#include <string.h>
#include "bos.h"

extern task_table_t bos_TaskTable[];
extern uint32_t     bos_CurrentTaskIdx;

/**
 * @brief         BOS_GetCurrentTaskID
 * @param[in]     void
 * @param[in,out] void
 * @return        uint32_t current task id
 */
uint32_t BOS_GetCurrentTaskID(void)
{
	return bos_TaskTable[bos_CurrentTaskIdx].task->task_id;
}

/**
 * @brief         BOS_GetCurrentTask
 * @param[in]     void
 * @param[in,out] void
 * @return        task_t *
 */
task_t * BOS_GetCurrentTask(void)
{
	return bos_TaskTable[bos_CurrentTaskIdx].task;
}
