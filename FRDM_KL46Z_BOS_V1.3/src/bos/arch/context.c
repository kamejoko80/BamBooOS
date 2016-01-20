/*
 *  Copyright   : 2015 
 *  File name   : context.c
 *  Author      : Dang Minh Phuong
 *  Description : Context switch for ARM cortex-M0+
 */

#include "bos.h"

#define __enter_critical  CPSID I
#define __exit_critical   CPSIE I 

/**
 * @brief         BOS_StartFirstStack
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */ 
__asm void BOS_StartFirstStack (void)
{
    extern bos_pStack;
    extern BOS_LoadContext;

    PRESERVE8
    
    __enter_critical

    BL BOS_LoadContext    // update current stack pointer 
    
    LDR   R1, =bos_pStack // R2 store address of bos_pStack
    LDR   R0, [R1]  // Load next psp

    ADDS  R0, #16   // Load R8-R11 first
    LDMIA R0!, {R4-R7}
    MOV   R8, R4
    MOV   R9, R5 
    MOV   R10, R6
    MOV   R11, R7

    LDR   R0, [R1]  // Load R4-R7 
    LDMIA R0!, {R4-R7}

    ADDS  R0, #16 // Update current PSP
    MSR   PSP, R0 // Load hw stack frame pointer

    MOVS  R0, #2 // Switch to thread mode
    MSR CONTROL, R0

    POP {R0-R5} // Restore hardware register 
    MOV R12, R4 // Restore R12
    MOV LR,  R5 // Restore LR

    __exit_critical

    POP {PC}    // Start firstack

    ALIGN
}

/**
 * @brief         PendSV_Handler
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
__asm PendSV_Handler (void)
{
    extern bos_pStack;
    extern BOS_LoadContext;
    extern BOS_SwitchContext;
    extern BOS_SaveContext;

#ifdef STACK_MONITOR
    extern BOS_StackMonitor;    
#endif
    
    PRESERVE8

    __enter_critical
    
#ifdef STACK_MONITOR
    BL    BOS_StackMonitor	
#endif
    
    /* Save context */ 
    MRS   R0, PSP         // Load current psp
    SUBS  R0, #32
    STMIA R0!, {R4-R7}    // Store R4-R7
    MOV   R4, R8 
    MOV   R5, R9 
    MOV   R6, R10 
    MOV   R7, R11
    STMIA R0!, {R4-R7}    // Store R8-R11 
    SUBS  R0, #32         // R0 handle psp to save

    LDR   R1, =bos_pStack // R1 store address of bos_pStack
    STR   R0, [R1]        // bos_pStack = current stack

    BL BOS_SaveContext    // Save context

    /* Switch context */
    BL BOS_SwitchContext
    BL BOS_LoadContext 

    LDR   R1, =bos_pStack // R1 store address of bos_pStack
    LDR   R0, [R1]        // Load next psp
    
    ADDS  R0, #16         // Load R8-R11 first
    LDMIA R0!, {R4-R7}
    MOV   R8,  R4
    MOV   R9,  R5 
    MOV   R10, R6
    MOV   R11, R7
    
    LDR   R0, [R1]        // Load R4-R7 
    LDMIA R0!, {R4-R7}
    
    ADDS  R0, #16         // Update current PSP
    MSR   PSP, R0         // Load hw stack frame pointer

    // Mask free stack memory by STACK_PATTERN
#ifdef STACK_MONITOR    
    LDR   R2, =STACK_PATTERN_WORD
    LDR   R3, [R1]        // R3 as index  
LOOP
    STR   R2, [R3]        // Mask STACK_PATTERN
    ADDS  R3, R3, #4 
    CMP   R0, R3
    BNE   LOOP   
#endif

    __exit_critical	
    
    LDR   R0, =0xFFFFFFFD // EXEC_RETURN (to thread mode)
    MOV   LR, R0     
    BX    LR
    NOP
    NOP
}
