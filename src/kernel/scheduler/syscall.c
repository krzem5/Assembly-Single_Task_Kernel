#include <kernel/scheduler/load_balancer.h>
#include <kernel/mp/thread.h>
#include <kernel/sandbox/sandbox.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>



SANDBOX_DECLARE_TYPE(DISABLE_SCHEDULER_API);



void syscall_scheduler_get_stats(syscall_registers_t* regs){
	if (sandbox_get(THREAD_DATA->sandbox,SANDBOX_FLAG_DISABLE_SCHEDULER_API)||regs->rdx!=sizeof(scheduler_load_balancer_stats_t)||!syscall_sanatize_user_memory(regs->rsi,regs->rdx)||!scheduler_load_balancer_get_stats(regs->rdi,(void*)(regs->rsi))){
		regs->rax=0;
		return;
	}
	regs->rax=1;
}
