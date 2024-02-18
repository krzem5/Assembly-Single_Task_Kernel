#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



static _Bool _test_module_deinit_triggered=0;



KERNEL_PUBLIC void test_module_mark_deinit_triggered(void){
	_test_module_deinit_triggered=1;
}



void test_module(void){
	TEST_MODULE("module");
	TEST_FUNC("module_load");
	TEST_GROUP("module already loaded");
	TEST_ASSERT(module_load("test")==module_self);
	TEST_GROUP("module not found");
	TEST_ASSERT(!module_load("invalid_module_name"));
	TEST_GROUP("invalid elf header");
	// invalid elf header
	TEST_GROUP("no module section");
	TEST_ASSERT(!module_load("test_module_no_module_section"));
	TEST_GROUP("wrong module section size");
	TEST_ASSERT(!module_load("test_module_wrong_module_section_size"));
	TEST_GROUP("unresolved symbols");
	TEST_ASSERT(!module_load("test_module_unresolved_symbols"));
	TEST_GROUP("failed init");
	_test_module_deinit_triggered=0;
	TEST_ASSERT(!module_load("test_module_failed_init"));
	TEST_ASSERT(_test_module_deinit_triggered==1);
}
