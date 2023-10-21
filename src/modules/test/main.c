#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "test_module"



void test_func(void){
	WARN("Called test_func");
}



static _Bool _init(void){
	ERROR("Module loaded! ~ %s",kernel_lookup_symbol(0x12345678,NULL));
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
