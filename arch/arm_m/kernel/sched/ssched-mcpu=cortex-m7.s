  .syntax unified
  .cpu cortex-m7
  .fpu softvfp
  .thumb

.global PendSV_Handler
.thumb_func
PendSV_Handler:
    push {LR}
    mrs   r0 ,  psp
    STMDB r0!,  {R4-R11} 
    bl Scheduler_Task
    LDMIA R0!,  {R4-R11}
    msr         psp, r0     
    POP {LR}
    bx  lr  


.global change_to_task_mode 
change_to_task_mode:        
    mrs r0, MSP
    msr psp, r0
    mrs r0 , CONTROL               
    orr r0 , r0, #2
    msr CONTROL, r0
    mrs r0 , CONTROL
    orr r0 , r0, #1                 
    msr CONTROL, r0
    bx  lr  



.global __sched 
__sched:
    dsb 
    ldr r0, =0xE000ED04  
    ldr r1, =0x10000000  
    isb
    str r1, [r0]
    bx lr    



