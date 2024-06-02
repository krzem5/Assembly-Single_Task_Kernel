#include <kernel/drive/drive.h>
#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#define KERNEL_LOG_NAME "initramfs_drive"



static void* KERNEL_INIT_WRITE _initramfs_drive_address=NULL;
static u64 KERNEL_INIT_WRITE _initramfs_drive_size=0;
static drive_t* _initramfs_drive=NULL;



static u64 _initramfs_read_write(drive_t* drive,u64 offset,u64 buffer,u64 count){
	if (offset&DRIVE_OFFSET_FLAG_WRITE){
		return 0;
	}
	mem_copy((void*)(buffer+VMM_HIGHER_HALF_ADDRESS_OFFSET),_initramfs_drive_address+offset,count);
	return count;
}



static const drive_type_t _initramfs_drive_type_config={
	"initramfs",
	DRIVE_TYPE_FLAG_NO_CACHE,
	_initramfs_read_write
};



void KERNEL_EARLY_EXEC initramfs_drive_init(void){
	INFO("Creating virtual initramfs drive...");
	_initramfs_drive_address=(void*)(kernel_data.initramfs_address+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	_initramfs_drive_size=kernel_data.initramfs_size;
	INFO("Address: %p, Size: %v",_initramfs_drive_address,_initramfs_drive_size);
	drive_config_t config={
		&_initramfs_drive_type_config,
		0,
		0,
		smm_alloc("initramfs",0),
		smm_alloc("initramfs",0),
		pmm_align_up_address(_initramfs_drive_size),
		1,
		NULL
	};
	_initramfs_drive=drive_create(&config);
}



void initramfs_drive_deinit(void){
	INFO("Unloading initramfs drive...");
	if (!_initramfs_drive){
		return;
	}
	handle_release(&(_initramfs_drive->handle));
	_initramfs_drive=NULL;
	mem_fill(_initramfs_drive_address,_initramfs_drive_size,0);
}
