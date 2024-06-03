#include <kernel/acl/acl.h>
#include <kernel/error/error.h>
#include <kernel/handle/handle.h>
#include <kernel/log/log.h>
#include <kernel/mp/event.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



extern error_t syscall_acl_get_permissions();
extern error_t syscall_acl_set_permissions();
extern error_t syscall_acl_request_permissions();



static handle_t* _test_acl_permission_request_handle=NULL;
static process_t* _test_acl_permission_request_process=NULL;
static u64 _test_acl_permission_request_flags=0;
static error_t _test_acl_permission_request_response=ERROR_OK;
static handle_t _test_acl_handle_without_acl;
static handle_t _test_acl_handle;
static process_t* _test_acl_second_test_process;



static error_t _permission_request_callback(handle_t* handle,process_t* process,u64 flags){
	_test_acl_permission_request_handle=handle;
	_test_acl_permission_request_process=process;
	_test_acl_permission_request_flags=flags;
	return _test_acl_permission_request_response;
}



static u64 _syscall_get_test_handle_without_acl(void){
	return _test_acl_handle_without_acl.rb_node.key;
}



static u64 _syscall_get_test_handle(void){
	return _test_acl_handle.rb_node.key;
}



static void _syscall_set_test_handle_flags(u64 process,u64 clear,u64 set){
	handle_t* handle=handle_lookup_and_acquire(process,process_handle_type);
	acl_set(_test_acl_handle.acl,(handle?KERNEL_CONTAINEROF(handle,process_t,handle):THREAD_DATA->process),clear,set);
	if (handle){
		handle_release(handle);
	}
}



static u64 _syscall_get_second_test_process(void){
	return _test_acl_second_test_process->handle.rb_node.key;
}



static syscall_callback_t const _test_sys_acl_syscall_functions[]={
	[1]=(syscall_callback_t)_syscall_get_test_handle_without_acl,
	[2]=(syscall_callback_t)_syscall_get_test_handle,
	[3]=(syscall_callback_t)_syscall_set_test_handle_flags,
	[4]=(syscall_callback_t)_syscall_get_second_test_process
};



