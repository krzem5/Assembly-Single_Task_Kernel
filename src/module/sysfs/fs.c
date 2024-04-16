#include <dynamicfs/dynamicfs.h>
#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "sysfs_fs"



filesystem_t* KERNEL_INIT_WRITE sysfs;



static const filesystem_descriptor_config_t _sysfs_filesystem_descriptor_config={
	"sysfs",
	NULL,
	NULL,
	NULL,
	NULL
};



MODULE_INIT(){
	LOG("Creating sysfs filesystem...");
	sysfs=dynamicfs_init("/sys",&_sysfs_filesystem_descriptor_config);
}
