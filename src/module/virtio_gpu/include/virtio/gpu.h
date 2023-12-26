#ifndef _VIRTIO_GPU_H_
#define _VIRTIO_GPU_H_ 1
#include <kernel/types.h>
#include <virtio/virtio.h>



typedef struct _VIRTIO_GPU_DEVICE{
	virtio_device_t* device;
	virtio_queue_t* controlq;
	virtio_queue_t* cursorq;
	u32 scanout_count;
} virtio_gpu_device_t;



void virtio_gpu_init(void);



#endif
