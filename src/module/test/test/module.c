#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



void test_module(void){
	TEST_MODULE("module");
	TEST_FUNC("module_load");
	TEST_GROUP("no module descriptor");
	TEST_ASSERT(!module_load("test_module_no_descriptor"));
}
