#include <kernel/isr/isr.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



void syscall_scheduler_get_stats(isr_state_t* regs){
	if (regs->rdx!=sizeof(scheduler_load_balancer_stats_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)||!scheduler_load_balancer_get_stats(regs->rdi,(void*)(regs->rsi))){
		regs->rax=0;
		return;
	}
	regs->rax=1;
}



void syscall_scheduler_get_timers(isr_state_t* regs){
	if (regs->rdx!=sizeof(scheduler_timers_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	*((scheduler_timers_t*)(regs->rsi))=*scheduler_get_timers(regs->rdi);
	regs->rax=1;
}
