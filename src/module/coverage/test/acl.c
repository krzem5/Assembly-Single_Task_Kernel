#include <kernel/acl/acl.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/mp/process.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "test_acl"



#define TEST_ASSERT(x) \
	do{ \
		if (!(x)){ \
			ERROR("%s: Test failed",#x); \
		} \
	} while (0)



void coverage_test_acl(void){
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
	handle_release(&(test_process->handle));
	acl_delete(acl);
}
