#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <test/test.h>
#define KERNEL_LOG_NAME "test"



static bool _test_module_deinit_triggered=0;



KERNEL_PUBLIC void test_module_mark_deinit_triggered(void){
	_test_module_deinit_triggered=1;
}



KERNEL_AWAITS void test_module(void){
	TEST_MODULE("module");
	TEST_FUNC("module_load");
	TEST_GROUP("module already loaded");
	TEST_ASSERT(module_load("test",0)==module_self);
	handle_release(&(module_self->handle));
	TEST_GROUP("module not found");
	TEST_ASSERT(!module_load("invalid_module_name",0));
	TEST_GROUP("invalid elf header");
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_signature",0));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_word_size",0));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_endianess",0));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_header_version",0));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_abi",0));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_type",0));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_machine",0));
	TEST_ASSERT(!module_load("/share/test/module/invalid_header_version",0));
	TEST_GROUP("no module section");
	TEST_ASSERT(!module_load("test_module_no_module_section",0));
	TEST_GROUP("unresolved symbols");
	TEST_ASSERT(!module_load("test_module_unresolved_symbols",0));
	TEST_GROUP("failed init");
	_test_module_deinit_triggered=0;
	TEST_ASSERT(!module_load("test_module_failed_init",0));
	TEST_ASSERT(_test_module_deinit_triggered==1);
	TEST_FUNC("module_unload");
	TEST_GROUP("already unloaded module");
	module_t* module=module_load("test_module_prevent_future_loads",0);
	TEST_ASSERT(module);
	_test_module_deinit_triggered=0;
	u64 module_handle=module->handle.rb_node.key;
	TEST_ASSERT(module_unload(module));
	TEST_ASSERT(_test_module_deinit_triggered==1);
	TEST_ASSERT(!module_unload(module));
	TEST_GROUP("prevent future module loads");
	TEST_ASSERT(module_load("test_module_prevent_future_loads",0)->handle.rb_node.key==module_handle);
	handle_release(&(module->handle));
	handle_release(&(module->handle));
	TEST_GROUP("correct args");
	module=module_load("test_module_normal_module",0);
	TEST_ASSERT(module);
	_test_module_deinit_triggered=0;
	module_handle=module->handle.rb_node.key;
	TEST_ASSERT(module_unload(module));
	TEST_ASSERT(_test_module_deinit_triggered==1);
	TEST_ASSERT(!module_unload(module));
	handle_release(&(module->handle));
	module_t* new_module=module_load("test_module_normal_module",0);
	TEST_ASSERT(new_module);
	TEST_ASSERT(new_module->handle.rb_node.key!=module_handle);
	TEST_ASSERT(module_unload(new_module));
	handle_release(&(new_module->handle));
	TEST_FUNC("module_lookup");
	TEST_GROUP("loaded module");
	module=module_lookup("test");
	TEST_ASSERT(module);
	handle_release(&(module->handle));
	TEST_GROUP("unloaded module");
	module=module_lookup("module_loader");
	TEST_ASSERT(module);
	handle_release(&(module->handle));
	TEST_GROUP("not found");
	TEST_ASSERT(!module_lookup("invalid_module_name"));
}
