#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/isr/isr.h>
#include <kernel/mp/thread.h>



void syscall_uid_get(isr_state_t* regs){
	regs->rax=THREAD_DATA->process->uid;
}



void syscall_gid_get(isr_state_t* regs){
	regs->rax=THREAD_DATA->process->gid;
}



void syscall_uid_set(isr_state_t* regs){
	if (!THREAD_DATA->process->uid||!THREAD_DATA->process->gid||uid_has_group(THREAD_DATA->process->uid,0)){
		THREAD_DATA->process->uid=regs->rdi;
		regs->rax=1;
	}
	else{
		regs->rax=0;
	}
}



void syscall_gid_set(isr_state_t* regs){
	if (!THREAD_DATA->process->uid||!THREAD_DATA->process->gid||uid_has_group(THREAD_DATA->process->uid,0)){
		THREAD_DATA->process->gid=regs->rdi;
		regs->rax=1;
	}
	else{
		regs->rax=0;
	}
}
