#ifndef _VIRTIO_BLK_H_
#define _VIRTIO_BLK_H_ 1
#include <kernel/types.h>
#include <virtio/device.h>



typedef struct _VIRTIO_BLK_DEVICE{
	virtio_device_t device;
} virtio_blk_device_t;



void virtio_blk_init(void);



#endif
