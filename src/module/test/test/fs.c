#include <kernel/error/error.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
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



extern error_t syscall_fs_get_next();
extern error_t syscall_fs_get_data();
extern error_t syscall_fs_mount();



static void _thread(void){
	mmap_region_t* temp_mmap_region=mmap_alloc(&(THREAD_DATA->process->mmap),0,2*PAGE_SIZE,NULL,MMAP_REGION_FLAG_VMM_NOEXECUTE|MMAP_REGION_FLAG_VMM_READWRITE|MMAP_REGION_FLAG_VMM_USER,NULL,0);
	char* buffer=(void*)(temp_mmap_region->rb_node.key);
	TEST_FUNC("syscall_fs_get_next");
	TEST_GROUP("first handle");
	error_t fs_handle=syscall_fs_get_next(0);
	TEST_ASSERT(!IS_ERROR(fs_handle)&&fs_handle);
	TEST_GROUP("next handle");
	fs_handle=syscall_fs_get_next(fs_handle);
	TEST_ASSERT(!IS_ERROR(fs_handle)&&fs_handle);
	TEST_GROUP("last handle");
	TEST_ASSERT(!syscall_fs_get_next(0xfffffffffffffffe));
	TEST_FUNC("syscall_fs_get_data");
	TEST_GROUP("invalid buffer length");
	TEST_ASSERT(syscall_fs_get_data(0,NULL,sizeof(filesystem_user_data_t)-1)==ERROR_INVALID_ARGUMENT(2));
	TEST_GROUP("invalid buffer");
	TEST_ASSERT(syscall_fs_get_data(0,NULL,sizeof(filesystem_user_data_t))==ERROR_INVALID_ARGUMENT(1));
	TEST_GROUP("invalid handle");
	TEST_ASSERT(syscall_fs_get_data(0xaabbccdd,buffer,2*PAGE_SIZE)==ERROR_INVALID_HANDLE);
	TEST_GROUP("correct args");
	// ...
	TEST_FUNC("syscall_fs_mount");
	TEST_GROUP("empty path");
	buffer[0]=0;
	TEST_ASSERT(syscall_fs_mount(0,buffer)==ERROR_INVALID_ARGUMENT(1));
	TEST_GROUP("path too long");
	memset(buffer,'A',2*PAGE_SIZE);
	TEST_ASSERT(syscall_fs_mount(0,buffer)==ERROR_INVALID_ARGUMENT(1));
	TEST_GROUP("invalid handle");
	strcpy(buffer,"/",2*PAGE_SIZE);
	TEST_ASSERT(syscall_fs_mount(0xaabbccdd,buffer)==ERROR_INVALID_HANDLE);
	TEST_GROUP("root override");
	strcpy(buffer,"/",2*PAGE_SIZE);
	// ...
	TEST_GROUP("path already present");
	strcpy(buffer,"/share",2*PAGE_SIZE);
	// ...
	TEST_GROUP("correct args");
	strcpy(buffer,"/test-mount-path",2*PAGE_SIZE);
	// ...
	mmap_dealloc_region(&(THREAD_DATA->process->mmap),temp_mmap_region);
}



void test_fs(void){
	TEST_MODULE("fs");
	TEST_FUNC("fs_create");
	TEST_GROUP("correct args");
	// ...
	TEST_GROUP("delete");
	// ...
	process_t* test_process=process_create("test-process","test-process");
	scheduler_enqueue_thread(thread_create_kernel_thread(test_process,"test-fs-thread",_thread,0x200000,0));
	event_await(test_process->event,0);
}
