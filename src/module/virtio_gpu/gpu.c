#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <ui/display.h>
#include <virtio/gpu.h>
#include <virtio/gpu_registers.h>
#include <virtio/gpu_virgl.h>
#include <virtio/registers.h>
#include <virtio/virtio.h>
#define KERNEL_LOG_NAME "virtio_gpu"



static omm_allocator_t* _virtio_gpu_device_allocator=NULL;



static virtio_gpu_resource_id_t _alloc_resource_id(void){
	static KERNEL_ATOMIC virtio_gpu_resource_id_t _next_id=1;
	return _next_id++;
}



static _Bool _display_resize_framebuffer(ui_display_t* display){
	virtio_gpu_device_t* gpu_device=display->ctx;
	if (display->framebuffer){
		// 1. VIRTIO_GPU_CMD_SET_SCANOUT with VIRTIO_GPU_NO_RESOURCE
		// 2. VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING
		// 3. VIRTIO_GPU_CMD_RESOURCE_UNREF
		panic("_display_resize_framebuffer: dealloc framebuffer");
		ui_framebuffer_delete(display->framebuffer);
		display->framebuffer=NULL;
	}
	if (!display->mode){
		return 1;
	}
	display->framebuffer=ui_framebuffer_create(display->mode->width,display->mode->height,UI_FRAMEBUFFER_FORMAT_BGRX);
	if (!display->framebuffer){
		return 0;
	}
	for (u32 x=0;x<display->framebuffer->size;x++){
		display->framebuffer->data[x]=x;
	}
	if (!gpu_device->framebuffer_resources[display->index]){
		gpu_device->framebuffer_resources[display->index]=_alloc_resource_id();
	}
	virtio_gpu_resource_create_2d_t* request_resource_create_2d=amm_alloc(sizeof(virtio_gpu_resource_create_2d_t));
	request_resource_create_2d->header.type=VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
	request_resource_create_2d->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request_resource_create_2d->header.fence_id=0;
	request_resource_create_2d->resource_id=gpu_device->framebuffer_resources[display->index];
	request_resource_create_2d->format=VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM;
	request_resource_create_2d->width=display->framebuffer->width;
	request_resource_create_2d->height=display->framebuffer->height;
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_resource_create_2d),
			sizeof(virtio_gpu_resource_create_2d_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_control_header_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request_resource_create_2d);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		WARN("Unable to create 2D resource");
		amm_dealloc(response);
		goto _cleanup;
	}
	virtio_gpu_resource_attach_backing_t* request_resource_attach_backing=amm_alloc(sizeof(virtio_gpu_resource_attach_backing_t)+sizeof(virtio_gpu_mem_entry_t));
	request_resource_attach_backing->header.type=VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
	request_resource_attach_backing->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request_resource_attach_backing->header.fence_id=0;
	request_resource_attach_backing->resource_id=gpu_device->framebuffer_resources[display->index];
	request_resource_attach_backing->entry_count=1;
	request_resource_attach_backing->entries[0].address=display->framebuffer->address;
	request_resource_attach_backing->entries[0].length=pmm_align_up_address(display->framebuffer->size);
	buffers[0].address=vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_resource_attach_backing);
	buffers[0].length=sizeof(virtio_gpu_resource_attach_backing_t)+sizeof(virtio_gpu_mem_entry_t);
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request_resource_attach_backing);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		WARN("Unable to attach 2D resource backing");
		amm_dealloc(response);
		goto _cleanup;
	}
	virtio_gpu_set_scanout_t* request_set_scanout=amm_alloc(sizeof(virtio_gpu_set_scanout_t));
	request_set_scanout->header.type=VIRTIO_GPU_CMD_SET_SCANOUT;
	request_set_scanout->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request_set_scanout->header.fence_id=0;
	request_set_scanout->rect.x=0;
	request_set_scanout->rect.y=0;
	request_set_scanout->rect.width=display->framebuffer->width;
	request_set_scanout->rect.height=display->framebuffer->height;
	request_set_scanout->scanout_id=display->index;
	request_set_scanout->resource_id=gpu_device->framebuffer_resources[display->index];
	buffers[0].address=vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_set_scanout);
	buffers[0].length=sizeof(virtio_gpu_set_scanout_t);
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request_set_scanout);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		WARN("Unable to set scanout resource");
		amm_dealloc(response);
		goto _cleanup;
	}
	amm_dealloc(response);
	return 1;
_cleanup:
	display->mode=NULL;
	ui_framebuffer_delete(display->framebuffer);
	display->framebuffer=NULL;
	return 0;
}



