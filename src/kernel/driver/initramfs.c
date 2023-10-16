#include <kernel/drive/drive.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "initramfs"



static u64 _initramfs_read_write(void* extra_data,u64 offset,void* buffer,u64 count){
	if (offset&DRIVE_OFFSET_FLAG_WRITE){
		return 0;
	}
	WARN("Read %p -> %v",offset,count);
	return 0;
}



void initramfs_load(void){
	ERROR("Loading initramfs...");
	INFO("Address: %p, Size: %v",kernel_data.initramfs_address,kernel_data.initramfs_size);
	drive_config_t config={
		.type=DRIVE_TYPE_INITRAMFS,
		.name="initramfs",
		.serial_number="initramfs",
		.model_number="initramfs",
		.block_count=pmm_align_up_address(kernel_data.initramfs_size)>>PAGE_SIZE_SHIFT,
		.block_size=PAGE_SIZE,
		.read_write=_initramfs_read_write
	};
	drive_create(&config);
}
