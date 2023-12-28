#include <kernel/module/module.h>
#include <ui/display.h>
#include <ui/display_info.h>
#include <ui/display_syscall.h>
#include <ui/framebuffer.h>



static _Bool _init(module_t* module){
	ui_display_info_init();
	ui_display_init();
	ui_display_syscall_init();
	ui_framebuffer_init();
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