static void _thread(void){
	TEST_FUNC("syscall_acl_get_permissions");
	TEST_GROUP("null arguments");
	TEST_ASSERT(syscall_acl_get_permissions(0,0)==ERROR_INVALID_HANDLE);
	TEST_GROUP("invalid handle");
	TEST_ASSERT(syscall_acl_get_permissions(0xaabbccdd,0)==ERROR_INVALID_HANDLE);
	TEST_GROUP("handle without ACL");
	TEST_ASSERT(syscall_acl_get_permissions(_test_acl_handle_without_acl.rb_node.key,0)==ERROR_NO_ACL);
	TEST_GROUP("default permissions");
	TEST_ASSERT(!syscall_acl_get_permissions(_test_acl_handle.rb_node.key,0));
	TEST_GROUP("invalid process handle");
	TEST_ASSERT(syscall_acl_get_permissions(_test_acl_handle.rb_node.key,0xaabbccdd)==ERROR_INVALID_HANDLE);
	TEST_GROUP("default other process permissions");
	TEST_ASSERT(!syscall_acl_get_permissions(_test_acl_handle.rb_node.key,_test_acl_second_test_process->handle.rb_node.key));
	TEST_GROUP("correct args");
	acl_set(_test_acl_handle.acl,THREAD_DATA->process,0,0xabcd);
	TEST_ASSERT(syscall_acl_get_permissions(_test_acl_handle.rb_node.key,0)==0xabcd);
	TEST_ASSERT(!syscall_acl_get_permissions(_test_acl_handle.rb_node.key,_test_acl_second_test_process->handle.rb_node.key));
	acl_set(_test_acl_handle.acl,_test_acl_second_test_process,0,0x44556677);
	TEST_ASSERT(syscall_acl_get_permissions(_test_acl_handle.rb_node.key,0)==0xabcd);
	TEST_ASSERT(syscall_acl_get_permissions(_test_acl_handle.rb_node.key,_test_acl_second_test_process->handle.rb_node.key)==0x44556677);
	acl_set(_test_acl_handle.acl,_test_acl_second_test_process,ACL_PERMISSION_MASK,0);
	TEST_FUNC("syscall_acl_set_permissions");
	TEST_GROUP("invalid clear flags");
	TEST_ASSERT(syscall_acl_set_permissions(0,0,0xffffffffffffffff,0)==ERROR_INVALID_ARGUMENT(2));
	TEST_GROUP("invalid set flags");
	TEST_ASSERT(syscall_acl_set_permissions(0,0,0,0xffffffffffffffff)==ERROR_INVALID_ARGUMENT(3));
	TEST_GROUP("invalid handle");
	TEST_ASSERT(syscall_acl_set_permissions(0xaabbccdd,0,0,0)==ERROR_INVALID_HANDLE);
	TEST_GROUP("handle without ACL");
	TEST_ASSERT(syscall_acl_set_permissions(_test_acl_handle_without_acl.rb_node.key,0,0,0)==ERROR_NO_ACL);
	TEST_GROUP("invalid process handle");
	TEST_ASSERT(syscall_acl_set_permissions(_test_acl_handle.rb_node.key,0xaabbccdd,0,0)==ERROR_INVALID_HANDLE);
	TEST_GROUP("correct args");
	acl_set(_test_acl_handle.acl,THREAD_DATA->process,0,ACL_PERMISSION_MASK);
	TEST_ASSERT(syscall_acl_set_permissions(_test_acl_handle.rb_node.key,_test_acl_second_test_process->handle.rb_node.key,0,0x1234)==ERROR_OK);
	TEST_ASSERT(syscall_acl_get_permissions(_test_acl_handle.rb_node.key,_test_acl_second_test_process->handle.rb_node.key)==0x1234);
	TEST_ASSERT(syscall_acl_set_permissions(_test_acl_handle.rb_node.key,_test_acl_second_test_process->handle.rb_node.key,0x0204,0x8000)==ERROR_OK);
	TEST_ASSERT(syscall_acl_get_permissions(_test_acl_handle.rb_node.key,_test_acl_second_test_process->handle.rb_node.key)==0x9030);
	TEST_GROUP("correct args, masked by current process permissions");
	acl_set(_test_acl_handle.acl,THREAD_DATA->process,ACL_PERMISSION_MASK,0x00f3);
	TEST_ASSERT(syscall_acl_get_permissions(_test_acl_handle.rb_node.key,_test_acl_second_test_process->handle.rb_node.key)==0x9030);
	TEST_ASSERT(syscall_acl_set_permissions(_test_acl_handle.rb_node.key,_test_acl_second_test_process->handle.rb_node.key,0x9f20,0x000f)==ERROR_OK);
	TEST_ASSERT(syscall_acl_get_permissions(_test_acl_handle.rb_node.key,_test_acl_second_test_process->handle.rb_node.key)==0x0013);
	acl_set(_test_acl_handle.acl,THREAD_DATA->process,ACL_PERMISSION_MASK,0);
	acl_set(_test_acl_handle.acl,_test_acl_second_test_process,ACL_PERMISSION_MASK,0);
	TEST_FUNC("syscall_acl_request_permissions");
	TEST_GROUP("invalid flags");
	TEST_ASSERT(syscall_acl_request_permissions(0,0,0xffffffffffffffff)==ERROR_INVALID_ARGUMENT(2));
	TEST_GROUP("invalid handle");
	TEST_ASSERT(syscall_acl_request_permissions(0xaabbccdd,0,0)==ERROR_INVALID_HANDLE);
	TEST_GROUP("handle without ACL");
	TEST_ASSERT(syscall_acl_request_permissions(_test_acl_handle_without_acl.rb_node.key,0,0)==ERROR_NO_ACL);
	TEST_GROUP("denied request without callback");
	TEST_ASSERT(syscall_acl_request_permissions(_test_acl_handle.rb_node.key,0,1)==ERROR_DENIED);
	TEST_GROUP("invalid process handle");
	TEST_ASSERT(syscall_acl_request_permissions(_test_acl_handle.rb_node.key,0xaabbccdd,1)==ERROR_INVALID_HANDLE);
	TEST_GROUP("denied request for other process without callback");
	TEST_ASSERT(syscall_acl_request_permissions(_test_acl_handle.rb_node.key,_test_acl_second_test_process->handle.rb_node.key,1)==ERROR_DENIED);
	TEST_GROUP("request accepted");
	TEST_ASSERT(acl_register_request_callback(_permission_request_callback)==1);
	_test_acl_permission_request_response=ERROR_OK;
	TEST_ASSERT(syscall_acl_request_permissions(_test_acl_handle.rb_node.key,0,0x12345)==ERROR_OK);
	TEST_ASSERT(_test_acl_permission_request_handle==&_test_acl_handle);
	TEST_ASSERT(_test_acl_permission_request_process==THREAD_DATA->process);
	TEST_ASSERT(_test_acl_permission_request_flags==0x12345);
	TEST_ASSERT(syscall_acl_get_permissions(_test_acl_handle.rb_node.key,0)==0x12345);
	TEST_GROUP("request for other process");
	TEST_ASSERT(syscall_acl_request_permissions(_test_acl_handle.rb_node.key,_test_acl_second_test_process->handle.rb_node.key,0xabcd)==ERROR_OK);
	TEST_ASSERT(_test_acl_permission_request_handle==&_test_acl_handle);
	TEST_ASSERT(_test_acl_permission_request_process==_test_acl_second_test_process);
	TEST_ASSERT(_test_acl_permission_request_flags==0xabcd);
	TEST_ASSERT(syscall_acl_get_permissions(_test_acl_handle.rb_node.key,_test_acl_second_test_process->handle.rb_node.key)==0xabcd);
	TEST_GROUP("request denied");
	_test_acl_permission_request_response=_ERROR(0xaabb);
	TEST_ASSERT(syscall_acl_request_permissions(_test_acl_handle.rb_node.key,0,0xffbb)==_ERROR(0xaabb));
	TEST_ASSERT(_test_acl_permission_request_handle==&_test_acl_handle);
	TEST_ASSERT(_test_acl_permission_request_process==THREAD_DATA->process);
	TEST_ASSERT(_test_acl_permission_request_flags==0xffbb);
	TEST_ASSERT(syscall_acl_get_permissions(_test_acl_handle.rb_node.key,0)==0x12345);
	TEST_ASSERT(acl_register_request_callback(NULL)==1);
	acl_set(_test_acl_handle.acl,THREAD_DATA->process,ACL_PERMISSION_MASK,0);
	acl_set(_test_acl_handle.acl,_test_acl_second_test_process,ACL_PERMISSION_MASK,0);
}



