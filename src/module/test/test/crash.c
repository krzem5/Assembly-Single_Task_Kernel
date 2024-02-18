#include <kernel/elf/elf.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
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



void test_crash(void){
	TEST_MODULE("crash");
	syscall_create_table("test_crash",_test_crash_syscall_functions,sizeof(_test_crash_syscall_functions)/sizeof(syscall_callback_t));
	TEST_FUNC("_isr_handler");
	TEST_GROUP("no crash");
	_test_crash_state=0x00;
	_execute_elf("/bin/test_crash_normal");
	TEST_ASSERT(_test_crash_state==0x01);
	_test_crash_state=0x00;
	_execute_elf("/bin/test_crash_invalid_read");
	TEST_ASSERT(_test_crash_state==0x00);
	_test_crash_state=0x00;
	_execute_elf("/bin/test_crash_invalid_write");
	TEST_ASSERT(_test_crash_state==0x00);
	_test_crash_state=0x00;
	_execute_elf("/bin/test_crash_wrong_iopl");
	TEST_ASSERT(_test_crash_state==0x00);
}
