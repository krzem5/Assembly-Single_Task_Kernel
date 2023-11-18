#include <kernel/module/module.h>



static _Bool _init(module_t* module){
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"dynamicfs",
	_init,
	_deinit,
	0
);
