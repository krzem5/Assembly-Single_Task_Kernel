#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test_handle_list"



void test_handle_list(void){
	LOG("Executing handle list tests...");
	handle_type_t handle_type=handle_alloc("test-handle-list-handle",NULL);
	handle_t handle;
	handle_new(&handle,handle_type,&handle);
	handle_finish_setup(&handle);
	TEST_ASSERT(!handle.handle_list);
	handle_list_t list;
	handle_list_init(&list);
	handle_list_push(&list,&handle);
	TEST_ASSERT(handle.handle_list==&list);
	handle_list_push(&list,&handle);
	TEST_ASSERT(handle.handle_list==&list);
	handle_list_pop(&handle);
	TEST_ASSERT(!handle.handle_list);
	handle_list_pop(&handle);
	TEST_ASSERT(!handle.handle_list);
	handle_list_destroy(&list);
	handle_release(&handle);
}
