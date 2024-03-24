#include <kernel/initramfs/drive.h>
#include <kernel/initramfs/fs.h>
#include <kernel/initramfs/partition.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "initramfs"



static _Bool _initramfs_is_loaded=0;



KERNEL_INIT(){
	LOG("Loading initramfs...");
	initramfs_fs_init();
	initramfs_partition_init();
	initramfs_drive_init();
	_initramfs_is_loaded=1;
}



KERNEL_PUBLIC void initramfs_unload(void){
	LOG("Unloading initramfs...");
	if (!_initramfs_is_loaded){
		return;
	}
	_initramfs_is_loaded=0;
	initramfs_drive_deinit();
	initramfs_partition_deinit();
	initramfs_fs_deinit();
}
