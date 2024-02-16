#if KERNEL_COVERAGE_ENABLED
#include <kernel/acl/acl.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "test_acl"



#define TEST_ASSERT(x) \
	do{ \
		if (!(x)){ \
			ERROR("%u: %s: Test failed",__LINE__,#x); \
		} \
	} while (0)



extern error_t syscall_acl_get_permissions();



static error_t _permission_request_response=ERROR_OK;



static error_t _permission_request_callback(handle_t* handle,process_t* process,u64 flags){
	return _permission_request_response;
}



static void _thread(process_t* second_test_process){
	TEST_ASSERT(syscall_acl_get_permissions(0,0)==ERROR_INVALID_HANDLE);
	TEST_ASSERT(syscall_acl_get_permissions(0xaabbccdd,0)==ERROR_INVALID_HANDLE);
	handle_type_t handle_type=handle_alloc("test-handle",NULL);
	handle_t handle;
	handle_new(&handle,handle_type,&handle);
	handle_finish_setup(&handle);
	TEST_ASSERT(syscall_acl_get_permissions(handle.rb_node.key,0)==ERROR_NO_ACL);
	handle.acl=acl_create();
	TEST_ASSERT(!syscall_acl_get_permissions(handle.rb_node.key,0));
	TEST_ASSERT(syscall_acl_get_permissions(handle.rb_node.key,0xaabbccdd)==ERROR_INVALID_HANDLE);
	TEST_ASSERT(!syscall_acl_get_permissions(handle.rb_node.key,second_test_process->handle.rb_node.key));
	handle_release(&handle);
	WARN("test-acl-thread");
}



static void _thread2(void){
	WARN("test-acl-thread2");
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
	process_t* second_test_process=process_create("test2-process","test2-process");
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
	acl_set(acl,second_test_process,0,1);
	acl_set(acl,test_process,0,1);
	acl_set(acl,test_process,1,0);
	TEST_ASSERT(!acl_get(acl,test_process));
	TEST_ASSERT(acl_get(acl,second_test_process)==1);
	acl_set(acl,second_test_process,1,0);
	TEST_ASSERT(!acl_get(acl,test_process));
	TEST_ASSERT(!acl_get(acl,second_test_process));
	for (u32 i=0;i<ACL_PROCESS_CACHE_SIZE-1;i++){
		(void)test_process_buffer;
		// handle_release(&(test_process_buffer[i]->handle));
	}
	acl_delete(acl);
	TEST_ASSERT(acl_register_request_callback(NULL)==1);
	TEST_ASSERT(acl_register_request_callback(_permission_request_callback)==1);
	TEST_ASSERT(!acl_register_request_callback(_permission_request_callback));
	scheduler_enqueue_thread(thread_create_kernel_thread(test_process,"test-acl-thread",_thread,0x200000,1,second_test_process));
	scheduler_enqueue_thread(thread_create_kernel_thread(second_test_process,"test-acl-thread2",_thread2,0x200000,0));
	event_await(test_process->event,0);
	event_await(second_test_process->event,0);
	TEST_ASSERT(acl_register_request_callback(NULL)==1);
}
#else
void coverage_test_acl(void){
	return;
}
#endif
