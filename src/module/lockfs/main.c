#include <kernel/module/module.h>
#include <lockfs/fs.h>



static _Bool _init(module_t* module){
	return lockfs_create_fs();
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	_init,
	_deinit,
	0
);
