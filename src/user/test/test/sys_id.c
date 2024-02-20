#include <sys/error/error.h>
#include <sys/id/group.h>
#include <sys/id/user.h>
#include <sys/string/string.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/syscall/syscall.h>
#include <sys/types.h>
#include <test/test.h>



void test_sys_id(void){
	u64 gid_syscall_table_offset=sys_syscall_get_table_offset("test_sys_gid");
	u64 uid_syscall_table_offset=sys_syscall_get_table_offset("test_sys_uid");
	TEST_MODULE("sys_id");
	TEST_FUNC("sys_gid_get");
	TEST_GROUP("correct args");
	TEST_ASSERT(!sys_gid_get());
	_sys_syscall1(gid_syscall_table_offset|0x00000001,1000);
	TEST_ASSERT(sys_gid_get()==1000);
	_sys_syscall1(gid_syscall_table_offset|0x00000001,0);
	TEST_FUNC("sys_gid_set");
	TEST_GROUP("not root");
	_sys_syscall1(uid_syscall_table_offset|0x00000001,1000);
	_sys_syscall1(gid_syscall_table_offset|0x00000001,1000);
	TEST_ASSERT(sys_gid_set(1001)==SYS_ERROR_DENIED);
	_sys_syscall1(uid_syscall_table_offset|0x00000001,0);
	_sys_syscall1(gid_syscall_table_offset|0x00000001,0);
	TEST_GROUP("correct args");
	TEST_ASSERT(!sys_gid_get());
	TEST_ASSERT(sys_gid_set(1000)==SYS_ERROR_OK);
	TEST_ASSERT(sys_gid_get()==1000);
	_sys_syscall1(gid_syscall_table_offset|0x00000001,0);
	TEST_FUNC("sys_gid_get_name");
	TEST_GROUP("invalid buffer length");
	char buffer_group[256];
	TEST_ASSERT(sys_gid_get_name(0,buffer_group,0)==SYS_ERROR_INVALID_ARGUMENT(2));
	TEST_GROUP("invalid buffer");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
	TEST_ASSERT(sys_gid_get_name(0,(void*)1,256)==SYS_ERROR_INVALID_ARGUMENT(1));
#pragma GCC diagnostic pop
	TEST_GROUP("invalid gid");
	TEST_ASSERT(sys_gid_get_name(0xaabbccdd,buffer_group,256)==SYS_ERROR_NOT_FOUND);
	TEST_GROUP("correct args");
	TEST_ASSERT(sys_gid_get_name(0,buffer_group,256)==SYS_ERROR_OK);
	TEST_ASSERT(!sys_string_compare(buffer_group,"root"));
	TEST_FUNC("sys_uid_get");
	TEST_GROUP("correct args");
	TEST_ASSERT(!sys_uid_get());
	_sys_syscall1(uid_syscall_table_offset|0x00000001,1000);
	TEST_ASSERT(sys_uid_get()==1000);
	_sys_syscall1(uid_syscall_table_offset|0x00000001,0);
	TEST_FUNC("sys_uid_set");
	TEST_GROUP("not root");
	_sys_syscall1(uid_syscall_table_offset|0x00000001,1000);
	_sys_syscall1(gid_syscall_table_offset|0x00000001,1000);
	TEST_ASSERT(sys_uid_set(1001)==SYS_ERROR_DENIED);
	_sys_syscall1(uid_syscall_table_offset|0x00000001,0);
	_sys_syscall1(gid_syscall_table_offset|0x00000001,0);
	TEST_GROUP("correct args");
	TEST_ASSERT(!sys_uid_get());
	TEST_ASSERT(sys_uid_set(1000)==SYS_ERROR_OK);
	TEST_ASSERT(sys_uid_get()==1000);
	_sys_syscall1(uid_syscall_table_offset|0x00000001,0);
	TEST_FUNC("sys_uid_get_name");
	TEST_GROUP("invalid buffer length");
	char buffer_user[256];
	TEST_ASSERT(sys_uid_get_name(0,buffer_user,0)==SYS_ERROR_INVALID_ARGUMENT(2));
	TEST_GROUP("invalid buffer");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
	TEST_ASSERT(sys_uid_get_name(0,(void*)1,256)==SYS_ERROR_INVALID_ARGUMENT(1));
#pragma GCC diagnostic pop
	TEST_GROUP("invalid uid");
	TEST_ASSERT(sys_uid_get_name(0xaabbccdd,buffer_user,256)==SYS_ERROR_NOT_FOUND);
	TEST_GROUP("correct args");
	TEST_ASSERT(sys_uid_get_name(0,buffer_user,256)==SYS_ERROR_OK);
	TEST_ASSERT(!sys_string_compare(buffer_user,"root"));
}
