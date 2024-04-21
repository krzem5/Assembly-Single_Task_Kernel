#include <kernel/drive/drive.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <virtio/blk.h>
#include <virtio/blk_registers.h>
#include <virtio/registers.h>
#include <virtio/virtio.h>
#define KERNEL_LOG_NAME "virtio_blk"



#define VIRTIO_DEFAULT_BLOCK_SIZE 512



static omm_allocator_t* KERNEL_INIT_WRITE _virtio_blk_device_allocator=NULL;



static u64 _virtio_blk_read_write(drive_t* drive,u64 offset,u64 buffer,u64 count){
	virtio_blk_device_t* blk_device=drive->extra_data;
	_Bool is_write=!!(offset&DRIVE_OFFSET_FLAG_WRITE);
	offset&=DRIVE_OFFSET_MASK;
	virtio_blk_request_header_t* header=amm_alloc(sizeof(virtio_blk_request_header_t));
	header->type=(is_write?VIRTIO_BLK_T_OUT:VIRTIO_BLK_T_IN);
	header->_zero=0;
	header->sector=offset;
	u8 status=0;
	virtio_buffer_t buffers[3]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)(&header)),
			sizeof(virtio_blk_request_header_t)
		},
		{
			buffer,
			count<<drive->block_size_shift
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)(&status)),
			sizeof(u8)
		}
	};
	virtio_queue_transfer(blk_device->queue,buffers,1+is_write,2-is_write);
	virtio_queue_wait(blk_device->queue);
	virtio_queue_pop(blk_device->queue,NULL);
	amm_dealloc(header);
	return (status==VIRTIO_BLK_S_OK?count:0);
}



static const drive_type_t _virtio_blk_drive_type_config={
	"virtio",
	0,
	_virtio_blk_read_write
};



static _Bool _virtio_driver_init(virtio_device_t* device,u64 features){
	if (!(features&(1ull<<VIRTIO_F_VERSION_1))){
		ERROR("Wrong device version");
		return 0;
	}
	virtio_queue_t* queue=virtio_init_queue(device,0);
	if (!queue){
		return 0;
	}
	virtio_blk_device_t* blk_device=omm_alloc(_virtio_blk_device_allocator);
	blk_device->device=device;
	blk_device->queue=queue;
	virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE|VIRTIO_DEVICE_STATUS_FLAG_DRIVER|VIRTIO_DEVICE_STATUS_FLAG_DRIVER_OK|VIRTIO_DEVICE_STATUS_FLAG_FEATURES_OK);
	drive_config_t config={
		&_virtio_blk_drive_type_config,
		device->index,
		0,
		smm_alloc("VirtIO",0),
		smm_alloc("VirtIO",0),
		virtio_read(device->device_field+VIRTIO_BLK_REG_CAPACITY,8),
		((features&(1<<VIRTIO_BLK_F_BLK_SIZE))?virtio_read(device->device_field+VIRTIO_BLK_REG_BLK_SIZE,4):VIRTIO_DEFAULT_BLOCK_SIZE),
		blk_device
	};
	drive_create(&config);
	return 1;
}



static const virtio_device_driver_t _virtio_blk_device_driver={
	"Block Device",
	0x0002,
	0,
	(1<<VIRTIO_BLK_F_SIZE_MAX)|(1<<VIRTIO_BLK_F_SEG_MAX)|(1<<VIRTIO_BLK_F_BLK_SIZE)|(1ull<<VIRTIO_F_VERSION_1),
	_virtio_driver_init
};



MODULE_INIT(){
	LOG("Initializing VirtIO block driver...");
	_virtio_blk_device_allocator=omm_init("virtio_blk_device",sizeof(virtio_blk_device_t),8,1);
	spinlock_init(&(_virtio_blk_device_allocator->lock));
}



MODULE_POSTINIT(){
	if (!virtio_register_device_driver(&_virtio_blk_device_driver)){
		ERROR("Unable to register VirtIO block driver");
	}
}
