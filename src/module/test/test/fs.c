#include <kernel/error/error.h>
#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/mmap/mmap.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



extern error_t syscall_fs_get_next();
extern error_t syscall_fs_get_data();
extern error_t syscall_fs_mount();



static filesystem_t* _test_fs_deleted_filesystem=NULL;
static filesystem_t* _test_fs_filesystem=NULL;



static void _deinit_callback(filesystem_t* fs){
	_test_fs_deleted_filesystem=fs;
}



static const filesystem_descriptor_config_t _test_fs_filesystem_descriptor_config={
	"test-fs-descriptor-config",
	_deinit_callback,
	NULL,
	NULL,
	NULL
};



static u64 _syscall_get_test_fs_handle(void){
	return _test_fs_filesystem->handle.rb_node.key;
}



static syscall_callback_t const _test_sys_fs_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_get_test_fs_handle
};



static void _thread(filesystem_descriptor_t* fs_descriptor){
	mmap_region_t* temp_mmap_region=mmap_alloc(THREAD_DATA->process->mmap,0,2*PAGE_SIZE,MMAP_REGION_FLAG_VMM_WRITE|MMAP_REGION_FLAG_VMM_USER,NULL);
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
	_test_fs_filesystem=fs_create(fs_descriptor);
	TEST_ASSERT(_test_fs_filesystem);
	for (u32 i=0;i<16;i++){
		_test_fs_filesystem->guid[i]=i*17;
	}
	TEST_ASSERT(syscall_fs_get_data(_test_fs_filesystem->handle.rb_node.key,buffer,2*PAGE_SIZE)==ERROR_OK);
	const filesystem_user_data_t* fs_user_data=(const void*)buffer;
	TEST_ASSERT(str_equal(fs_user_data->type,_test_fs_filesystem_descriptor_config.name));
	TEST_ASSERT(!fs_user_data->partition);
	for (u32 i=0;i<16;i++){
		TEST_ASSERT(fs_user_data->guid[i]==i*17);
	}
	TEST_ASSERT(str_equal(fs_user_data->mount_path,""));
	TEST_FUNC("syscall_fs_mount");
	TEST_GROUP("empty path");
	buffer[0]=0;
	TEST_ASSERT(syscall_fs_mount(0,buffer)==ERROR_INVALID_ARGUMENT(1));
	TEST_GROUP("path too long");
	mem_fill(buffer,2*PAGE_SIZE,'A');
	TEST_ASSERT(syscall_fs_mount(0,buffer)==ERROR_INVALID_ARGUMENT(1));
	TEST_GROUP("invalid handle");
	str_copy("/",buffer,2*PAGE_SIZE);
	TEST_ASSERT(syscall_fs_mount(0xaabbccdd,buffer)==ERROR_INVALID_HANDLE);
	TEST_GROUP("path already present");
	str_copy("/share",buffer,2*PAGE_SIZE);
	TEST_ASSERT(syscall_fs_mount(_test_fs_filesystem->handle.rb_node.key,buffer)==ERROR_ALREADY_PRESENT);
	TEST_GROUP("correct args");
	str_copy("/test-mount-path",buffer,2*PAGE_SIZE);
	// syscall_fs_mount: correct args => ERROR_OK
	mmap_dealloc_region(THREAD_DATA->process->mmap,temp_mmap_region);
}



void test_fs(void){
	TEST_MODULE("fs");
	filesystem_descriptor_t* fs_descriptor=fs_register_descriptor(&_test_fs_filesystem_descriptor_config);
	TEST_FUNC("fs_create");
	TEST_GROUP("correct args");
	filesystem_t* fs=fs_create(fs_descriptor);
	TEST_ASSERT(fs);
	TEST_ASSERT(fs->descriptor==fs_descriptor);
	TEST_ASSERT(!fs->functions);
	TEST_ASSERT(!fs->partition);
	TEST_ASSERT(!fs->root);
	for (u32 i=0;i<16;i++){
		TEST_ASSERT(!fs->guid[i]);
	}
	TEST_ASSERT(!fs->is_mounted);
	handle_release(&(fs->handle));
	TEST_GROUP("delete");
	fs=fs_create(fs_descriptor);
	TEST_ASSERT(fs);
	handle_release(&(fs->handle));
	TEST_ASSERT(_test_fs_deleted_filesystem==fs);
	process_t* test_process=process_create("test-process","test-process",0x1000,0x3000);
	handle_acquire(&(test_process->handle));
	scheduler_enqueue_thread(thread_create_kernel_thread(test_process,"test.fs",_thread,1,fs_descriptor));
	event_await(test_process->event,0);
	handle_release(&(test_process->handle));
	syscall_create_table("test_sys_fs",_test_sys_fs_syscall_functions,sizeof(_test_sys_fs_syscall_functions)/sizeof(syscall_callback_t));
}
