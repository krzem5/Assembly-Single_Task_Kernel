#include <kernel/fd/fd.h>
#include <kernel/memory/vmm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_fd_open(syscall_reg_state_t* regs){
	u64 length=syscall_get_string_length(regs->rsi);
	if (!length){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_open(regs->rdi,(void*)(regs->rsi),length,regs->rdx);
}



void syscall_fd_close(syscall_reg_state_t* regs){
	regs->rax=fd_close(regs->rdi);
}



void syscall_fd_read(syscall_reg_state_t* regs){
	if (regs->rdx>syscall_get_user_pointer_max_length(regs->rsi)){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_read(regs->rdi,(void*)(regs->rsi),regs->rdx,regs->r8);
}



void syscall_fd_write(syscall_reg_state_t* regs){
	if (regs->rdx>syscall_get_user_pointer_max_length(regs->rsi)){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_write(regs->rdi,(void*)(regs->rsi),regs->rdx,regs->r8);
}



void syscall_fd_seek(syscall_reg_state_t* regs){
	regs->rax=fd_seek(regs->rdi,regs->rsi,regs->rdx);
}



void syscall_fd_resize(syscall_reg_state_t* regs){
	regs->rax=fd_resize(regs->rdi,regs->rsi,regs->rdx);
}



void syscall_fd_stat(syscall_reg_state_t* regs){
	if (regs->rdx!=sizeof(fd_stat_t)||regs->rdx>syscall_get_user_pointer_max_length(regs->rsi)){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_stat(regs->rdi,(void*)(regs->rsi));
}



void syscall_fd_dup(syscall_reg_state_t* regs){
	regs->rax=fd_dup(regs->rdi,regs->rsi);
}



void syscall_fd_path(syscall_reg_state_t* regs){
	if (regs->rdx>syscall_get_user_pointer_max_length(regs->rsi)){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_path(regs->rdi,(void*)(regs->rsi),regs->rdx);
}



void syscall_fd_iter_start(syscall_reg_state_t* regs){
	regs->rax=fd_iter_start(regs->rdi);
}



void syscall_fd_iter_get(syscall_reg_state_t* regs){
	if (regs->rdx>syscall_get_user_pointer_max_length(regs->rsi)){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_iter_get(regs->rdi,(void*)(regs->rsi),regs->rdx);
}



void syscall_fd_iter_next(syscall_reg_state_t* regs){
	regs->rax=fd_iter_next(regs->rdi);
}



void syscall_fd_iter_stop(syscall_reg_state_t* regs){
	regs->rax=fd_iter_stop(regs->rdi);
}
