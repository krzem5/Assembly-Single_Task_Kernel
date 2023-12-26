#include <kernel/log/log.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <virtio/device.h>
#include <virtio/blk.h>
#include <virtio/blk_registers.h>
#define KERNEL_LOG_NAME "virtio_blk"



static const virtio_device_driver_t _virtio_blk_device_driver={
	"Block Device",
	0x0002,
	(1<<VIRTIO_BLK_F_SIZE_MAX)|(1<<VIRTIO_BLK_F_SEG_MAX)|(1<<VIRTIO_BLK_F_BLK_SIZE)
};



void virtio_blk_init(void){
	LOG("Initializing VirtIO block driver...");
	if (!virtio_register_device_driver(&_virtio_blk_device_driver)){
		ERROR("Unable to register VirtIO block driver");
	}
}
