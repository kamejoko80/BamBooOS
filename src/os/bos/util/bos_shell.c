/*
 *  Copyright (C) : 2015
 *  File name     : bos_shell.c
 *  Description   : Bamboo OS shell module
 *  Author        : Dang Minh Phuong
 *  Email         : kamejoko80@yahoo.com
 *
 *  This program is free software, you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bos.h"
#include "input.h"

/* Shell message input completed event */
#define EVT_UART_RECEIVE EVT_MASK(30)

#define LINE_SPACE_01 4
#define LINE_SPACE_02 20
#define LINE_SPACE_03 15
#define LINE_SPACE_04 15

#define MAX_INPUT_SIZE 64

#define SHELL_PROMPT() PRINT("\r\nBOS# ")

extern uint32_t bos_TaskCount;
extern task_table_t bos_TaskTable[];

/* For asynchronous uart receive */
static char g_rx_buffer[MAX_INPUT_SIZE];

/* declare task variables */
TASK_CREATE(shell, 1024);

// http://ascii-table.com/ansi-escape-sequences-vt-100.php
void go_left(uint32_t n)
{
    char int_str[50];
    /* change n to string */
    sprintf(int_str, "%d", (int)n);
    // ESC[ValueD
    BOS_EnterCritical();

    printf("%c", 27);
    printf("%c", 0x5B);
    printf("%s", int_str);
    printf("D");
    BOS_ExitCritical();
}

void go_right(uint32_t n)
{
    char int_str[50];
    /* change n to string */
    sprintf(int_str, "%d", (int)n);
    // ESC[ValueC
    BOS_EnterCritical();
    printf("%c", 27);
    printf("%c", 0x5B);
    printf("%s", int_str);
    printf("C");
    BOS_ExitCritical();
}

void cursor_blink(void)
{
    // Esc[5m
    BOS_EnterCritical();
    printf("%c", 27);
    printf("%c", 0x5B);
    printf("5");
    printf("m");
    BOS_ExitCritical();
}

uint32_t num_len(uint32_t n)
{
    char int_str[50];
    /* change n to string */
    sprintf(int_str, "%d", (int)n);
    return strlen(int_str);
}

/**
 * @brief         Calculate stack free memory
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
uint32_t task_free (uint32_t task_idx)
{
    uint32_t i;
    uint32_t free_size = 0;
    uint8_t *stack_mem;

    task_t *task = bos_TaskTable[task_idx].task;
    stack_mem = (uint8_t*)task->stack_start;

    for(i = 0; i < task->stack_size; i++)
    {
        if(stack_mem[i] == STACK_PATTERN_BYTE)
        {
            free_size++;
        }
        else
        {
            return free_size;
        }
    }
    return free_size;
}

/**
 * @brief         rx_complete
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void rx_complete(char *buffer, uint32_t len)
{
    /* UART0 receive callback function */
    BOS_SetEvent(EVT_UART_RECEIVE);
}

/**
 * @brief         check_cmd
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
bool check_cmd (char *str, const char *cmd)
{
    if(strstr(str, cmd)!=NULL)
    {
        return true;
    }
    return false;
}

/**
 * @brief         cmd_help
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void cmd_help (void)
{
    PRINT("\r\n");
    PRINT("    info    print system info\r\n");
    PRINT("    ps      print task list\r\n");
#if 0
    PRINT("    mbx     send email to another task\r\n");
#endif
    SHELL_PROMPT();
    SHELL_PROMPT();
}

/**
 * @brief         cmd_ps
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void cmd_ps (void)
{
    uint32_t i;
    uint32_t len;
    uint32_t task_free_size;

    PRINT("\r\n");

    PRINT("ID");
    go_right(LINE_SPACE_01 - strlen("ID"));

    PRINT("TASK_NAME");
    go_right(LINE_SPACE_02 - strlen("TASK_NAME"));

    PRINT("STACK_SIZE");
    go_right(LINE_SPACE_03 - strlen("STACK_SIZE"));

    PRINT("STACK_FREE");
    go_right(LINE_SPACE_04 - strlen("STACK_FREE"));

    PRINT("STATE");
    PRINT("\r\n\r\n");

    for (i = 0; i < bos_TaskCount; i++)
    {
        PRINT("%d", (int)bos_TaskTable[i].task->task_id);
        len = num_len(bos_TaskTable[i].task->task_id);
        go_right(LINE_SPACE_01 - len);

        PRINT("%s", bos_TaskTable[i].task->name);
        len = strlen(bos_TaskTable[i].task->name);
        go_right(LINE_SPACE_02 - len);

        PRINT("%d", (int)bos_TaskTable[i].task->stack_size);
        len = num_len(bos_TaskTable[i].task->stack_size);
        go_right(LINE_SPACE_03 - len);

        task_free_size = task_free(i);
        PRINT("%d", (int)task_free_size);
        len = num_len(task_free_size);
        go_right(LINE_SPACE_04 - len);

        switch (bos_TaskTable[i].task->flags)
        {
            case STATE_TERM:
                PRINT("[T]");     // Terminated
            break;
            case STATE_READY:
                PRINT("[R]");     // Ready
            break;
            case STATE_RUN:
                PRINT("[A]");     // Active
            break;
            case STATE_WAIT:
                PRINT("[W]");     // Wait
            break;
            default:
                PRINT("[U]");     // Unknow
            break;
        }
        PRINT("\r\n");
    }

    SHELL_PROMPT();
}

/**
 * @brief         print system info
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void cmd_info (void)
{
    PRINT("\r\n");
    PRINT("BOS Version : %s\r\n", BOS_VERSION);
    PRINT("Build time  : %s %s\r\n", COMPILE_DATE, COMPILE_TIME);
    PRINT("CPU variant : %s\r\n", CPU_VARIANT);
    PRINT("Target name : %s\r\n", TARGET_NAME);
    PRINT("Board name  : %s\r\n", BOARD_NAME);
    SHELL_PROMPT();
}

#if 0
/**
 * @brief         cmd_mbx
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void cmd_mbx (void)
{
    extern task_t task_07;

    // Allocate mailbox
    bos_mbx_t *mbx = BOS_AllocateMBX(4);

    mbx->data[0] = 0x44;
    mbx->data[1] = 0x55;
    mbx->data[2] = 0x66;
    mbx->data[3] = 0x77;

    BOS_SendMBX(mbx, &task_07);
    SHELL_PROMPT();
}
#endif

/**
 * @brief         shell_func
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void shell_func (void)
{
    //cursor_blink();

    while(1)
    {
        //SHELL_PROMPT();

        Input_GetsAsync((char *)g_rx_buffer, MAX_INPUT_SIZE, &rx_complete);
        BOS_WaitEvent(EVT_UART_RECEIVE);
        BOS_ClearEvent(EVT_UART_RECEIVE);

        if(check_cmd((char *)g_rx_buffer, "help"))
        {
            cmd_help();
        }
        else if(check_cmd((char *)g_rx_buffer, "info"))
        {
            cmd_info();
        }
        else if(check_cmd((char *)g_rx_buffer, "ps"))
        {
            cmd_ps();
        }
#if 0
        else if(check_cmd((char *)g_rx_buffer, "mbx"))
        {
            cmd_mbx();
        }
#endif
        else
        {
            PRINT("\r\nCommand not found, type help for more details\r\n");
            SHELL_PROMPT();
        }
    }
}

/**
 * @brief         BOS_StartShell
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void BOS_InitShell(void)
{
    TASK_INIT(shell, shell_func, 6);
}
