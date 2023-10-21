#include <kernel/log/log.h>
#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "test_module"



void test_func(void){
	WARN("Called test_func");
}



static _Bool _init(void){
	ERROR("Module loaded!");
	test_func();
	return 1;
}



static void _deinit(void){
	WARN("Module unloaded!");
}



MODULE_DECLARE(
	"test",
	_init,
	_deinit
);
