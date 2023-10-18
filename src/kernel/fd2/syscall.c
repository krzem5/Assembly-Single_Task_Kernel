#include <kernel/fd2/fd2.h>
#include <kernel/memory/vmm.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



void syscall_fd2_open(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=FD2_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd2_open(regs->rdi,(void*)(regs->rsi),regs->rdx,regs->r8);
}



void syscall_fd2_close(syscall_registers_t* regs){
	regs->rax=fd2_close(regs->rdi);
}



void syscall_fd2_delete(syscall_registers_t* regs){
	regs->rax=fd2_delete(regs->rdi);
}



void syscall_fd2_read(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=FD2_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd2_read(regs->rdi,(void*)(regs->rsi),regs->rdx);
}



void syscall_fd2_write(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=FD2_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd2_write(regs->rdi,(void*)(regs->rsi),regs->rdx);
}



void syscall_fd2_seek(syscall_registers_t* regs){
	regs->rax=fd2_seek(regs->rdi,regs->rsi,regs->rdx);
}



void syscall_fd2_resize(syscall_registers_t* regs){
	regs->rax=fd2_resize(regs->rdi,regs->rsi);
}



void syscall_fd2_absolute_path(syscall_registers_t* regs){
	if (!syscall_sanatize_user_memory(regs->rsi,regs->rdx)){
		regs->rax=FD2_ERROR_INVALID_POINTER;
		return;
	}
	regs->rax=fd2_absolute_path(regs->rdi,(void*)(regs->rsi),regs->rdx);
}



void syscall_fd2_stat(syscall_registers_t* regs){
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



void syscall_fd2_get_relative(syscall_registers_t* regs){
	regs->rax=fd2_get_relative(regs->rdi,regs->rsi,regs->rdx);
}



void syscall_fd2_move(syscall_registers_t* regs){
	regs->rax=fd2_move(regs->rdi,regs->rsi);
}
