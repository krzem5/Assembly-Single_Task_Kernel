#include <kernel/fd/fd.h>
#include <kernel/memory/vmm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_fd_open(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=FD2_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd2_open(regs->rdi,(void*)(regs->rsi),regs->rdx,regs->r8);
}



void syscall_fd_close(syscall_registers_t* regs){
	regs->rax=fd2_close(regs->rdi);
}



void syscall_fd_read(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=FD2_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd2_read(regs->rdi,(void*)(regs->rsi),regs->rdx);
}



void syscall_fd_write(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=FD2_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd2_write(regs->rdi,(void*)(regs->rsi),regs->rdx);
}



void syscall_fd_seek(syscall_registers_t* regs){
	regs->rax=fd2_seek(regs->rdi,regs->rsi,regs->rdx);
}



void syscall_fd_resize(syscall_registers_t* regs){
	regs->rax=fd2_resize(regs->rdi,regs->rsi,regs->rdx);
}



void syscall_fd_stat(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(fd2_stat_t)){
		regs->rax=FD2_ERROR_INVALID_POINTER;
		return;
	}
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=FD2_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd2_stat(regs->rdi,(void*)(regs->rsi));
}



void syscall_fd_dup(syscall_registers_t* regs){
	regs->rax=fd2_dup(regs->rdi,regs->rsi);
}



void syscall_fd_path(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=FD2_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd2_path(regs->rdi,(void*)(regs->rsi),regs->rdx);
}



void syscall_fd_iter_start(syscall_registers_t* regs){
	regs->rax=fd2_iter_start(regs->rdi);
}



void syscall_fd_iter_get(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=FD2_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd2_iter_get(regs->rdi,(void*)(regs->rsi),regs->rdx);
}



void syscall_fd_iter_next(syscall_registers_t* regs){
	regs->rax=fd2_iter_next(regs->rdi);
}



void syscall_fd_iter_stop(syscall_registers_t* regs){
	regs->rax=fd2_iter_stop(regs->rdi);
}

