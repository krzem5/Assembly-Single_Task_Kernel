#include <kernel/fd/fd.h>
#include <kernel/memory/vmm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_fd_open(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=_open(regs->rdi,(void*)(regs->rsi),regs->rdx,regs->r8);
}



void syscall_fd_close(syscall_registers_t* regs){
	regs->rax=_close(regs->rdi);
}



void syscall_fd_read(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=_read(regs->rdi,(void*)(regs->rsi),regs->rdx);
}



void syscall_fd_write(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=_write(regs->rdi,(void*)(regs->rsi),regs->rdx);
}



void syscall_fd_seek(syscall_registers_t* regs){
	regs->rax=_seek(regs->rdi,regs->rsi,regs->rdx);
}



void syscall_fd_resize(syscall_registers_t* regs){
	regs->rax=_resize(regs->rdi,regs->rsi,regs->rdx);
}



void syscall_fd_stat(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(_stat_t)){
		regs->rax=_ERROR_INVALID_POINTER;
		return;
	}
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=_stat(regs->rdi,(void*)(regs->rsi));
}



void syscall_fd_dup(syscall_registers_t* regs){
	regs->rax=_dup(regs->rdi,regs->rsi);
}



void syscall_fd_path(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=_path(regs->rdi,(void*)(regs->rsi),regs->rdx);
}



void syscall_fd_iter_start(syscall_registers_t* regs){
	regs->rax=_iter_start(regs->rdi);
}



void syscall_fd_iter_get(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=_iter_get(regs->rdi,(void*)(regs->rsi),regs->rdx);
}



void syscall_fd_iter_next(syscall_registers_t* regs){
	regs->rax=_iter_next(regs->rdi);
}



void syscall_fd_iter_stop(syscall_registers_t* regs){
	regs->rax=_iter_stop(regs->rdi);
}

