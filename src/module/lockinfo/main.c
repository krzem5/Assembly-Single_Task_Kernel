#include <kernel/module/module.h>
#include <lockinfo/syscall.h>



static _Bool _init(module_t* module){
	return lockinfo_syscall_init();
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);
