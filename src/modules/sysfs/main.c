#include <sysfs/fs.h>
#include <sysfs/handle.h>
#include <sysfs/memory.h>
#include <kernel/module/module.h>



static _Bool _init(module_t* module){
	sysfs_create_fs();
	sysfs_handle_init();
	sysfs_memory_init();
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"devfs",
	_init,
	_deinit
);
