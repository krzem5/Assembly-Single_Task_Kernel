#include <kernel/drive/drive.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "initramfs_drive"



static u64 _initramfs_read_write(drive_t* drive,u64 offset,void* buffer,u64 count){
	if (offset&DRIVE_OFFSET_FLAG_WRITE){
		return 0;
	}
	if (offset>=kernel_data.initramfs_size){
		return 0;
	}
	if (offset+count>kernel_data.initramfs_size){
		count=kernel_data.initramfs_size-offset;
	}
	if (!count){
		return 0;
	}
	memcpy(buffer,(void*)(kernel_data.initramfs_address+offset+VMM_HIGHER_HALF_ADDRESS_OFFSET),count);
	return count;
}



static drive_type_t _initramfs_drive_type={
	"initramfs",
	_initramfs_read_write
};



void initramfs_drive_init(void){
	INFO("Creating virtual initramfs drive...");
	INFO("Address: %p, Size: %v",kernel_data.initramfs_address,kernel_data.initramfs_size);
	drive_config_t config={
		&_initramfs_drive_type,
		0,
		0,
		smm_alloc("initramfs",0),
		smm_alloc("initramfs",0),
		pmm_align_up_address(kernel_data.initramfs_size),
		1,
		NULL
	};
	drive_create(&config);
}
