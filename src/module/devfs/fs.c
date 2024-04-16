#include <dynamicfs/dynamicfs.h>
#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "devfs_fs"



filesystem_t* KERNEL_INIT_WRITE devfs;



static const filesystem_descriptor_config_t _devfs_filesystem_descriptor_config={
	"devfs",
	NULL,
	NULL,
	NULL,
	NULL
};



MODULE_INIT(){
	LOG("Creating devfs filesystem...");
	devfs=dynamicfs_init("/dev",&_devfs_filesystem_descriptor_config);
}
