#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/isr/isr.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>



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



void syscall_uid_get_name(isr_state_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	regs->rax=uid_get_name(regs->rdi,(char*)(regs->rsi),regs->rdx);
}



void syscall_gid_get_name(isr_state_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=0;
		return;
	}
	regs->rax=gid_get_name(regs->rdi,(char*)(regs->rsi),regs->rdx);
}
