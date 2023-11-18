#include <kernel/module/module.h>
#include <usb_msc/driver.h>



static _Bool _init(module_t* module){
	usb_msc_driver_install();
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"usb_msc",
	_init,
	_deinit,
	0
);
