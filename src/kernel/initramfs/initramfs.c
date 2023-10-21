#include <kernel/drive/drive.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "initramfs"



static u64 _initramfs_read_write(void* extra_data,u64 offset,void* buffer,u64 count){
	if (offset&DRIVE_OFFSET_FLAG_WRITE){
		return 0;
	}
	offset<<=PAGE_SIZE_SHIFT;
	count<<=PAGE_SIZE_SHIFT;
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
	return count>>PAGE_SIZE_SHIFT;
}



static drive_type_t _initramfs_drive_type={
	"INITRAMFS",
	_initramfs_read_write
};



static _Bool _initramfs_load_partitions(drive_t* drive){
	if (!streq(drive->type->name,"INITRAMFS")){
		return 0;
	}
	partition_t* partition=partition_create(drive,"INITRAMFS",0,drive->block_count);
	if (!partition->fs){
		return 0;
	}
	vfs_mount(partition->fs,NULL);
	return 1;
}



static partition_descriptor_t _initramfs_partition_descriptor={
	"INITRAMFS",
	_initramfs_load_partitions
};



void initramfs_load(void){
	LOG("Loading initramfs...");
	INFO("Address: %p, Size: %v",kernel_data.initramfs_address,kernel_data.initramfs_size);
	INFO("Registering partition descriptor...");
	partition_register_descriptor(&_initramfs_partition_descriptor);
	INFO("Creating virtual drive...");
	drive_config_t config={
		.type=&_initramfs_drive_type,
		.name="initramfs",
		.serial_number="initramfs",
		.model_number="initramfs",
		.block_count=pmm_align_up_address(kernel_data.initramfs_size)>>PAGE_SIZE_SHIFT,
		.block_size=PAGE_SIZE
	};
	drive_create(&config);
}
