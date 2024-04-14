#ifndef _VIRTIO_BLK_H_
#define _VIRTIO_BLK_H_ 1
#include <kernel/types.h>
#include <virtio/virtio.h>



typedef struct _VIRTIO_BLK_DEVICE{
	virtio_device_t* device;
	virtio_queue_t* queue;
} virtio_blk_device_t;



#endif
