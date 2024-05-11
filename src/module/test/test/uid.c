#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/id/flags.h>
#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/string.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



extern error_t syscall_uid_get();
extern error_t syscall_uid_set();
extern error_t syscall_uid_get_name();



static void _syscall_set_arbitrary_uid(u64 uid){
	THREAD_DATA->process->uid=uid;
}



static syscall_callback_t const _test_sys_uid_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_set_arbitrary_uid,
};



static void _thread(void){
	mmap_region_t* temp_mmap_region=mmap_alloc(THREAD_DATA->process->mmap,0,PAGE_SIZE,MMAP_REGION_FLAG_VMM_WRITE|MMAP_REGION_FLAG_VMM_USER,NULL);
	char* buffer=(void*)(temp_mmap_region->rb_node.key);
	TEST_FUNC("syscall_uid_get");
	TEST_GROUP("correct args");
	TEST_ASSERT(!syscall_uid_get());
	THREAD_DATA->process->uid=2000;
	TEST_ASSERT(syscall_uid_get()==2000);
	THREAD_DATA->process->uid=0;
	TEST_FUNC("syscall_uid_set");
	TEST_GROUP("not root");
	THREAD_DATA->process->uid=2000;
	THREAD_DATA->process->gid=2000;
	TEST_ASSERT(syscall_uid_set(2001)==ERROR_DENIED);
	THREAD_DATA->process->uid=0;
	THREAD_DATA->process->gid=0;
	TEST_GROUP("correct args");
	TEST_ASSERT(!syscall_uid_get());
	TEST_ASSERT(syscall_uid_set(2000)==ERROR_OK);
	TEST_ASSERT(syscall_uid_get()==2000);
	THREAD_DATA->process->uid=0;
	TEST_FUNC("syscall_uid_get_name");
	TEST_GROUP("invalid buffer length");
	TEST_ASSERT(syscall_uid_get_name(0,buffer,0)==ERROR_INVALID_ARGUMENT(2));
	TEST_GROUP("invalid buffer");
	TEST_ASSERT(syscall_uid_get_name(0,NULL,PAGE_SIZE)==ERROR_INVALID_ARGUMENT(1));
	TEST_GROUP("invalid uid");
	TEST_ASSERT(syscall_uid_get_name(0xaabbccdd,buffer,PAGE_SIZE)==ERROR_NOT_FOUND);
	TEST_GROUP("correct args");
	TEST_ASSERT(syscall_uid_get_name(0,buffer,PAGE_SIZE)==ERROR_OK);
	TEST_ASSERT(str_equal(buffer,"root"));
	mmap_dealloc_region(THREAD_DATA->process->mmap,temp_mmap_region);
}



