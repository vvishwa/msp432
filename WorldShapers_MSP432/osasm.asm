;/*****************************************************************************/
; OSasm.asm: low-level OS commands, written in assembly                       */
; Runs on LM4F120/TM4C123/MSP432
; Lab 3 starter file
; March 2, 2016

;
        .thumb
        .text
        .align 2
        .global  RunPt            ; currently running thread
        .global  StartOS
        .global  SysTick_Handler
        .global  Scheduler
        .global  PendSV_Handler

RunPtAddr .field RunPt,32
SysTick_Handler:  .asmfunc      ; 1) Saves R0-R3,R12,LR,PC,PSR
    CPSID   I                  ; 2) Prevent interrupt during switch
       ; same as Lab 3
    CPSIE   I                  ; 9) tasks run with interrupts enabled
    BX      LR                 ; 10) restore R0-R3,R12,LR,PC,PSR

       .endasmfunc
StartOS: .asmfunc
    LDR     R0, RunPtAddr      ; currently running thread
       ; same as Lab 3
    CPSIE   I                  ; Enable interrupts at processor level
    BX      LR                 ; start first thread
       .endasmfunc
    
LRvalueAddr .field 0xFFFFFFF9,32
PendSV_Handler: .asmfunc
    LDR     R0, RunPtAddr         ; run this thread next
    LDR     R2, [R0]           ; R2 = value of RunPt
    LDR     SP, [R2]           ; new thread SP; SP = RunPt->stackPointer;
    POP     {R4-R11}           ; restore regs r4-11
    LDR     LR,LRvalueAddr
    BX      LR     
       .endasmfunc
      .end
