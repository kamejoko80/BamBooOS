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
