#include <dynamicfs/dynamicfs.h>
#include <kernel/fs/fs.h>
#include <kernel/log/log.h>
#define KERNEL_LOG_NAME "procfs_fs"



filesystem_t* procfs;



static filesystem_descriptor_t _procfs_filesystem_descriptor={
	"procfs",
	NULL,
	NULL
};



void procfs_create_fs(void){
	LOG("Creating procfs filesystem...");
	procfs=dynamicfs_init("/proc",&_procfs_filesystem_descriptor);
}
