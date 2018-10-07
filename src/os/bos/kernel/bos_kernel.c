/*
 *  Copyright   : 2015
 *  File name   : bos.c
 *  Author      : Dang Minh Phuong
 *  Description : Bamboo OS kernel
 */

#include "bos.h"
#include "bos_common.h"
#include "bos_timer.h"
#include "bos_mbx.h"

/* Gobal variables */
volatile task_table_t bos_TaskTable[MAX_TASKS];
volatile uint32_t     bos_TaskCount = 0;
volatile uint32_t     bos_CurrentTaskIdx = 0;
volatile uint32_t     bos_Registered_Timer = 0;
volatile uint32_t     bos_SystemTick = 0;
volatile uint32_t     bos_pStack;
const    uint32_t     bos_StackPatternWord = STACK_PATTERN_WORD;

/* External functions */
extern void BOS_StartFirstStack(void);

/* External variable */
extern bos_timer_t bos_timer_list;
extern uint32_t bos_TaskEvt;
extern bos_mbx_t bos_mbx_list;
extern uint32_t bos_PostedMBXCount;

static void BOS_ErrorHook(void);
static void BOS_TaskExit(void);
void BOS_TaskInit(const char* name, task_t *task, void *stack, uint32_t stack_size, void *task_func, uint32_t priority);
void BOS_Start(task_t *task);
void BOS_WaitEvent(uint32_t evt_mask);
void BOS_ClearEvent(uint32_t evt_mask);
void BOS_SetEvent(uint32_t evt_mask);
bool BOS_CheckMbxForTask(uint32_t task_id);
void BOS_SaveContext(void);
void BOS_SwitchContext(void);
void BOS_LoadContext(void);

/**
 * @brief         BOS_ErrorHook
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
static void BOS_ErrorHook (void)
{
    printf("\r\nSystem error: No task ready\r\n");
    while(1);
}

#ifdef STACK_MONITOR
/**
 * @brief         BOS_ErrorHook
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
static void BOS_ErrorStackOverFlow (void)
{
    printf("\r\nSystem error: Stack over flow\r\n");
    printf("Task    : %s\r\n", bos_TaskTable[bos_CurrentTaskIdx].task->name);
    printf("Task ID : %d\r\n", (int)bos_TaskTable[bos_CurrentTaskIdx].task->task_id);
    while(1);
}

/**
 * @brief         BOS_StackMonitor
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void BOS_StackMonitor (void)
{
    uint8_t  *stack_start;
    stack_start = (uint8_t *)bos_TaskTable[bos_CurrentTaskIdx].task->stack_start;

    if(stack_start[0] != STACK_PATTERN_BYTE)
    {
        BOS_ErrorStackOverFlow();
    }
}

#endif

/**
 * @brief         BOS_TaskExit
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
static void BOS_TaskExit (void)
{
    /* Notify task terminated */
    bos_TaskTable[bos_CurrentTaskIdx].task->flags = STATE_TERM;

    /* Trigger PendSV handler */
    SCB->ICSR |= (1 << 28);
}

/**
 * @brief         BOS_TaskInit
 * @param[in]     void *task_func, uint32_t priority
 * @param[in,out] task_t *task
 * @return        void
 */
void BOS_TaskInit(const char* name, task_t *task, void *stack, uint32_t stack_size, void *task_func, uint32_t priority)
{
  stack_frame_t *process_frame;

    /* Initialize task */
    memset(task, 0, sizeof(task_t));

#ifdef STACK_MONITOR
    memset(stack, STACK_PATTERN_BYTE, stack_size);
#endif
    memcpy(task->name, name, MAX_NAME_SIZE);
    task->stack_start = stack;
    task->stack_size = stack_size;
    task->stack = (void*)(((uint32_t)stack) + stack_size - sizeof(stack_frame_t));
    process_frame = (stack_frame_t*)(task->stack);
    process_frame->r0 = 0;
    process_frame->r1 = 0;
    process_frame->r2 = 0;
    process_frame->r3 = 0;
    process_frame->r12 = 0;
    process_frame->pc = (uint32_t)task_func;
    process_frame->lr = (uint32_t)BOS_TaskExit;
    process_frame->psr = 0x21000000;

    process_frame->r4  = 0x4;
    process_frame->r5  = 0x5;
    process_frame->r6  = 0x6;
    process_frame->r7  = 0x7;
    process_frame->r8  = 0x8;
    process_frame->r9  = 0x9;
    process_frame->r10 = 0x10;
    process_frame->r11 = 0x11;

    task->flags = STATE_READY;
    task->task_id = bos_TaskCount;
    task->wait_evt = 0;
    task->expire = 0;
    task->start_time = 0;

    if (priority > LOWEST_PRIORITY)
    {
        task->priority = LOWEST_PRIORITY;
    }
    else
    {
        task->priority = priority;
    }

    if(bos_TaskCount < MAX_TASKS)
    {
        bos_TaskTable[bos_TaskCount].task = task;
        bos_TaskCount++;
    }
    else
    {
        printf("Error number of task exceed max limit\r\n");
    }
}

