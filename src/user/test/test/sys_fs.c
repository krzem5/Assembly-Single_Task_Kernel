#include <sys/error/error.h>
#include <sys/fs/fs.h>
#include <sys/memory/memory.h>
#include <sys/string/string.h>
#include <sys/syscall/syscall.h>
#include <sys/types.h>
#include <test/test.h>



void test_sys_fs(void){
	u64 syscall_table_offset=sys_syscall_get_table_offset("test_sys_fs");
	TEST_MODULE("sys_fs");
	TEST_FUNC("sys_fs_iter_start");
	TEST_GROUP("correct args");
	sys_error_t fs_handle=sys_fs_iter_start();
	TEST_ASSERT(!SYS_IS_ERROR(fs_handle)&&fs_handle);
	TEST_FUNC("sys_fs_iter_next");
	TEST_GROUP("next handle");
	fs_handle=sys_fs_iter_next(fs_handle);
	TEST_ASSERT(!SYS_IS_ERROR(fs_handle)&&fs_handle);
	TEST_GROUP("last handle");
	TEST_ASSERT(!sys_fs_iter_next(0xfffffffffffffffe));
	TEST_FUNC("sys_fs_get_data");
	TEST_GROUP("invalid buffer");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
	TEST_ASSERT(sys_fs_get_data(0,(void*)1)==SYS_ERROR_INVALID_ARGUMENT(1));
#pragma GCC diagnostic pop
	TEST_GROUP("invalid handle");
	sys_fs_data_t fs_data;
	TEST_ASSERT(sys_fs_get_data(0xaabbccdd,&fs_data)==SYS_ERROR_INVALID_HANDLE);
	TEST_GROUP("correct args");
	fs_handle=_sys_syscall0(syscall_table_offset|0x00000001);
	TEST_ASSERT(sys_fs_get_data(fs_handle,&fs_data)==SYS_ERROR_OK);
	TEST_ASSERT(!sys_string_compare(fs_data.type,"test-fs-descriptor-config"));
	TEST_ASSERT(!fs_data.partition);
	for (u32 i=0;i<16;i++){
		TEST_ASSERT(fs_data.guid[i]==i*17);
	}
	TEST_ASSERT(!sys_string_compare(fs_data.mount_path,""));
	TEST_FUNC("sys_fs_mount");
	TEST_GROUP("empty path");
	TEST_ASSERT(sys_fs_mount(0,"")==SYS_ERROR_INVALID_ARGUMENT(1));
	TEST_GROUP("path too long");
	char path[8192];
	sys_memory_set(path,8192,'A');
	TEST_ASSERT(sys_fs_mount(0,path)==SYS_ERROR_INVALID_ARGUMENT(1));
	TEST_GROUP("invalid handle");
	TEST_ASSERT(sys_fs_mount(0xaabbccdd,"/")==SYS_ERROR_INVALID_HANDLE);
	TEST_GROUP("path already present");
	TEST_ASSERT(sys_fs_mount(fs_handle,"/share")==SYS_ERROR_ALREADY_PRESENT);
}
