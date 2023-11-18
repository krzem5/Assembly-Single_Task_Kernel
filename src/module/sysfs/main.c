#include <kernel/module/module.h>
#include <sysfs/fs.h>
#include <sysfs/handle.h>
#include <sysfs/load_balancer.h>
#include <sysfs/memory.h>
#include <sysfs/module.h>
#include <sysfs/scheduler.h>



static _Bool _init(module_t* module){
	sysfs_create_fs();
	sysfs_handle_init();
	sysfs_load_balancer_init();
	sysfs_memory_init();
	sysfs_module_init();
	sysfs_scheduler_init();
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
