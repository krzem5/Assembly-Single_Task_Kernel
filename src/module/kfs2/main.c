#include <kernel/module/module.h>
#include <kfs2/fs.h>
#include <kfs2/io.h>



static _Bool _init(module_t* module){
	kfs2_io_init();
	kfs2_register_fs();
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
