#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/thread/thread.h>
#include <kernel/types.h>



void syscall_thread_stop(syscall_registers_t* regs){
	scheduler_dequeue();
}



void syscall_thread_create(syscall_registers_t* regs){
	regs->rax=0;
}
