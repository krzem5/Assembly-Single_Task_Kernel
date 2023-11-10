#include <devfs/bios.h>
#include <devfs/drive.h>
#include <devfs/fs.h>
#include <devfs/numa.h>
#include <devfs/partition.h>
#include <devfs/pci.h>
#include <devfs/virtual_file.h>
#include <kernel/module/module.h>



static _Bool _init(module_t* module){
	devfs_create_fs();
	devfs_bios_init();
	devfs_drive_init();
	devfs_numa_init();
	devfs_partition_init();
	devfs_pci_init();
	devfs_virtual_file_init();
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
