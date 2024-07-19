#include <kernel/cpu/cpu.h>
#include <kernel/log/log.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



extern error_t syscall_cpu_get_count();



static u64 _syscall_get_cpu_count(void){
	return cpu_count;
}



static syscall_callback_t const _test_sys_cpu_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_get_cpu_count,
};



static void _thread(void){
	TEST_FUNC("syscall_cpu_get_count");
	TEST_GROUP("correct args");
	TEST_ASSERT(syscall_cpu_get_count()==cpu_count);
	handle_release(&(THREAD_DATA->header.current_thread->handle));
}



KERNEL_AWAITS void test_cpu(void){
	TEST_MODULE("cpu");
	process_t* test_process=process_create("test-process","test-process",0x1000,0x3000);
	handle_acquire(&(test_process->handle));
	scheduler_enqueue_thread(thread_create_kernel_thread(test_process,"test.cpu",_thread,0));
	event_await(&(test_process->event),1,0);
	handle_release(&(test_process->handle));
	syscall_create_table("test_sys_cpu",_test_sys_cpu_syscall_functions,sizeof(_test_sys_cpu_syscall_functions)/sizeof(syscall_callback_t));
}
