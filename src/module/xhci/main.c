#include <kernel/module/module.h>
#include <xhci/device.h>



static _Bool _init(module_t* module){
	xhci_locate_devices();
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"xhci",
	_init,
	_deinit,
	0
);
