#include <sys/acl/acl.h>
#include <sys/error/error.h>
#include <sys/syscall/kernel_syscalls.h>
#include <sys/syscall/syscall.h>
#include <sys/types.h>
#include <test/test.h>



void test_sys_acl(void){
	u64 syscall_table_offset=sys_syscall_get_table_offset("test_sys_acl");
	TEST_MODULE("sys_acl");
	TEST_FUNC("sys_acl_get_permissions");
	TEST_GROUP("null arguments");
	TEST_ASSERT(sys_acl_get_permissions(0,0)==SYS_ERROR_INVALID_HANDLE);
	TEST_GROUP("invalid handle");
	TEST_ASSERT(sys_acl_get_permissions(0xaabbccdd,0)==SYS_ERROR_INVALID_HANDLE);
	TEST_GROUP("handle without ACL");
	u64 handle_without_acl=_sys_syscall0(syscall_table_offset|0x00000001);
	TEST_ASSERT(sys_acl_get_permissions(handle_without_acl,0)==SYS_ERROR_NO_ACL);
	TEST_GROUP("default permissions");
	u64 handle=_sys_syscall0(syscall_table_offset|0x00000002);
	TEST_ASSERT(!sys_acl_get_permissions(handle,0));
	TEST_GROUP("invalid process handle");
	TEST_ASSERT(sys_acl_get_permissions(handle,0xaabbccdd)==SYS_ERROR_INVALID_HANDLE);
	TEST_GROUP("default other process permissions");
	u64 second_test_process=_sys_syscall0(syscall_table_offset|0x00000004);
	TEST_ASSERT(!sys_acl_get_permissions(handle,second_test_process));
	TEST_GROUP("correct args");
	_sys_syscall3(syscall_table_offset|0x00000003,0,0,0xabcd);
	TEST_ASSERT(sys_acl_get_permissions(handle,0)==0xabcd);
	TEST_ASSERT(!sys_acl_get_permissions(handle,second_test_process));
	_sys_syscall3(syscall_table_offset|0x00000003,second_test_process,0,0x44556677);
	TEST_ASSERT(sys_acl_get_permissions(handle,0)==0xabcd);
	TEST_ASSERT(sys_acl_get_permissions(handle,second_test_process)==0x44556677);
	_sys_syscall3(syscall_table_offset|0x00000003,0,SYS_ACL_PERMISSION_FLAG_MASK,0);
	_sys_syscall3(syscall_table_offset|0x00000003,second_test_process,SYS_ACL_PERMISSION_FLAG_MASK,0);
	TEST_FUNC("sys_acl_set_permissions");
	TEST_GROUP("invalid clear flags");
	TEST_ASSERT(sys_acl_set_permissions(0,0,0xffffffffffffffff,0)==SYS_ERROR_INVALID_ARGUMENT(2));
	TEST_GROUP("invalid set flags");
	TEST_ASSERT(sys_acl_set_permissions(0,0,0,0xffffffffffffffff)==SYS_ERROR_INVALID_ARGUMENT(3));
	TEST_GROUP("invalid handle");
	TEST_ASSERT(sys_acl_set_permissions(0xaabbccdd,0,0,0)==SYS_ERROR_INVALID_HANDLE);
	TEST_GROUP("handle without ACL");
	TEST_ASSERT(sys_acl_set_permissions(handle_without_acl,0,0,0)==SYS_ERROR_NO_ACL);
	TEST_GROUP("invalid process handle");
	TEST_ASSERT(sys_acl_set_permissions(handle,0xaabbccdd,0,0)==SYS_ERROR_INVALID_HANDLE);
	TEST_GROUP("correct args");
	_sys_syscall3(syscall_table_offset|0x00000003,0,0,SYS_ACL_PERMISSION_FLAG_MASK);
	TEST_ASSERT(sys_acl_set_permissions(handle,second_test_process,0,0x1234)==SYS_ERROR_OK);
	TEST_ASSERT(sys_acl_get_permissions(handle,second_test_process)==0x1234);
	TEST_ASSERT(sys_acl_set_permissions(handle,second_test_process,0x0204,0x8000)==SYS_ERROR_OK);
	TEST_ASSERT(sys_acl_get_permissions(handle,second_test_process)==0x9030);
	TEST_GROUP("correct args, masked by current process permissions");
	_sys_syscall3(syscall_table_offset|0x00000003,0,SYS_ACL_PERMISSION_FLAG_MASK,0x00f3);
	TEST_ASSERT(sys_acl_get_permissions(handle,second_test_process)==0x9030);
	TEST_ASSERT(sys_acl_set_permissions(handle,second_test_process,0x9f20,0x000f)==SYS_ERROR_OK);
	TEST_ASSERT(sys_acl_get_permissions(handle,second_test_process)==0x0013);
	_sys_syscall3(syscall_table_offset|0x00000003,0,SYS_ACL_PERMISSION_FLAG_MASK,0);
	_sys_syscall3(syscall_table_offset|0x00000003,second_test_process,SYS_ACL_PERMISSION_FLAG_MASK,0);
	TEST_FUNC("sys_acl_request_permissions");
	TEST_GROUP("invalid flags");
	TEST_ASSERT(sys_acl_request_permissions(0,0,0xffffffffffffffff)==SYS_ERROR_INVALID_ARGUMENT(2));
	TEST_GROUP("invalid handle");
	TEST_ASSERT(sys_acl_request_permissions(0xaabbccdd,0,0)==SYS_ERROR_INVALID_HANDLE);
	TEST_GROUP("handle without ACL");
	TEST_ASSERT(sys_acl_request_permissions(handle_without_acl,0,0)==SYS_ERROR_NO_ACL);
	TEST_GROUP("denied request without callback");
	TEST_ASSERT(sys_acl_request_permissions(handle,0,1)==SYS_ERROR_DENIED);
	TEST_GROUP("invalid process handle");
	TEST_ASSERT(sys_acl_request_permissions(handle,0xaabbccdd,1)==SYS_ERROR_INVALID_HANDLE);
	TEST_GROUP("denied request for other process without callback");
	TEST_ASSERT(sys_acl_request_permissions(handle,second_test_process,1)==SYS_ERROR_DENIED);
}