/**
 * @brief         BOS_Start
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */

#define NVIC_SYSPRI2              ((volatile uint32_t *) 0xe000ed20)
#define MIN_INTERRUPT_PRIORITY    ( 255UL )
#define NVIC_PENDSV_PRI           ( MIN_INTERRUPT_PRIORITY << 16UL )
#define NVIC_SYSTICK_PRI          ( MIN_INTERRUPT_PRIORITY << 24UL )

void BOS_Start (task_t *task)
{
    /* Update current task idx */
    bos_CurrentTaskIdx = task->task_id;

    /* Init timer list */
    INIT_LIST_HEAD(&bos_timer_list.list);

    /* Init mail box */
    BOS_InitMBX();

    /* Make PendSV, CallSV and SysTick the same priority as the kernel. */
    *(NVIC_SYSPRI2) |= NVIC_PENDSV_PRI;
    *(NVIC_SYSPRI2) |= NVIC_SYSTICK_PRI;

    /* 1ms tick */
    SysTick_Config(SystemCoreClock/1000);

    BOS_StartFirstStack();

}

/**
 * @brief         BOS_TaskDelay
 * @param[in]     uint32_t tick
 * @param[in,out] task_t *task
 * @return        void
 */
void BOS_Delay (uint32_t tick)
{
    task_t *task = BOS_GetCurrentTask();
    if (tick > 0)
    {
        task->flags = STATE_WAIT; /* task wait */
        task->start_time =  bos_SystemTick;
        task->expire = tick;

        /* Trigger PendSV, switch to another task */
        SCB->ICSR |= (1 << 28);
    }
}

/**
 * @brief         BOS_SaveContext
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void BOS_SaveContext (void)
{
	bos_TaskTable[bos_CurrentTaskIdx].task->stack = (void *) bos_pStack;
}

/**
 * @brief         BOS_SwitchContext
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void BOS_SwitchContext (void)
{
    uint32_t i, task_id;
    uint32_t task_found = 0;
    uint32_t task_idx;
    uint32_t task_priority = LOWEST_PRIORITY;
    uint32_t flags, wait_evt, start_time, expire;

    /* check task READY or WAIT with highest priority */
    for(i = 0; i < bos_TaskCount; i++)
    {
        flags      = bos_TaskTable[i].task->flags;
        wait_evt   = bos_TaskTable[i].task->wait_evt;
        start_time = bos_TaskTable[i].task->start_time;
        expire     = bos_TaskTable[i].task->expire;
        task_id    = bos_TaskTable[i].task->task_id;

        if(( flags == STATE_READY) ||
            ( (flags == STATE_WAIT) &&
            ( (wait_evt & bos_TaskEvt) || /* check incoming event */
                ((expire > 0) && ((bos_SystemTick - start_time) >= expire)) || /* check delay */
                (BOS_CheckMbxForTask(task_id) == true) /* check incoming mailbox */
            )
            )
          )
        {
            if(bos_TaskTable[i].task->priority <= task_priority)
            {
                task_priority = bos_TaskTable[i].task->priority;
                task_idx = i;
                task_found = 1;
            }
        }
    }

    /* if found switch context */
    if (task_found)
    {
        /* Change previous preemted stack to ready state */
        if(bos_TaskTable[bos_CurrentTaskIdx].task->flags == STATE_RUN)
        {
            bos_TaskTable[bos_CurrentTaskIdx].task->flags = STATE_READY;
        }

        bos_CurrentTaskIdx = task_idx;
        bos_TaskTable[task_idx].task->flags = STATE_RUN;

        /* if task delay then reset expire and start_time */
        if (bos_TaskTable[task_idx].task->expire > 0)
        {
            bos_TaskTable[task_idx].task->expire = 0;
            bos_TaskTable[task_idx].task->start_time = 0;
        }
    }
    else
    {
        BOS_ErrorHook();
    }
}

