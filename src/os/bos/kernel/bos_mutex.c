/*
 *  Copyright (C) : 2018
 *  File name     : bos_mutex.c
 *  Description   : Bamboo OS mutex module
 *  Author        : Dang Minh Phuong
 *  Email         : kamejoko80@yahoo.com
 *
 *  This program is free software, you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include "bos_mutex.h"
#include "bos_list.h"
#include "trace.h"
#include "bos.h"

/* Global variable handles timer list */
bos_mutex_t bos_mutex_list;

void BOS_MutexInit(void)
{
    /* Init mutex list */
    INIT_LIST_HEAD(&bos_mutex_list.list);
    bos_mutex_list.locked = false;
    bos_mutex_list.locked_by_task_id = 0;
    memset(bos_mutex_list.suspend_task_id, 0, MAX_SUSPEND_TASK);
}

void BOS_MutexAddSuspendTaskID(bos_mutex_t *mutex, uint8_t task_id)
{
    uint8_t i;
    bool found = false;

    for(i = 0; i < MAX_SUSPEND_TASK; i++)
    {
        if(mutex->suspend_task_id[i] == 0)
        {
            found = true;
            mutex->suspend_task_id[i] = task_id;
            break;
        }
    }

    if (!found)
    {
        printf(" Error, could not add task into mutex suspend list\r\n");

        /* let system hang at here */
        BOS_EnterCritical();
        while(1);
    }
}

void BOS_MutexRemoveSuspendTaskID(bos_mutex_t *mutex, uint8_t task_id)
{
    uint8_t i;

    for(i = 0; i < MAX_SUSPEND_TASK; i++)
    {
        if(mutex->suspend_task_id[i] == task_id)
        {
            mutex->suspend_task_id[i] = 0;
        }
    }
}

bool BOS_MutexGetStatusSuspendTaskID(bos_mutex_t *mutex, uint8_t task_id)
{
    uint8_t i;
    bool ret = false;

    for(i = 0; i < MAX_SUSPEND_TASK; i++)
    {
        if(mutex->suspend_task_id[i] == task_id)
        {
            ret = true;
            break;
        }
    }

    return ret;
}

/**
 * @brief         BOS_MutexLock
 * @param[in]     void
 * @param[in,out] bos_mutex_t *mutex
 * @return        void
 */
void BOS_MutexLock(bos_mutex_t *mutex)
{
    struct list_head *pos;
    bos_mutex_t *mtx;
    bool found = false;
    task_t *task;

    /* get current task TCB */
    task = BOS_GetCurrentTask();

    /* check for each item */
    list_for_each(pos, &bos_mutex_list.list)
    {
        /* get entry from list */
        mtx = list_entry(pos, bos_mutex_t, list);

        if (mtx == mutex)
        {
            found = true;
        }
    }

    /* Add mutex into the list */
    if (!found)
    {
        BOS_EnterCritical();
        list_add(&mutex->list, &bos_mutex_list.list);
        BOS_ExitCritical();
    }

    /* check if the task has already gotten the lock */
    if((mutex->locked == true) && (mutex->locked_by_task_id == task->task_id))
    {
        return;
    }

    /*
     * Add task id into suspend list
     * Switch context if other task has taken the mutex lock
     */
    if((mutex->locked == true) && (mutex->locked_by_task_id != task->task_id))
    {
        BOS_MutexAddSuspendTaskID(mutex, task->task_id);

        task->flags = STATE_WAIT; /* task wait */

        /* Trigger PendSV, switch to another task */
        SCB->ICSR |= (1 << 28);
    }

    /* To make sure other tasks released the lock */
    while((mutex->locked == true) && (mutex->locked_by_task_id != task->task_id))
    {

    }

    /* task resumes to get the mutex lock here */
    BOS_EnterCritical();
    mutex->locked = true;
    mutex->locked_by_task_id = task->task_id;
    BOS_MutexRemoveSuspendTaskID(mutex, task->task_id);
    BOS_ExitCritical();
}

/**
 * @brief         BOS_MutexUnLock
 * @param[in]     void
 * @param[in,out] bos_mutex_t *mutex
 * @return        void
 */
void BOS_MutexUnLock(bos_mutex_t *mutex)
{
    struct list_head *pos;
    bos_mutex_t *mtx;
    task_t *task;

    /* get current task TCB */
    task = BOS_GetCurrentTask();

    /* check for each item */
    list_for_each(pos, &bos_mutex_list.list)
    {
        /* get entry from list */
        mtx = list_entry(pos, bos_mutex_t, list);

        if (mtx == mutex)
        {
            if((mtx->locked == true) && (mtx->locked_by_task_id == task->task_id))
            {
                /* remove mutex entry */
                BOS_EnterCritical();

                /* reset mutex lock */
                mtx->locked = false;
                mtx->locked_by_task_id = 0;
                BOS_ExitCritical();

                return;
            }
        }
    }
}

/**
 * @brief         BOS_MutexStatusReleased
 * @param[in]     task_t *task
 * @param[in,out] void
 * @return        bool
 */
bool BOS_MutexStatusReleased(task_t *task)
{
    struct list_head *pos;
    bos_mutex_t *mtx;

    if(list_empty(&bos_mutex_list.list))
    {
        return false;
    }

    /* check for each item */
    list_for_each(pos, &bos_mutex_list.list)
    {
        /* get entry from list */
        mtx = list_entry(pos, bos_mutex_t, list);

        if(mtx != NULL)
        {
            /* Check if the mutex has been released the lock by other tasks */
            if ((mtx->locked == false) && (BOS_MutexGetStatusSuspendTaskID(mtx, task->task_id)))
            {
                return true;
            }
        }
    }

    return false;
}

