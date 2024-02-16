#include <kernel/acl/acl.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/mp/process.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "test_acl"



#define TEST_ASSERT(x) \
	do{ \
		if (!(x)){ \
			ERROR("%u: %s: Test failed",__LINE__,#x); \
		} \
	} while (0)



static error_t _permission_request_callback(handle_t* handle,process_t* process,u64 flags){
	return ERROR_OK;
}



void coverage_test_acl(void){
	LOG("Executing ACL tests...");
	acl_t* acl=acl_create();
	TEST_ASSERT(acl);
	acl_delete(acl);
	acl=acl_create();
	TEST_ASSERT(acl_get(acl,process_kernel)==ACL_PERMISSION_MASK);
	acl_set(acl,process_kernel,0,ACL_PERMISSION_MASK);
	TEST_ASSERT(acl_get(acl,process_kernel)==ACL_PERMISSION_MASK);
	acl_set(acl,process_kernel,ACL_PERMISSION_MASK,0);
	TEST_ASSERT(acl_get(acl,process_kernel)==ACL_PERMISSION_MASK);
	acl_set(acl,process_kernel,ACL_PERMISSION_MASK,ACL_PERMISSION_MASK);
	TEST_ASSERT(acl_get(acl,process_kernel)==ACL_PERMISSION_MASK);
	process_t* test_process=process_create("test-process","test-process");
	process_t* test_process_buffer[ACL_PROCESS_CACHE_SIZE-1];
	for (u32 i=0;i<ACL_PROCESS_CACHE_SIZE-1;i++){
		test_process_buffer[i]=process_create("filler-test-process","filler-test-process");
	}
	process_t* test2_process=process_create("test2-process","test2-process");
	TEST_ASSERT(!acl_get(acl,test_process));
	acl_set(acl,test_process,0,0);
	TEST_ASSERT(!acl_get(acl,test_process));
	acl_set(acl,test_process,ACL_PERMISSION_MASK,0);
	TEST_ASSERT(!acl_get(acl,test_process));
	acl_set(acl,test_process,0,0xffff);
	TEST_ASSERT(acl_get(acl,test_process)==0xffff);
	acl_set(acl,test_process,0,0);
	TEST_ASSERT(acl_get(acl,test_process)==0xffff);
	acl_set(acl,test_process,0x0088,0);
	TEST_ASSERT(acl_get(acl,test_process)==0xff77);
	acl_set(acl,test_process,0x0700,0x0080);
	TEST_ASSERT(acl_get(acl,test_process)==0xf8f7);
	acl_set(acl,test_process,ACL_PERMISSION_MASK,0);
	TEST_ASSERT(!acl_get(acl,test_process));
	acl_set(acl,test2_process,0,1);
	acl_set(acl,test_process,0,1);
	acl_set(acl,test_process,1,0);
	TEST_ASSERT(!acl_get(acl,test_process));
	TEST_ASSERT(acl_get(acl,test2_process)==1);
	acl_set(acl,test2_process,1,0);
	TEST_ASSERT(!acl_get(acl,test_process));
	TEST_ASSERT(!acl_get(acl,test2_process));
	handle_release(&(test_process->handle));
	for (u32 i=0;i<ACL_PROCESS_CACHE_SIZE-1;i++){
		(void)test_process_buffer;
		// handle_release(&(test_process_buffer[i]->handle));
	}
	handle_release(&(test2_process->handle));
	acl_delete(acl);
	TEST_ASSERT(acl_register_request_callback(NULL)==1);
	TEST_ASSERT(acl_register_request_callback(_permission_request_callback)==1);
	TEST_ASSERT(!acl_register_request_callback(_permission_request_callback));
	TEST_ASSERT(acl_register_request_callback(NULL)==1);
}
