#include <kernel/fs/fs.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "os_loader"



static _Bool _init(module_t* module){
	LOG("Searching for boot filesystem...");
	filesystem_t* boot_fs=NULL;
	HANDLE_FOREACH(HANDLE_TYPE_FS){
		handle_acquire(handle);
		filesystem_t* fs=handle->object;
		for (u8 i=0;i<16;i++){
			if (fs->uuid[i]!=kernel_data.boot_fs_uuid[i]){
				goto _check_next_fs;
			}
		}
		boot_fs=fs;
		break;
_check_next_fs:
		handle_release(handle);
	}
	if (!boot_fs){
		panic("Unable to finde boot filesystem");
	}
	vfs_mount(boot_fs,NULL);
	return 1;
}



static void _deinit(module_t* module){
	return;
}



MODULE_DECLARE(
	"os_loader",
	_init,
	_deinit
);