void test_acl(void){
	TEST_MODULE("acl");
	TEST_FUNC("acl_create");
	TEST_GROUP("correct args");
	acl_t* acl=acl_create();
	TEST_ASSERT(acl);
	TEST_FUNC("acl_delete");
	TEST_GROUP("correct args");
	acl_delete(acl);
	TEST_FUNC("acl_get");
	acl=acl_create();
	TEST_GROUP("kernel permissions");
	TEST_ASSERT(acl_get(acl,process_kernel)==ACL_PERMISSION_MASK);
	acl_set(acl,process_kernel,0,ACL_PERMISSION_MASK);
	TEST_ASSERT(acl_get(acl,process_kernel)==ACL_PERMISSION_MASK);
	acl_set(acl,process_kernel,ACL_PERMISSION_MASK,0);
	TEST_ASSERT(acl_get(acl,process_kernel)==ACL_PERMISSION_MASK);
	acl_set(acl,process_kernel,ACL_PERMISSION_MASK,ACL_PERMISSION_MASK);
	TEST_ASSERT(acl_get(acl,process_kernel)==ACL_PERMISSION_MASK);
	process_t* test_process=process_create("test-process","test-process",0x1000,0x3000);
	process_t* test_process_buffer[ACL_PROCESS_CACHE_SIZE-1];
	for (u32 i=0;i<ACL_PROCESS_CACHE_SIZE-1;i++){
		test_process_buffer[i]=process_create("filler-test-process","filler-test-process",0x1000,0x3000);
	}
	_test_acl_second_test_process=process_create("second-test-process","second-test-process",0x1000,0x3000);
	TEST_GROUP("default permissions");
	TEST_ASSERT(!acl_get(acl,test_process));
	TEST_FUNC("acl_set");
	TEST_GROUP("no change without permissions");
	acl_set(acl,test_process,0,0);
	TEST_ASSERT(!acl_get(acl,test_process));
	TEST_GROUP("clear all permissions without permissions");
	acl_set(acl,test_process,ACL_PERMISSION_MASK,0);
	TEST_ASSERT(!acl_get(acl,test_process));
	TEST_GROUP("set permissions");
	acl_set(acl,test_process,0,0xffff);
	TEST_ASSERT(acl_get(acl,test_process)==0xffff);
	TEST_GROUP("no change");
	acl_set(acl,test_process,0,0);
	TEST_ASSERT(acl_get(acl,test_process)==0xffff);
	TEST_GROUP("clear permissions");
	acl_set(acl,test_process,0x0088,0);
	TEST_ASSERT(acl_get(acl,test_process)==0xff77);
	TEST_GROUP("clear and set permissions");
	acl_set(acl,test_process,0x0700,0x0080);
	TEST_ASSERT(acl_get(acl,test_process)==0xf8f7);
	TEST_GROUP("clear all permissions");
	acl_set(acl,test_process,ACL_PERMISSION_MASK,0);
	TEST_ASSERT(!acl_get(acl,test_process));
	TEST_GROUP("cache repopulation");
	acl_set(acl,_test_acl_second_test_process,0,1);
	acl_set(acl,test_process,0,1);
	acl_set(acl,test_process,1,0);
	TEST_ASSERT(!acl_get(acl,test_process));
	TEST_ASSERT(acl_get(acl,_test_acl_second_test_process)==1);
	acl_set(acl,_test_acl_second_test_process,1,0);
	TEST_ASSERT(!acl_get(acl,test_process));
	TEST_ASSERT(!acl_get(acl,_test_acl_second_test_process));
	for (u32 i=0;i<ACL_PROCESS_CACHE_SIZE-1;i++){
		(void)test_process_buffer;
		// handle_release(&(test_process_buffer[i]->handle));
	}
	acl_delete(acl);
	TEST_FUNC("acl_register_request_callback");
	TEST_GROUP("remove callback");
	TEST_ASSERT(acl_register_request_callback(NULL)==1);
	TEST_GROUP("register callback");
	TEST_ASSERT(acl_register_request_callback(_permission_request_callback)==1);
	TEST_GROUP("callback already registered");
	TEST_ASSERT(!acl_register_request_callback(_permission_request_callback));
	TEST_ASSERT(acl_register_request_callback(NULL)==1);
	handle_type_t handle_type=handle_alloc("test.acl_handle",0,NULL);
	handle_new(handle_type,&_test_acl_handle_without_acl);
	handle_new(handle_type,&_test_acl_handle);
	_test_acl_handle.acl=acl_create();
	handle_acquire(&(test_process->handle));
	scheduler_enqueue_thread(thread_create_kernel_thread(test_process,"test.acl",_thread,0));
	event_await(test_process->event,0);
	handle_release(&(test_process->handle));
	syscall_create_table("test_sys_acl",_test_sys_acl_syscall_functions,sizeof(_test_sys_acl_syscall_functions)/sizeof(syscall_callback_t));
}
