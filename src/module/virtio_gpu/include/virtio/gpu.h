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



virtio_gpu_resp_display_info_t* virtio_gpu_command_get_display_info(virtio_gpu_device_t* gpu_device);



virtio_gpu_resource_id_t virtio_gpu_command_resource_create_2d(virtio_gpu_device_t* gpu_device,u32 format,u32 width,u32 height,virtio_gpu_resource_id_t resource_id);



void virtio_gpu_command_set_scanout(virtio_gpu_device_t* gpu_device,ui_display_t* display);



void virtio_gpu_command_resource_flush(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,u32 width,u32 height);



void virtio_gpu_command_transfer_to_host_2d(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,u32 width,u32 height);



void virtio_gpu_command_resource_attach_backing(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,u64 address,u32 length);



virtio_gpu_resp_capset_info_t* virtio_gpu_command_get_capset_info(virtio_gpu_device_t* gpu_device,u32 index);



virtio_gpu_resp_capset_t* virtio_gpu_command_get_capset(virtio_gpu_device_t* gpu_device,u32 capset_id,u32 capset_version,u32 capset_size);



virtio_gpu_resp_edid_t* virtio_gpu_command_get_edid(virtio_gpu_device_t* gpu_device,u32 scanout);



#endif
