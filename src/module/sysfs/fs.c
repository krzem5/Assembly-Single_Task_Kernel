#include <dynamicfs/dynamicfs.h>
#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#define KERNEL_LOG_NAME "sysfs_fs"



filesystem_t* sysfs;



static const filesystem_descriptor_config_t _sysfs_filesystem_descriptor_config={
	"sysfs",
	NULL,
	NULL
};



void sysfs_create_fs(void){
	LOG("Creating sysfs filesystem...");
	sysfs=dynamicfs_init("/sys",&_sysfs_filesystem_descriptor_config);
}
