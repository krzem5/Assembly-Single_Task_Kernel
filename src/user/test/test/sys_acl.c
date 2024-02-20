#include <sys/acl/acl.h>
#include <sys/error/error.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <test/test.h>



void test_sys_acl(void){
	TEST_MODULE("sys_acl");
	TEST_FUNC("sys_acl_get_permissions");
	TEST_GROUP("null arguments");
	TEST_ASSERT(sys_acl_get_permissions(0,0)==SYS_ERROR_INVALID_HANDLE);
	TEST_GROUP("invalid handle");
	TEST_ASSERT(sys_acl_get_permissions(0xaabbccdd,0)==SYS_ERROR_INVALID_HANDLE);
	// TEST_GROUP("handle without ACL");
	// handle_type_t handle_type=handle_alloc("test-acl-handle",NULL);
	// handle_t handle;
	// handle_new(&handle,handle_type,&handle);
	// handle_finish_setup(&handle);
	// TEST_ASSERT(sys_acl_get_permissions(handle.rb_node.key,0)==SYS_ERROR_NO_ACL);
	// TEST_GROUP("default permissions");
	// handle.acl=acl_create();
	// TEST_ASSERT(!sys_acl_get_permissions(handle.rb_node.key,0));
	// TEST_GROUP("invalid process handle");
	// TEST_ASSERT(sys_acl_get_permissions(handle.rb_node.key,0xaabbccdd)==SYS_ERROR_INVALID_HANDLE);
	// TEST_GROUP("default other process permissions");
	// TEST_ASSERT(!sys_acl_get_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key));
	// TEST_GROUP("correct args");
	// acl_set(handle.acl,THREAD_DATA->process,0,0xabcd);
	// TEST_ASSERT(sys_acl_get_permissions(handle.rb_node.key,0)==0xabcd);
	// TEST_ASSERT(!sys_acl_get_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key));
	// acl_set(handle.acl,second_test_process,0,0x44556677);
	// TEST_ASSERT(sys_acl_get_permissions(handle.rb_node.key,0)==0xabcd);
	// TEST_ASSERT(sys_acl_get_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key)==0x44556677);
	TEST_FUNC("sys_acl_set_permissions");
	// acl_delete(handle.acl);
	// handle.acl=NULL;
	TEST_GROUP("invalid clear flags");
	TEST_ASSERT(sys_acl_set_permissions(0,0,0xffffffffffffffff,0)==SYS_ERROR_INVALID_ARGUMENT(2));
	TEST_GROUP("invalid set flags");
	TEST_ASSERT(sys_acl_set_permissions(0,0,0,0xffffffffffffffff)==SYS_ERROR_INVALID_ARGUMENT(3));
	TEST_GROUP("invalid handle");
	TEST_ASSERT(sys_acl_set_permissions(0xaabbccdd,0,0,0)==SYS_ERROR_INVALID_HANDLE);
	// TEST_GROUP("handle without ACL");
	// TEST_ASSERT(sys_acl_set_permissions(handle.rb_node.key,0,0,0)==SYS_ERROR_NO_ACL);
	// TEST_GROUP("invalid process handle");
	// handle.acl=acl_create();
	// TEST_ASSERT(sys_acl_set_permissions(handle.rb_node.key,0xaabbccdd,0,0)==SYS_ERROR_INVALID_HANDLE);
	// TEST_GROUP("correct args");
	// acl_set(handle.acl,THREAD_DATA->process,0,ACL_PERMISSION_MASK);
	// TEST_ASSERT(sys_acl_set_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key,0,0x1234)==SYS_ERROR_OK);
	// TEST_ASSERT(sys_acl_get_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key)==0x1234);
	// TEST_ASSERT(sys_acl_set_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key,0x0204,0x8000)==SYS_ERROR_OK);
	// TEST_ASSERT(sys_acl_get_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key)==0x9030);
	// TEST_GROUP("correct args, masked by current process permissions");
	// acl_set(handle.acl,THREAD_DATA->process,ACL_PERMISSION_MASK,0x00f3);
	// TEST_ASSERT(sys_acl_get_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key)==0x9030);
	// TEST_ASSERT(sys_acl_set_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key,0x9f20,0x000f)==SYS_ERROR_OK);
	// TEST_ASSERT(sys_acl_get_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key)==0x0013);
	TEST_FUNC("sys_acl_request_permissions");
	// acl_delete(handle.acl);
	// handle.acl=NULL;
	TEST_GROUP("invalid flags");
	TEST_ASSERT(sys_acl_request_permissions(0,0,0xffffffffffffffff)==SYS_ERROR_INVALID_ARGUMENT(2));
	TEST_GROUP("invalid handle");
	TEST_ASSERT(sys_acl_request_permissions(0xaabbccdd,0,0)==SYS_ERROR_INVALID_HANDLE);
	// TEST_GROUP("handle without ACL");
	// TEST_ASSERT(sys_acl_request_permissions(handle.rb_node.key,0,0)==SYS_ERROR_NO_ACL);
	// TEST_GROUP("denied request without callback");
	// handle.acl=acl_create();
	// TEST_ASSERT(sys_acl_request_permissions(handle.rb_node.key,0,1)==SYS_ERROR_DENIED);
	// TEST_GROUP("invalid process handle");
	// TEST_ASSERT(sys_acl_request_permissions(handle.rb_node.key,0xaabbccdd,1)==SYS_ERROR_INVALID_HANDLE);
	// TEST_GROUP("denied request for other process without callback");
	// TEST_ASSERT(sys_acl_request_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key,1)==SYS_ERROR_DENIED);
	// TEST_GROUP("request accepted");
	// TEST_ASSERT(acl_register_request_callback(_permission_request_callback)==1);
	// _permission_request_response=SYS_ERROR_OK;
	// TEST_ASSERT(sys_acl_request_permissions(handle.rb_node.key,0,0x12345)==SYS_ERROR_OK);
	// TEST_ASSERT(_permission_request_handle==&handle);
	// TEST_ASSERT(_permission_request_process==THREAD_DATA->process);
	// TEST_ASSERT(_permission_request_flags==0x12345);
	// TEST_ASSERT(sys_acl_get_permissions(handle.rb_node.key,0)==0x12345);
	// TEST_GROUP("request for other process");
	// TEST_ASSERT(sys_acl_request_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key,0xabcd)==SYS_ERROR_OK);
	// TEST_ASSERT(_permission_request_handle==&handle);
	// TEST_ASSERT(_permission_request_process==second_test_process);
	// TEST_ASSERT(_permission_request_flags==0xabcd);
	// TEST_ASSERT(sys_acl_get_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key)==0xabcd);
	// TEST_GROUP("request denied");
	// _permission_request_response=_ERROR(0xaabb);
	// TEST_ASSERT(sys_acl_request_permissions(handle.rb_node.key,0,0xffbb)==_ERROR(0xaabb));
	// TEST_ASSERT(_permission_request_handle==&handle);
	// TEST_ASSERT(_permission_request_process==THREAD_DATA->process);
	// TEST_ASSERT(_permission_request_flags==0xffbb);
	// TEST_ASSERT(sys_acl_get_permissions(handle.rb_node.key,0)==0x12345);
	// TEST_ASSERT(acl_register_request_callback(NULL)==1);
	// handle_release(&handle);
}
