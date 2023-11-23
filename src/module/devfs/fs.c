#include <dynamicfs/dynamicfs.h>
#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#define KERNEL_LOG_NAME "devfs_fs"



filesystem_t* devfs;



static const filesystem_descriptor_config_t _devfs_filesystem_descriptor_config={
	"devfs",
	NULL,
	NULL
};



void devfs_create_fs(void){
	LOG("Creating devfs filesystem...");
	devfs=dynamicfs_init("/dev",&_devfs_filesystem_descriptor_config);
}
