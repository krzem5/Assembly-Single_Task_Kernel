#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



static u8 _test_crash_state=0x00;



static void _syscall_store_state(u8 state){
	_test_crash_state=state;
}



static syscall_callback_t const _test_crash_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_store_state
};



static void _execute_elf(const char* path){
	error_t ret=elf_load(path,0,NULL,0,NULL,0);
	TEST_ASSERT(!IS_ERROR(ret));
	if (!IS_ERROR(ret)){
		handle_t* handle=handle_lookup_and_acquire(ret,process_handle_type);
		process_t* process=handle->object;
		event_t* delete_event=process->event;
		handle_release(handle);
		event_await(delete_event,0);
	}
}



static void _thread_no_crash(void){
	_test_crash_state=0x01;
}



static void _thread_crash_invalid_read(void){
	(void)(*((volatile const u8*)NULL));
	_test_crash_state=0x01;
}



static void _thread_crash_invalid_write(void){
	*((volatile u8*)NULL)=0x00;
	_test_crash_state=0x01;
}



void test_crash(void){
	TEST_MODULE("crash");
	syscall_create_table("test_crash",_test_crash_syscall_functions,sizeof(_test_crash_syscall_functions)/sizeof(syscall_callback_t));
	TEST_FUNC("_isr_handler");
	TEST_GROUP("no user crash");
	_test_crash_state=0x00;
	_execute_elf("/bin/test_crash_normal");
	TEST_ASSERT(_test_crash_state==0x01);
	TEST_GROUP("invalid user read");
	_test_crash_state=0x00;
	_execute_elf("/bin/test_crash_invalid_read");
	TEST_ASSERT(_test_crash_state==0x00);
	TEST_GROUP("invalid user write");
	_test_crash_state=0x00;
	_execute_elf("/bin/test_crash_invalid_write");
	TEST_ASSERT(_test_crash_state==0x00);
	TEST_GROUP("invalid user instruction");
	_test_crash_state=0x00;
	_execute_elf("/bin/test_crash_wrong_iopl");
	TEST_ASSERT(_test_crash_state==0x00);
	TEST_GROUP("no kernel crash");
	_test_crash_state=0x00;
	process_t* process=process_create("test-process","test-process");
	scheduler_enqueue_thread(thread_create_kernel_thread(process,"test-crash-kernel-no-crash",_thread_no_crash,0x200000,0));
	event_await(process->event,0);
	TEST_ASSERT(_test_crash_state==0x01);
	TEST_GROUP("invalid kernel read");
	_test_crash_state=0x00;
	process=process_create("test-process","test-process");
	scheduler_enqueue_thread(thread_create_kernel_thread(process,"test-crash-kernel-invalid-read",_thread_crash_invalid_read,0x200000,0));
	event_await(process->event,0);
	TEST_ASSERT(_test_crash_state==0x00);
	TEST_GROUP("invalid kernel write");
	_test_crash_state=0x00;
	process=process_create("test-process","test-process");
	scheduler_enqueue_thread(thread_create_kernel_thread(process,"test-crash-kernel-invalid-write",_thread_crash_invalid_write,0x200000,0));
	event_await(process->event,0);
	TEST_ASSERT(_test_crash_state==0x00);
}
