#include <kernel/drive/drive.h>
#include <kernel/log/log.h>
#include <kernel/memory/smm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <virtio/blk.h>
#include <virtio/blk_registers.h>
#include <virtio/device.h>
#include <virtio/registers.h>
#define KERNEL_LOG_NAME "virtio_blk"



#define VIRTIO_DEFAULT_BLOCK_SIZE 512



static u64 _virtio_blk_read_write(drive_t* drive,u64 offset,void* buffer,u64 count){
	virtio_device_t* device=drive->extra_data;
	(void)device;
	return 0;
}



static const drive_type_t _virtio_blk_drive_type_config={
	"virtio",
	_virtio_blk_read_write
};



static _Bool _virtio_driver_init(virtio_device_t* device,u64 features){
	if (!(features&(1ull<<VIRTIO_F_VERSION_1))){
		ERROR("Wrong device version");
		return 0;
	}
	virtio_init_queue(device,0);
	virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE|VIRTIO_DEVICE_STATUS_FLAG_DRIVER|VIRTIO_DEVICE_STATUS_FLAG_DRIVER_OK|VIRTIO_DEVICE_STATUS_FLAG_FEATURES_OK);
	// panic("A");
	drive_config_t config={
		&_virtio_blk_drive_type_config,
		device->index,
		0,
		smm_alloc("VirtIO",0),
		smm_alloc("VirtIO",0),
		virtio_read(device->device_field+VIRTIO_BLK_REG_CAPACITY,8),
		((features&(1<<VIRTIO_BLK_F_BLK_SIZE))?virtio_read(device->device_field+VIRTIO_BLK_REG_BLK_SIZE,4):VIRTIO_DEFAULT_BLOCK_SIZE),
		/*alloc device*/NULL
	};
	drive_create(&config);
	return 1;
}



static const virtio_device_driver_t _virtio_blk_device_driver={
	"Block Device",
	0x0002,
	(1<<VIRTIO_BLK_F_SIZE_MAX)|(1<<VIRTIO_BLK_F_SEG_MAX)|(1<<VIRTIO_BLK_F_BLK_SIZE)|(1ull<<VIRTIO_F_VERSION_1),
	_virtio_driver_init
};



void virtio_blk_init(void){
	LOG("Initializing VirtIO block driver...");
	if (!virtio_register_device_driver(&_virtio_blk_device_driver)){
		ERROR("Unable to register VirtIO block driver");
	}
}
