#include <kernel/fd/fd.h>
#include <kernel/memory/vmm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_fd_open(syscall_registers_t* regs){
	if (regs->rdi&&FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_open(regs->rdi,VMM_TRANSLATE_ADDRESS(address),regs->rdx,regs->r8);
}



void syscall_fd_close(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_close(regs->rdi);
}



void syscall_fd_delete(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_delete(regs->rdi);
}



void syscall_fd_read(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_read(regs->rdi,VMM_TRANSLATE_ADDRESS(address),regs->rdx);
}



void syscall_fd_write(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_write(regs->rdi,VMM_TRANSLATE_ADDRESS(address),regs->rdx);
}



void syscall_fd_seek(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_seek(regs->rdi,regs->rsi,regs->rdx);
}



void syscall_fd_stat(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	if (regs->rdx!=sizeof(fd_stat_t)){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=FD_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd_stat(regs->rdi,VMM_TRANSLATE_ADDRESS(address));
}



void syscall_fd_get_relative(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_get_relative(regs->rdi,regs->rsi,regs->rdx);
}



void syscall_fd_move(syscall_registers_t* regs){
	if (FD_OUT_OF_RANGE(regs->rdi)||FD_OUT_OF_RANGE(regs->rsi)){
		regs->rax=FD_ERROR_INVALID_FD;
		return;
	}
	regs->rax=fd_move(regs->rdi,regs->rsi);
}
