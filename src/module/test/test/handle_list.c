#include <kernel/handle/handle.h>
#include <kernel/handle/handle_list.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



void test_handle_list(void){
	TEST_MODULE("handle_list");
	handle_type_t handle_type=handle_alloc("test.handle_list",NULL);
	handle_t handle;
	handle_new(&handle,handle_type,&handle);
	handle_finish_setup(&handle);
	TEST_ASSERT(!handle.handle_list);
	TEST_FUNC("handle_list_init");
	TEST_GROUP("correct args");
	handle_list_t list;
	handle_list_init(&list);
	TEST_FUNC("handle_list_push");
	TEST_GROUP("correct args");
	handle_list_push(&list,&handle);
	TEST_ASSERT(handle.handle_list==&list);
	TEST_GROUP("handle already in list");
	handle_list_push(&list,&handle);
	TEST_ASSERT(handle.handle_list==&list);
	TEST_FUNC("handle_list_pop");
	TEST_GROUP("correct args");
	handle_list_pop(&handle);
	TEST_ASSERT(!handle.handle_list);
	TEST_GROUP("handle not in list");
	handle_list_pop(&handle);
	TEST_ASSERT(!handle.handle_list);
	TEST_FUNC("handle_list_destroy");
	TEST_GROUP("release handles in list");
	handle_acquire(&handle);
	handle_list_push(&list,&handle);
	handle_list_destroy(&list);
	// TEST_ASSERT(handle.rc==1);
	handle_release(&handle);
}
