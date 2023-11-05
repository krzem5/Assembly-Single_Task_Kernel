#include <devfs/fs.h>
#include <devfs/pci.h>
#include <kernel/module/module.h>



static _Bool _init(module_t* module){
	devfs_create_fs();
	devfs_pci_init();
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
