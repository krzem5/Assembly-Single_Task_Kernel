#include <kernel/module/module.h>
#include <procfs/fs.h>
#include <procfs/process.h>
#include <procfs/thread.h>



static _Bool _init(module_t* module){
	procfs_create_fs();
	procfs_process_init();
	procfs_thread_init();
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"procfs",
	_init,
	_deinit
);
