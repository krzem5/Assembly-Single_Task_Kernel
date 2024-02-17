#include <kernel/cpu/cpu.h>
#include <kernel/log/log.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



extern error_t syscall_cpu_get_count();



static void _thread(void){
	TEST_FUNC("syscall_cpu_get_count");
	TEST_GROUP("correct args");
	TEST_ASSERT(syscall_cpu_get_count()==cpu_count);
}



void test_cpu(void){
	TEST_MODULE("cpu");
	process_t* test_process=process_create("test-process","test-process");
	scheduler_enqueue_thread(thread_create_kernel_thread(test_process,"test-cpu-thread",_thread,0x200000,0));
	event_await(test_process->event,0);
}
