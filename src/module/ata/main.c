#include <ata/device.h>
#include <kernel/module/module.h>



static _Bool _init(module_t* module){
	return ata_locate_devices();
}



static void _deinit(module_t* module){
	ata_deinit();
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);
