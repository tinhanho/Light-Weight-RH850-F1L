#include "context.h"
#include "scheduler.h"
#include "os.h"

#pragma inline_asm SaveContext
void SaveContext (void) {
	mov     #_CurrentTask, r1       ;get OSKernel
	ld.w	12[r1],r2               ;get needSaveContext flag of current task
	cmp     0x0, r2	                ;check value of needSaveContext flag
	bnz     _SaveContext+0x12       ;if needSaveContext not equals to zero, skip "jarl _SchedulerKernelISR" instruction
	jarl	_ScheduleKernelISR+0x32, lp ;if needSaveContext equals to 0, jump to ScheduleKernelISR
	ld.w	0[r1], r2               ;get CurrentConst
	ld.w	4[r2], r1               ;get context of current task
	stsr	0, r2                   ;save eipc
	st.w	r2, 8[r1]	
	stsr    1, r2                   ;save eipsw
	st.w	r2, 12[r1]
	st.w	r4, 24[r1]              ;save general purpose registers
	st.w	r5, 28[r1]
	st.w	r6, 32[r1]
	st.w	r7, 36[r1]
	st.w	r8, 40[r1]
	st.w	r9, 44[r1]
	st.w	r10, 48[r1]
	st.w	r11, 52[r1]
	st.w	r12, 56[r1]
	st.w	r13, 60[r1]
	st.w	r14, 64[r1]
	st.w	r15, 68[r1]
	st.w	r16, 72[r1]
	st.w	r17, 76[r1]
	st.w	r18, 80[r1]
	st.w	r19, 84[r1]
	st.w	r20, 88[r1]
	st.w	r21, 92[r1]
	st.w	r22, 96[r1]
	st.w	r23, 100[r1]
	st.w	r24, 104[r1]
	st.w	r25, 108[r1]
	st.w	r26, 112[r1]
	st.w	r27, 116[r1]
	st.w	r28, 120[r1]
	st.w	r29, 124[r1]
	st.w	r30, 128[r1]
}

#pragma inline_asm LoadContext
void LoadContext (void) {
	mov 	#_CurrentTask, r1       ;get OSKernel
	ld.w	0[r1], r2               ;get CurrentConst
	ld.w	4[r2], r1               ;get context of current task
	ld.w    0[r1], sp               ;load sp(r3)
	ld.w	4[r1], lp               ;load lp(r31)
	ld.w	8[r1], r2               ;load eipc
	ldsr	r2, 0				
	ld.w	12[r1], r2              ;load eipsw
	ldsr	r2, 1				
	ld.w	20[r1], r2
	ld.w	24[r1], r4              ;load general purpose registers
	ld.w	28[r1], r5			
	ld.w	32[r1], r6
	ld.w	36[r1], r7			
	ld.w	40[r1], r8
	ld.w	44[r1], r9			
	ld.w	48[r1], r10
	ld.w	52[r1], r11			
	ld.w	56[r1], r12
	ld.w	60[r1], r13			
	ld.w	64[r1], r14
	ld.w	68[r1], r15			
	ld.w	72[r1], r16
	ld.w	76[r1], r17			
	ld.w	80[r1], r18
	ld.w	84[r1], r19			
	ld.w	88[r1], r20
	ld.w	92[r1], r21
	ld.w	96[r1], r22			
	ld.w	100[r1], r23
	ld.w	104[r1], r24			
	ld.w	108[r1], r25
	ld.w	112[r1], r26			
	ld.w	116[r1], r27
	ld.w	120[r1], r28			
	ld.w	124[r1], r29
	ld.w	128[r1], r30
	ld.w	16[r1], r1
	eiret
}

#pragma inline_asm ScheduleKernelISR
void ScheduleKernelISR(void) {
	add		-0x0C, sp
	st.w    r1, 8[sp]               ;push r1, r2, lp to stack, they would be used in SaveContext
	st.w	r2, 4[sp]
	st.w	lp, 0[sp]
	jarl	_SaveContext, lp        ;jump to SaveContext
	ld.w	4[sp], r2               ;load r2 back
	st.w	r2, 20[r1]              ;save r2
	mov 	r1, r4                  ;move pointer to currentConst from r1 to r4
	ld.w	8[sp], r2               ;load r1 back
	st.w	r2, 16[r4]              ;save r1
	ld.w	0[sp], r1               ;load lp back
	st.w	r1, 4[r4]               ;save lp
	add		0x0C, sp                ;add sp(r3) back to right value
	st.w	sp, 0[r4]               ;save sp(r3), all context have been saved
	jarl	_Scheduler, lp          ;jump to Scheduler
	jarl	_LoadContext, lp        ;jump to LoadContext
}

void StartSchedule(void) {
	InitTask(1);
	CurrentTask.CurrentConst = &TaskConst[1];
	CurrentTask.CurrentVar = &TaskVar[1];
	CurrentTask.CurrentVar->activateCount++;
	CurrentTask.currentID = 1;
	CurrentTask.CurrentVar->state = RUNNING;
	CurrentTask.needSaveContext = NEEDSAVE;
	GetInternalResource();
	LoadContext();
}