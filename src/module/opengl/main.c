#include <kernel/module/module.h>
#include <opengl/opengl.h>
#include <opengl/syscall.h>



static _Bool _init(module_t* module){
	opengl_init();
	opengl_syscall_init();
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
