#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/log/log.h>
#include <kernel/memory/mmap.h>
#include <kernel/memory/pmm.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



extern error_t syscall_uid_get();
extern error_t syscall_uid_set();
extern error_t syscall_uid_get_name();



static void _thread(void){
	mmap_region_t* temp_mmap_region=mmap_alloc(&(THREAD_DATA->process->mmap),0,2*PAGE_SIZE,NULL,MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE|MMAP_REGION_FLAG_VMM_USER,NULL,0);
	char* buffer=(void*)(temp_mmap_region->rb_node.key);
	(void)buffer;
	TEST_FUNC("syscall_uid_get");
	// syscall_uid_get
	TEST_FUNC("syscall_uid_set");
	// syscall_uid_set
	TEST_FUNC("syscall_uid_get_name");
	// syscall_uid_get_name
	mmap_dealloc_region(&(THREAD_DATA->process->mmap),temp_mmap_region);
}



void test_uid(void){
	TEST_MODULE("id");
	TEST_FUNC("uid_create");
	TEST_GROUP("already present");
	TEST_ASSERT(uid_create(0,"root")==ERROR_ALREADY_PRESENT);
	TEST_GROUP("correct args");
	TEST_ASSERT(uid_create(1000,"user")==ERROR_OK);
	TEST_ASSERT(uid_delete(1000)==ERROR_OK);
	TEST_FUNC("uid_delete");
	TEST_GROUP("invalid uid");
	TEST_ASSERT(uid_delete(0xaabbccdd)==ERROR_NOT_FOUND);
	TEST_GROUP("correct args");
	TEST_ASSERT(uid_create(1000,"user")==ERROR_OK);
	TEST_ASSERT(uid_delete(1000)==ERROR_OK);
	TEST_ASSERT(uid_delete(1000)==ERROR_NOT_FOUND);
	TEST_FUNC("uid_add_group");
	// uid_add_group
	TEST_FUNC("uid_has_group");
	// uid_has_group
	TEST_FUNC("uid_get_name");
	// uid_get_name
	TEST_FUNC("uid_get_flags");
	// uid_get_flags
	process_t* test_process=process_create("test-process","test-process");
	scheduler_enqueue_thread(thread_create_kernel_thread(test_process,"test-uid-thread",_thread,0x200000,0));
	event_await(test_process->event,0);
}