static void _display_flush_framebuffer(ui_display_t* display){
	virtio_gpu_device_t* gpu_device=display->ctx;
	virtio_gpu_transfer_to_host_2d_t* request_transfer_to_host_2d=amm_alloc(sizeof(virtio_gpu_transfer_to_host_2d_t));
	request_transfer_to_host_2d->header.type=VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D;
	request_transfer_to_host_2d->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request_transfer_to_host_2d->header.fence_id=0;
	request_transfer_to_host_2d->rect.x=0;
	request_transfer_to_host_2d->rect.y=0;
	request_transfer_to_host_2d->rect.width=display->framebuffer->width;
	request_transfer_to_host_2d->rect.height=display->framebuffer->height;
	request_transfer_to_host_2d->offset=0;
	request_transfer_to_host_2d->resource_id=gpu_device->framebuffer_resources[display->index];
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_transfer_to_host_2d),
			sizeof(virtio_gpu_transfer_to_host_2d_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_control_header_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request_transfer_to_host_2d);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		WARN("Unable to transfer framebuffer to host");
		amm_dealloc(response);
		return;
	}
	virtio_gpu_resource_flush_t* request_resource_flush=amm_alloc(sizeof(virtio_gpu_resource_flush_t));
	request_resource_flush->header.type=VIRTIO_GPU_CMD_RESOURCE_FLUSH;
	request_resource_flush->header.flags=0;
	request_resource_flush->rect.x=0;
	request_resource_flush->rect.y=0;
	request_resource_flush->rect.width=display->framebuffer->width;
	request_resource_flush->rect.height=display->framebuffer->height;
	request_resource_flush->resource_id=gpu_device->framebuffer_resources[display->index];
	buffers[0].address=vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_resource_flush);
	buffers[0].length=sizeof(virtio_gpu_resource_flush_t);
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request_resource_flush);
	amm_dealloc(response);
}



static const ui_display_driver_t _virtio_gpu_display_driver={
	"VirtIO GPU",
	_display_resize_framebuffer,
	_display_flush_framebuffer
};



static void _load_capsets(virtio_gpu_device_t* gpu_device){
	virtio_gpu_get_capset_info_t* request_get_capset_info=amm_alloc(sizeof(virtio_gpu_get_capset_info_t));
	request_get_capset_info->header.type=VIRTIO_GPU_CMD_GET_CAPSET_INFO;
	request_get_capset_info->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request_get_capset_info->header.fence_id=0;
	virtio_gpu_resp_capset_info_t* response_capset_info=amm_alloc(sizeof(virtio_gpu_resp_capset_info_t));
	virtio_buffer_t buffers_get_capset_info[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_get_capset_info),
			sizeof(virtio_gpu_get_capset_info_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response_capset_info),
			sizeof(virtio_gpu_resp_capset_info_t)
		}
	};
	virtio_gpu_get_capset_t* request_get_capset=amm_alloc(sizeof(virtio_gpu_get_capset_t));
	request_get_capset->header.type=VIRTIO_GPU_CMD_GET_CAPSET;
	request_get_capset->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request_get_capset->header.fence_id=0;
	virtio_buffer_t buffers_get_capset[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_get_capset),
			sizeof(virtio_gpu_get_capset_t)
		}
	};
	u32 max_capset=virtio_read(gpu_device->device->device_field+VIRTIO_GPU_REG_NUM_CAPSETS,4);
	for (u32 i=0;i<max_capset;i++){
		request_get_capset_info->capset_index=i;
		virtio_queue_transfer(gpu_device->controlq,buffers_get_capset_info,1,1);
		virtio_queue_wait(gpu_device->controlq);
		virtio_queue_pop(gpu_device->controlq,NULL);
		if (response_capset_info->header.type!=VIRTIO_GPU_RESP_OK_CAPSET_INFO){
			WARN("Unable to get capset info from capset #%u",i);
			continue;
		}
		request_get_capset->capset_id=response_capset_info->capset_id;
		request_get_capset->capset_version=response_capset_info->capset_max_version;
		virtio_gpu_resp_capset_t* response_capset=amm_alloc(sizeof(virtio_gpu_resp_capset_t)+response_capset_info->capset_max_size);
		buffers_get_capset[1].address=vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response_capset);
		buffers_get_capset[1].length=sizeof(virtio_gpu_resp_capset_t)+response_capset_info->capset_max_size;
		virtio_queue_transfer(gpu_device->controlq,buffers_get_capset,1,1);
		virtio_queue_wait(gpu_device->controlq);
		virtio_queue_pop(gpu_device->controlq,NULL);
		if (response_capset->header.type==VIRTIO_GPU_RESP_OK_CAPSET){
			if (request_get_capset->capset_id==VIRTIO_GPU_CAPSET_VIRGL2){
				virtio_gpu_virgl_load_opengl_from_capset(1,response_capset->capset_data,response_capset_info->capset_max_size);
			}
		}
		amm_dealloc(response_capset);
	}
	amm_dealloc(request_get_capset);
	amm_dealloc(request_get_capset_info);
	amm_dealloc(response_capset_info);
}



