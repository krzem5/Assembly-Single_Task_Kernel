#include <ahci/device.h>
#include <kernel/module/module.h>



static _Bool _init(module_t* module){
	ahci_locate_devices();
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"ahci",
	_init,
	_deinit
);
