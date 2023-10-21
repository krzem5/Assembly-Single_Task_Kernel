#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "test_module"



static _Bool _init(module_t* module){
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
