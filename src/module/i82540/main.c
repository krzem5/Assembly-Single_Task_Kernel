#include <i82540/device.h>
#include <kernel/module/module.h>



static _Bool _init(module_t* module){
	i82540_locate_devices();
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);
