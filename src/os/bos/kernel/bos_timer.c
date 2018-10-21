/*
 *  Copyright (C) : 2015
 *  File name     : bos_timer.c
 *  Description   : Bamboo OS timer module
 *  Author        : Dang Minh Phuong
 *  Email         : kamejoko80@yahoo.com
 *
 *  This program is free software, you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include "bos_timer.h"
#include "bos_list.h"

/* External variable */
extern uint32_t bos_SystemTick;
extern uint32_t bos_Registered_Timer;

/* Global variable handles timer list */
bos_timer_t bos_timer_list;

/**
 * @brief         BOS_TimerInit
 * @param[in]     void
 * @param[in,out] bos_timer_t *timer
 * @return        void
 */
void BOS_TimerInit(bos_timer_t *timer)
{
    timer->start = false;
    timer->trigger = false;
    timer->counter = 0;
    list_add(&(timer->list), &(bos_timer_list.list));
    bos_Registered_Timer++;
}

/**
 * @brief         BOS_TimerStart
 * @param[in]     uint32_t start_time, uint32_t interval, uint32_t evt
 * @param[in,out] bos_timer_t *timer
 * @return        void
 */
void BOS_CyclicTimerStart(bos_timer_t *timer,
                          uint32_t start_time,
                          uint32_t interval,
                          uint32_t evt,
                          void (*cb_func)(void))
{
    timer->start_time = start_time;
    timer->interval   = interval;
    timer->one_shot   = false;
    timer->evt        = evt;
    timer->cb_func    = cb_func;
    timer->init_time  = bos_SystemTick; /* get current tick time */
    timer->start = true;
}

/**
 * @brief         BOS_TimerStart
 * @param[in]     uint32_t start_time, uint32_t interval, uint32_t evt
 * @param[in,out] bos_timer_t *timer
 * @return        void
 */
void BOS_OneShotTimerStart(bos_timer_t *timer,
                           uint32_t start_time,
                           uint32_t interval,
                           uint32_t evt,
                           void (*cb_func)(void))
{
    timer->start_time = start_time;
    timer->interval   = interval;
    timer->one_shot   = true;
    timer->evt        = evt;
    timer->cb_func    = cb_func;
    timer->init_time  = bos_SystemTick; /* get current tick time */
    timer->start = true;
}

/**
 * @brief         BOS_TimerStop
 * @param[in]     void
 * @param[in,out] bos_timer_t *timer
 * @return        void
 */
void BOS_TimerStop(bos_timer_t *timer)
{
    timer->start = false;
    timer->trigger = false;
    timer->counter = 0;

    if(bos_Registered_Timer > 0)
    {
        bos_Registered_Timer--;
    }

}
