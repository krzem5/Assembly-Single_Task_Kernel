#ifndef _VIRTIO_GPU_H_
#define _VIRTIO_GPU_H_ 1
#include <kernel/types.h>
#include <ui/display.h>
#include <virtio/gpu_registers.h>
#include <virtio/virtio.h>



typedef struct _VIRTIO_GPU_DEVICE{
	virtio_device_t* device;
	virtio_queue_t* controlq;
	virtio_queue_t* cursorq;
	u32 scanout_count;
	ui_display_t** displays;
	virtio_gpu_resource_id_t* framebuffer_resources;
} virtio_gpu_device_t;



void virtio_gpu_init(void);



#endif
