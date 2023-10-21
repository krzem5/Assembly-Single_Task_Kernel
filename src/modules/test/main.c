#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "test_module"



void test_func(void){
	WARN("Called test_func");
}



static _Bool _init(module_t* module){
	test_func();
	ERROR("Module loaded! @ %p",module->ex_region.base);
	return 1;
}



static void _deinit(module_t* module){
	WARN("Module unloaded!");
}



MODULE_DECLARE(
	"test",
	_init,
	_deinit
);
