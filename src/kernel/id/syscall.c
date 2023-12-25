#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/mp/thread.h>
#include <kernel/syscall/syscall.h>



u64 syscall_uid_get(syscall_reg_state_t* regs){
	return THREAD_DATA->process->uid;
}



u64 syscall_gid_get(syscall_reg_state_t* regs){
	return THREAD_DATA->process->gid;
}



u64 syscall_uid_set(syscall_reg_state_t* regs){
	if (!THREAD_DATA->process->uid||!THREAD_DATA->process->gid||uid_has_group(THREAD_DATA->process->uid,0)){
		THREAD_DATA->process->uid=regs->rdi;
		return 1;
	}
	return 0;
}



u64 syscall_gid_set(syscall_reg_state_t* regs){
	if (!THREAD_DATA->process->uid||!THREAD_DATA->process->gid||uid_has_group(THREAD_DATA->process->uid,0)){
		THREAD_DATA->process->gid=regs->rdi;
		return 1;
	}
	return 0;
}



u64 syscall_uid_get_name(syscall_reg_state_t* regs){
	if (regs->rdx>syscall_get_user_pointer_max_length(regs->rsi)){
		return 0;
	}
	return uid_get_name(regs->rdi,(char*)(regs->rsi),regs->rdx);
}



u64 syscall_gid_get_name(syscall_reg_state_t* regs){
	if (regs->rdx>syscall_get_user_pointer_max_length(regs->rsi)){
		return 0;
	}
	return gid_get_name(regs->rdi,(char*)(regs->rsi),regs->rdx);
}