void test_uid(void){
	TEST_MODULE("uid");
	TEST_FUNC("uid_create");
	TEST_GROUP("already present");
	TEST_ASSERT(uid_create(0,"root")==ERROR_ALREADY_PRESENT);
	TEST_GROUP("correct args");
	TEST_ASSERT(uid_create(2000,"user")==ERROR_OK);
	TEST_ASSERT(uid_delete(2000)==ERROR_OK);
	TEST_FUNC("uid_delete");
	TEST_GROUP("invalid uid");
	TEST_ASSERT(uid_delete(0xaabbccdd)==ERROR_NOT_FOUND);
	TEST_GROUP("correct args");
	TEST_ASSERT(uid_create(2000,"user")==ERROR_OK);
	TEST_ASSERT(uid_add_group(2000,2000)==ERROR_OK);
	TEST_ASSERT(uid_delete(2000)==ERROR_OK);
	TEST_ASSERT(uid_delete(2000)==ERROR_NOT_FOUND);
	TEST_FUNC("uid_add_group");
	TEST_GROUP("invalid uid");
	TEST_ASSERT(uid_add_group(0xaabbccdd,0)==ERROR_NOT_FOUND);
	TEST_GROUP("already present");
	TEST_ASSERT(uid_add_group(0,0)==ERROR_ALREADY_PRESENT);
	TEST_GROUP("correct args");
	TEST_ASSERT(!uid_has_group(0,2000));
	TEST_ASSERT(uid_add_group(0,2000)==ERROR_OK);
	TEST_ASSERT(uid_has_group(0,2000)==1);
	TEST_ASSERT(uid_remove_group(0,2000)==ERROR_OK);
	TEST_FUNC("uid_has_group");
	TEST_GROUP("invalid uid");
	TEST_ASSERT(uid_has_group(0xaabbccdd,0)==ERROR_NOT_FOUND);
	TEST_GROUP("correct args, group not present");
	TEST_ASSERT(!uid_has_group(0,2000));
	TEST_GROUP("correct args, group present");
	TEST_ASSERT(uid_has_group(0,0)==1);
	TEST_FUNC("uid_remove_group");
	TEST_GROUP("invalid uid");
	TEST_ASSERT(uid_remove_group(0xaabbccdd,0)==ERROR_NOT_FOUND);
	TEST_GROUP("group not found");
	TEST_ASSERT(uid_remove_group(0,0xaabbccdd)==ERROR_NOT_FOUND);
	TEST_GROUP("correct args");
	TEST_ASSERT(uid_add_group(0,2000)==ERROR_OK);
	TEST_ASSERT(uid_remove_group(0,2000)==ERROR_OK);
	TEST_ASSERT(!uid_has_group(0,2000));
	TEST_FUNC("uid_get_name");
	TEST_GROUP("invalid buffer length");
	TEST_ASSERT(uid_get_name(0,NULL,0)==ERROR_NO_SPACE);
	char buffer[256];
	TEST_GROUP("invalid uid");
	TEST_ASSERT(uid_get_name(0xaabbccdd,buffer,256)==ERROR_NOT_FOUND);
	TEST_GROUP("correct args");
	TEST_ASSERT(uid_get_name(0,buffer,256)==ERROR_OK);
	TEST_ASSERT(str_equal(buffer,"root"));
	TEST_FUNC("uid_get_flags");
	TEST_GROUP("invalid uid");
	TEST_ASSERT(uid_get_flags(0xaabbccdd)==ERROR_NOT_FOUND);
	TEST_GROUP("correct args");
	TEST_ASSERT(!uid_get_flags(0));
	TEST_ASSERT(uid_set_flags(0,0,ID_FLAG_BYPASS_VFS_PERMISSIONS)==ERROR_OK);
	TEST_ASSERT(uid_get_flags(0)==ID_FLAG_BYPASS_VFS_PERMISSIONS);
	TEST_ASSERT(uid_set_flags(0,ID_FLAG_BYPASS_VFS_PERMISSIONS,0)==ERROR_OK);
	TEST_FUNC("uid_set_flags");
	TEST_GROUP("invalid uid");
	TEST_ASSERT(uid_set_flags(0xaabbccdd,0,0)==ERROR_NOT_FOUND);
	TEST_GROUP("correct args");
	TEST_ASSERT(!uid_get_flags(0));
	TEST_ASSERT(uid_set_flags(0,0,ID_FLAG_BYPASS_VFS_PERMISSIONS)==ERROR_OK);
	TEST_ASSERT(uid_get_flags(0)==ID_FLAG_BYPASS_VFS_PERMISSIONS);
	TEST_ASSERT(uid_set_flags(0,ID_FLAG_BYPASS_VFS_PERMISSIONS,ID_FLAG_BYPASS_VFS_PERMISSIONS|ID_FLAG_ROOT_ACL)==ERROR_OK);
	TEST_ASSERT(uid_get_flags(0)==(ID_FLAG_BYPASS_VFS_PERMISSIONS|ID_FLAG_ROOT_ACL));
	TEST_ASSERT(uid_set_flags(0,ID_FLAG_BYPASS_VFS_PERMISSIONS|ID_FLAG_ROOT_ACL,0)==ERROR_OK);
	TEST_ASSERT(!uid_get_flags(0));
	process_t* test_process=process_create("test-process","test-process",0x1000,0x3000);
	handle_acquire(&(test_process->handle));
	scheduler_enqueue_thread(thread_create_kernel_thread(test_process,"test.uid",_thread,0));
	event_await(test_process->event,0);
	handle_release(&(test_process->handle));
	syscall_create_table("test_sys_uid",_test_sys_uid_syscall_functions,sizeof(_test_sys_uid_syscall_functions)/sizeof(syscall_callback_t));
}
