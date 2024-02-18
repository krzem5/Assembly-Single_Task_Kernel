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
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_signature"));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_word_size"));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_endianess"));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_header_version"));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_abi"));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_type"));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_machine"));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_version"));
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
	TEST_FUNC("module_unload");
	TEST_GROUP("already unloaded module");
	module_t* module=module_load("test_module_prevent_future_loads");
	TEST_ASSERT(module);
	_test_module_deinit_triggered=0;
	u64 module_handle=module->handle.rb_node.key;
	TEST_ASSERT(module_unload(module));
	TEST_ASSERT(_test_module_deinit_triggered==1);
	TEST_ASSERT(!module_unload(module));
	TEST_GROUP("prevent future module loads");
	TEST_ASSERT(module_load("test_module_prevent_future_loads")->handle.rb_node.key==module_handle);
	TEST_GROUP("correct args");
	module=module_load("test_module_normal_module");
	TEST_ASSERT(module);
	_test_module_deinit_triggered=0;
	module_handle=module->handle.rb_node.key;
	TEST_ASSERT(module_unload(module));
	TEST_ASSERT(_test_module_deinit_triggered==1);
	TEST_ASSERT(!module_unload(module));
	module_t* new_module=module_load("test_module_normal_module");
	TEST_ASSERT(new_module);
	TEST_ASSERT(new_module->handle.rb_node.key!=module_handle);
	TEST_ASSERT(module_unload(new_module));
	TEST_FUNC("module_lookup");
	TEST_GROUP("loaded module");
	TEST_ASSERT(module_lookup("test"));
	TEST_GROUP("unloaded module");
	TEST_ASSERT(module_lookup("module_loader"));
	TEST_GROUP("not found");
	TEST_ASSERT(!module_lookup("invalid_module_name"));
}