static void _fetch_edid_data(virtio_gpu_device_t* gpu_device){
	virtio_gpu_get_edid_t* request_get_edid=amm_alloc(sizeof(virtio_gpu_get_edid_t));
	request_get_edid->header.type=VIRTIO_GPU_CMD_GET_EDID;
	request_get_edid->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request_get_edid->header.fence_id=0;
	virtio_gpu_resp_edid_t* response=amm_alloc(sizeof(virtio_gpu_resp_edid_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_get_edid),
			sizeof(virtio_gpu_get_edid_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_resp_edid_t)
		}
	};
	for (u32 i=0;i<gpu_device->scanout_count;i++){
		request_get_edid->scanout=i;
		virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
		virtio_queue_wait(gpu_device->controlq);
		virtio_queue_pop(gpu_device->controlq,NULL);
		if (response->header.type!=VIRTIO_GPU_RESP_OK_EDID){
			WARN("No EDID response from display #%u",i);
			continue;
		}
		gpu_device->displays[i]=ui_display_create(&_virtio_gpu_display_driver,gpu_device,i,response->edid,1024);
	}
	amm_dealloc(request_get_edid);
	amm_dealloc(response);
}



static void _fetch_display_info(virtio_gpu_device_t* gpu_device){
	virtio_write(gpu_device->device->device_field+VIRTIO_GPU_REG_EVENTS_CLEAR,4,VIRTIO_GPU_EVENT_DISPLAY);
	virtio_gpu_control_header_t* request_display_info=amm_alloc(sizeof(virtio_gpu_control_header_t));
	request_display_info->type=VIRTIO_GPU_CMD_GET_DISPLAY_INFO;
	request_display_info->flags=VIRTIO_GPU_FLAG_FENCE;
	request_display_info->fence_id=0;
	virtio_gpu_resp_display_info_t* response=amm_alloc(sizeof(virtio_gpu_resp_display_info_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_display_info),
			sizeof(virtio_gpu_control_header_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_resp_display_info_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	if (response->header.type!=VIRTIO_GPU_RESP_OK_DISPLAY_INFO){
		goto _cleanup;
	}
	for (u32 i=0;i<gpu_device->scanout_count;i++){
		if (!gpu_device->displays[i]){
			continue;
		}
		INFO("Detected size of display #%u: %u x %u",i,(response->displays+i)->rect.width,(response->displays+i)->rect.height);
		const ui_display_info_mode_t* mode=ui_display_info_find_mode(gpu_device->displays[i]->display_info,(response->displays+i)->rect.width,(response->displays+i)->rect.height,0);
		if (!mode){
			mode=gpu_device->displays[i]->display_info->modes;
		}
		if (mode){
			ui_display_set_mode(gpu_device->displays[i],mode);
		}
		else{
			WARN("Unable to find matching display mode");
		}
	}
_cleanup:
	amm_dealloc(request_display_info);
	amm_dealloc(response);
	return;
}



static _Bool _virtio_driver_init(virtio_device_t* device,u64 features){
	if (features!=((1<<VIRTIO_GPU_F_VIRGL)|(1<<VIRTIO_GPU_F_EDID))){
		return 0;
	}
	virtio_queue_t* controlq=virtio_init_queue(device,0);
	if (!controlq){
		return 0;
	}
	virtio_queue_t* cursorq=virtio_init_queue(device,1);
	if (!cursorq){
		return 0;
	}
	virtio_gpu_device_t* gpu_device=omm_alloc(_virtio_gpu_device_allocator);
	gpu_device->device=device;
	gpu_device->controlq=controlq;
	gpu_device->cursorq=cursorq;
	gpu_device->scanout_count=virtio_read(device->device_field+VIRTIO_GPU_REG_NUM_SCANOUTS,4);
	gpu_device->displays=amm_alloc(gpu_device->scanout_count*sizeof(ui_display_t*));
	gpu_device->framebuffer_resources=amm_alloc(gpu_device->scanout_count*sizeof(virtio_gpu_resource_id_t));
	for (u32 i=0;i<gpu_device->scanout_count;i++){
		gpu_device->displays[i]=NULL;
		gpu_device->framebuffer_resources[i]=VIRTIO_GPU_NO_RESOURCE;
	}
	virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE|VIRTIO_DEVICE_STATUS_FLAG_DRIVER|VIRTIO_DEVICE_STATUS_FLAG_DRIVER_OK|VIRTIO_DEVICE_STATUS_FLAG_FEATURES_OK);
	_load_capsets(gpu_device);
	_fetch_edid_data(gpu_device);
	_fetch_display_info(gpu_device);
	return 1;
}



static const virtio_device_driver_t _virtio_gpu_device_driver={
	"GPU Device",
	0x0010,
	(1<<VIRTIO_GPU_F_VIRGL)|(1<<VIRTIO_GPU_F_EDID),
	_virtio_driver_init
};



void virtio_gpu_init(void){
	LOG("Initializing VirtIO GPU driver...");
	_virtio_gpu_device_allocator=omm_init("virtio_gpu_device",sizeof(virtio_gpu_device_t),8,1,pmm_alloc_counter("omm_virtio_gpu_device"));
	spinlock_init(&(_virtio_gpu_device_allocator->lock));
	if (!virtio_register_device_driver(&_virtio_gpu_device_driver)){
		ERROR("Unable to register VirtIO GPU driver");
	}
}
