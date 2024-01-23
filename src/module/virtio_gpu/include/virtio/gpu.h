#ifndef _VIRTIO_GPU_H_
#define _VIRTIO_GPU_H_ 1
#include <kernel/resource/resource.h>
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
	resource_manager_t* resource_manager;
} virtio_gpu_device_t;



void virtio_gpu_init(void);



virtio_gpu_resp_display_info_t* virtio_gpu_command_get_display_info(virtio_gpu_device_t* gpu_device);



virtio_gpu_resource_id_t virtio_gpu_command_resource_create_2d(virtio_gpu_device_t* gpu_device,u32 format,u32 width,u32 height,virtio_gpu_resource_id_t resource_id);



void virtio_gpu_command_resource_unref(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id);



void virtio_gpu_command_set_scanout(virtio_gpu_device_t* gpu_device,ui_display_t* display,virtio_gpu_resource_id_t resource_id);



void virtio_gpu_command_resource_flush(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,u32 width,u32 height);



void virtio_gpu_command_transfer_to_host_2d(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,u32 width,u32 height);



void virtio_gpu_command_resource_attach_backing(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,u64 address,u32 length);



void virtio_gpu_command_resource_detach_backing(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id);



virtio_gpu_resp_capset_info_t* virtio_gpu_command_get_capset_info(virtio_gpu_device_t* gpu_device,u32 index);



virtio_gpu_resp_capset_t* virtio_gpu_command_get_capset(virtio_gpu_device_t* gpu_device,u32 capset_id,u32 capset_version,u32 capset_size);



virtio_gpu_resp_edid_t* virtio_gpu_command_get_edid(virtio_gpu_device_t* gpu_device,u32 scanout);



void virtio_gpu_command_ctx_create(virtio_gpu_device_t* gpu_device,u32 ctx,u32 type);



void virtio_gpu_command_ctx_destroy(virtio_gpu_device_t* gpu_device,u32 ctx);



void virtio_gpu_command_ctx_attach_resource(virtio_gpu_device_t* gpu_device,u32 ctx,virtio_gpu_resource_id_t resource_id);



void virtio_gpu_command_ctx_detach_resource(virtio_gpu_device_t* gpu_device,u32 ctx,virtio_gpu_resource_id_t resource_id);



virtio_gpu_resource_id_t virtio_gpu_command_resource_create_3d(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,u32 target,u32 format,u32 bind,u32 width,u32 height,u32 depth,u32 array_size,u32 last_level,u32 nr_samples);



void virtio_gpu_command_transfer_to_host_3d(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,const virtio_gpu_box_t* box,u32 level,u32 stride,u32 layer_stride);



void virtio_gpu_command_transfer_from_host_3d(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,const virtio_gpu_box_t* box,u32 level,u32 stride,u32 layer_stride);



void virtio_gpu_command_submit_3d(virtio_gpu_device_t* gpu_device,u32 ctx,u64 buffer,u32 size);



void virtio_gpu_command_update_cursor(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,const virtio_gpu_cursor_pos_t* pos,u32 hot_x,u32 hot_y);



void virtio_gpu_command_move_cursor(virtio_gpu_device_t* gpu_device,const virtio_gpu_cursor_pos_t* pos);



#endif
