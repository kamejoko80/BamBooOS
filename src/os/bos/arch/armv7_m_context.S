/*
 *  Copyright   : 2018
 *  File name   : armv7_m_context.S
 *  Author      : Dang Minh Phuong
 *  Description : Context switch for armv7-m
 */
    .syntax unified
    .arch armv7-m
    .text
    .thumb

/**
  * @ingroup armv7a stack frame
  * @{
  *
  * @file
  * @brief   Implementation of the kernel's architecture dependent thread
  *          interface
  *
  * Use the following register layout when saving context onto the stack:
  *
  * -------- Highest address (bottom of stack)
  * | ..   |
  * --------
  * | ..   |
  * --------
  * | PSR  |
  * --------
  * | PC   |
  * --------
  * | LR   |
  * --------
  * | R12  |
  * --------
  * | R3   |
  * --------
  * | R2   |
  * --------
  * | R1   |
  * --------
  * | R0   |
  * --------      <-- psp (hard)
  * | R11  |
  * --------
  * | R10  |
  * --------        stack growth
  * | R9   |
  * --------             ||
  * | R8   |             ||
  * --------            _||_
  * | R7   |            \  /
  * --------             \/
  * | R6   |
  * --------
  * | R5   |
  * --------
  * | R4   |
  * --------      <-- psp (soft) <-- bos_pStack
  * | ..   |
  * --------
  * | ..   |
  * --------
  * | ..   |
  * -------- Lowest address (top of stack, stack_start)
  *
  * @author  Dang Minh Phuong <kamejoko80@yahoo.com>
  *
  * @}
  */

/**
 * @brief         BOS_StartFirstStack
 * @context       normal
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
    .thumb_func
    .align 2
    .globl BOS_StartFirstStack
    .type  BOS_StartFirstStack, %function

BOS_StartFirstStack:

    /* Disable interrupt */
    cpsid i

    /* Get psp (soft) from current TCB */
    bl    BOS_LoadContext
    ldr   r1, =bos_pStack
    ldr   r0, [r1]

    /* Load r4-r11 */
    ldmia r0!, {r4-r11}

    /* Load psp (hard) */
    msr   psp, r0

    /* Switch to thread mode */
    movs  r0,  #2
    msr   control, r0

    /* Load r0-r3, r12, lr */
    pop {r0-r3, r12, lr}

    /* Enable interrupt */
    cpsie i

    /* Execute task function */
    pop {pc}

/**
 * @brief         PendSV_Handler
 * @context       interrupt
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
    .thumb_func
    .align 2
    .globl PendSV_Handler
    .type  PendSV_Handler, %function

PendSV_Handler:

    /* Disable interrupt */
    cpsid i

#ifdef STACK_MONITOR
    bl BOS_StackMonitor
#endif

    /***********************/
    /***** Save context ****/
    /***********************/

    /* Get psp (hard) */
    mrs   r0, psp
    subs  r0, #32

    /* Store r4-r11 */
    stmia r0!, {r4-r11}

    /* Store psp (soft) */
    subs  r0, #32
    ldr   r1, =bos_pStack
    str   r0, [r1]

    /* Save current TCB */
    bl BOS_SaveContext

    /***********************/
    /**** Switch context ***/
    /***********************/

    bl BOS_SwitchContext
    bl BOS_LoadContext

    /* Get psp (soft) */
    ldr   r1, =bos_pStack
    ldr   r0, [r1]

    /* Load r4-r11 */
    ldmia r0!, {r4-r11}

    /* Load psp (hard) */
    msr   psp, r0

#ifdef STACK_MONITOR
    /* Remark soft stack frame by pattern word */
    ldr   r3, =bos_StackPatternWord
    ldr   r2, [r3]
    ldr   r3, [r1]
.loop:
    str   r2, [r3]
    adds  r3, r3, #4
    cmp   r0, r3
    bne   .loop
#endif

    /* Enable interrupt */
    cpsie i

    /* Exec return to thread mode */
    ldr   r0, =0xFFFFFFFD
    mov   lr, r0
    bx    lr
    nop
    nop
