#include <dynamicfs/dynamicfs.h>
#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#define KERNEL_LOG_NAME "procfs_fs"



filesystem_t* KERNEL_INIT_WRITE procfs;



static const filesystem_descriptor_config_t _procfs_filesystem_descriptor_config={
	"procfs",
	NULL,
	NULL
};



MODULE_INIT(){
	LOG("Creating procfs filesystem...");
	procfs=dynamicfs_init("/proc",&_procfs_filesystem_descriptor_config);
}
