#include <kernel/module/module.h>
#include <ui/display.h>



static _Bool _init(module_t* module){
	ui_display_init();
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
