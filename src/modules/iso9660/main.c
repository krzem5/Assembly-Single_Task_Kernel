#include <iso9660/fs.h>
#include <iso9660/partition.h>
#include <kernel/module/module.h>



static _Bool _init(module_t* module){
	iso9660_register_fs();
	iso9660_register_partition();
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"iso9660",
	_init,
	_deinit
);