/**
 * @brief         BOS_LoadContext
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void BOS_LoadContext (void)
{
	bos_pStack = (uint32_t)bos_TaskTable[bos_CurrentTaskIdx].task->stack;
}

/**
 * @brief         BOS_CheckMbxForTask
 * @param[in]     task_id
 * @param[in,out] void
 * @return        bool
 */
bool BOS_CheckMbxForTask(uint32_t task_id)
{
    struct list_head *pos;
    bos_mbx_t *mbx;

    if (bos_PostedMBXCount)
    {
        /* check for each item */
        list_for_each(pos, &bos_mbx_list.list)
        {
            /* get entry from list */
            mbx = list_entry(pos, bos_mbx_t, list);

            /* check mail receive*/
            if ((mbx->receiver_id == task_id) && (mbx->flag == MBX_POS))
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief         BOS_ExamineContextSwitch
 * @param[in]     void
 * @param[in,out] uint32_t *priority
 * @return        uint32_t
 */
uint32_t BOS_ExamineContextSwitch (uint32_t *priority)
{
    uint32_t i, task_id;
    uint32_t task_found = 0;
    uint32_t task_priority = LOWEST_PRIORITY;
    uint32_t flags, wait_evt, start_time, expire;

    /* check task READY or WAIT with highest priority */
    for(i = 0; i < bos_TaskCount; i++)
    {
        flags      = bos_TaskTable[i].task->flags;
        wait_evt   = bos_TaskTable[i].task->wait_evt;
        start_time = bos_TaskTable[i].task->start_time;
        expire     = bos_TaskTable[i].task->expire;
        task_id    = bos_TaskTable[i].task->task_id;

        if((flags == STATE_READY) ||
            ( (flags == STATE_WAIT) &&
            ( (wait_evt & bos_TaskEvt) || /* check incoming event */
              ((expire > 0) && ((bos_SystemTick - start_time) >= expire)) || /* check delay */
               (BOS_CheckMbxForTask(task_id) == true) /* check incoming mailbox */
            )
            )
        )
        {
            if(bos_TaskTable[i].task->priority < task_priority)
            {
                task_priority = bos_TaskTable[i].task->priority;
                task_found = 1;
            }
        }
    }

    *priority = task_priority;

    return task_found;
}

/**
 * @brief         BOS_Schedule
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
static void BOS_Schedule (void)
{
    uint32_t task_Priority;
    uint32_t cur_TaskPriority;

    cur_TaskPriority = bos_TaskTable[bos_CurrentTaskIdx].task->priority;

    /* preempt current task if higher priority task is available */
    if(BOS_ExamineContextSwitch(&task_Priority))
    {
        if(task_Priority < cur_TaskPriority)
        {
            /* Trigger PendSV handler to switch context */
            SCB->ICSR |= (1 << 28);
        }
    }
}

/**
 * @brief         BOS_TimerHandle
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
static void BOS_TimerHandle (void)
{
    struct list_head *pos;
    bos_timer_t *timer;

    /* return if there is no registered timer */
    if (!bos_Registered_Timer)
    {
        return;
    }

    /* check for each item */
    list_for_each(pos, &bos_timer_list.list)
    {
        /* get entry from list */
        timer = list_entry(pos, bos_timer_t, list);

        /* precess only timer which has been started already */
        if(timer->start)
        {
            /* Timer start but not trigger */
            if(!timer->trigger)
            {
                if((timer->init_time + timer->start) <= bos_SystemTick)
                {
                    timer->trigger = true; /* trigger it */
                }
            }
            else
            {
                timer->counter++;
            }

            /* check whether timer expire */
            if(timer->counter >= timer->interval)
            {
                /* Set event if it has */
                if(timer->evt)
                {
                    BOS_SetEvent (timer->evt);
                }

                /* Execute call back function */
                if(timer->cb_func != NULL)
                {
                    timer->cb_func();
                }

                /* disable timer if oneshot */
                if(timer->one_shot)
                {
                    timer->start = false;
                }

                /* Reset timer counter */
                timer->counter = 0;
            }
        }
    }
}

/**
 * @brief         SysTick_Handler
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void SysTick_Handler (void)
{
	bos_SystemTick++;
	BOS_TimerHandle();
	BOS_Schedule();
}

