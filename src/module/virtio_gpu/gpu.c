#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <ui/display.h>
#include <virgl/virgl.h>
#include <virtio/gpu.h>
#include <virtio/gpu_registers.h>
#include <virtio/registers.h>
#include <virtio/virtio.h>
#define KERNEL_LOG_NAME "virtio_gpu"



#define CONTEXT_NAME "virtio_gpu_ctx"



static omm_allocator_t* _virtio_gpu_device_allocator=NULL;



static virtio_gpu_resource_id_t _alloc_resource_id(void){
	static KERNEL_ATOMIC virtio_gpu_resource_id_t _next_id=1;
	return _next_id++;
}



static _Bool _display_resize_framebuffer(ui_display_t* display){
	LOG("Resizing display #%u framebuffer...",display->index);
	virtio_gpu_device_t* gpu_device=display->ctx;
	if (display->framebuffer){
		INFO("Deleting old framebuffer...");
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
	INFO("Allocating new %u x %u framebuffer...",display->mode->width,display->mode->height);
	display->framebuffer=ui_framebuffer_create(display->mode->width,display->mode->height,UI_FRAMEBUFFER_FORMAT_BGRX);
	if (!display->framebuffer){
		return 0;
	}
	gpu_device->framebuffer_resources[display->index]=virtio_gpu_command_resource_create_2d(gpu_device,VIRTIO_GPU_FORMAT_B8G8R8X8_UNORM,display->framebuffer->width,display->framebuffer->height,gpu_device->framebuffer_resources[display->index]);
	virtio_gpu_command_resource_attach_backing(gpu_device,gpu_device->framebuffer_resources[display->index],display->framebuffer->address,display->framebuffer->size);
	virtio_gpu_command_set_scanout(gpu_device,display);
	return 1;
}



static void _display_flush_framebuffer(ui_display_t* display){
	virtio_gpu_device_t* gpu_device=display->ctx;
	virtio_gpu_command_transfer_to_host_2d(gpu_device,gpu_device->framebuffer_resources[display->index],display->framebuffer->width,display->framebuffer->height);
	virtio_gpu_command_resource_flush(gpu_device,gpu_device->framebuffer_resources[display->index],display->framebuffer->width,display->framebuffer->height);
}



static const ui_display_driver_t _virtio_gpu_display_driver={
	"VirtIO GPU",
	_display_resize_framebuffer,
	_display_flush_framebuffer
};



static void _load_capsets(virtio_gpu_device_t* gpu_device){
	u32 max_capset=virtio_read(gpu_device->device->device_field+VIRTIO_GPU_REG_NUM_CAPSETS,4);
	for (u32 i=0;i<max_capset;i++){
		virtio_gpu_resp_capset_info_t* response_capset_info=virtio_gpu_command_get_capset_info(gpu_device,i);
		if (!response_capset_info){
			WARN("Unable to get capset info from capset #%u",i);
			continue;
		}
		virtio_gpu_resp_capset_t* response_capset=virtio_gpu_command_get_capset(gpu_device,response_capset_info->capset_id,response_capset_info->capset_max_version,response_capset_info->capset_max_size);
		if (response_capset){
			if (response_capset_info->capset_id==VIRTIO_GPU_CAPSET_VIRGL2){
				virgl_load_from_virtio_gpu_capset(gpu_device,1,response_capset->capset_data,response_capset_info->capset_max_size);
			}
			amm_dealloc(response_capset);
		}
		amm_dealloc(response_capset_info);
	}
}



static void _fetch_edid_data(virtio_gpu_device_t* gpu_device){
	for (u32 i=0;i<gpu_device->scanout_count;i++){
		virtio_gpu_resp_edid_t* edid=virtio_gpu_command_get_edid(gpu_device,i);
		if (!edid){
			WARN("No EDID response from display #%u",i);
			continue;
		}
		gpu_device->displays[i]=ui_display_create(&_virtio_gpu_display_driver,gpu_device,i,edid->edid,1024);
		amm_dealloc(edid);
	}
}



static void _fetch_display_info(virtio_gpu_device_t* gpu_device){
	virtio_write(gpu_device->device->device_field+VIRTIO_GPU_REG_EVENTS_CLEAR,4,VIRTIO_GPU_EVENT_DISPLAY);
	virtio_gpu_resp_display_info_t* display_info=virtio_gpu_command_get_display_info(gpu_device);
	if (!display_info){
		WARN("Unable to get display info");
		return;
	}
	for (u32 i=0;i<gpu_device->scanout_count;i++){
		if (!gpu_device->displays[i]){
			continue;
		}
		INFO("Detected size of display #%u: %u x %u",i,(display_info->displays+i)->rect.width,(display_info->displays+i)->rect.height);
		const ui_display_info_mode_t* mode=ui_display_info_find_mode(gpu_device->displays[i]->display_info,(display_info->displays+i)->rect.width,(display_info->displays+i)->rect.height,0);
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
	amm_dealloc(display_info);
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



virtio_gpu_resp_display_info_t* virtio_gpu_command_get_display_info(virtio_gpu_device_t* gpu_device){
	virtio_gpu_control_header_t* request=amm_alloc(sizeof(virtio_gpu_control_header_t));
	request->type=VIRTIO_GPU_CMD_GET_DISPLAY_INFO;
	request->flags=VIRTIO_GPU_FLAG_FENCE;
	request->fence_id=0;
	virtio_gpu_resp_display_info_t* response=amm_alloc(sizeof(virtio_gpu_resp_display_info_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
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
	amm_dealloc(request);
	if (response->header.type==VIRTIO_GPU_RESP_OK_DISPLAY_INFO){
		return response;
	}
	ERROR("virtio_gpu_command_get_display_info failed");
	amm_dealloc(response);
	return NULL;
}



virtio_gpu_resource_id_t virtio_gpu_command_resource_create_2d(virtio_gpu_device_t* gpu_device,u32 format,u32 width,u32 height,virtio_gpu_resource_id_t resource_id){
	if (!resource_id){
		resource_id=_alloc_resource_id();
	}
	virtio_gpu_resource_create_2d_t* request=amm_alloc(sizeof(virtio_gpu_resource_create_2d_t));
	request->header.type=VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
	request->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request->header.fence_id=0;
	request->resource_id=resource_id;
	request->format=format;
	request->width=width;
	request->height=height;
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
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
	amm_dealloc(request);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		ERROR("virtio_gpu_create_2d_resource failed");
	}
	amm_dealloc(response);
	return resource_id;
}


void virtio_gpu_command_resource_unref(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id){
	virtio_gpu_resource_unref_t* request=amm_alloc(sizeof(virtio_gpu_resource_unref_t));
	request->header.type=VIRTIO_GPU_CMD_RESOURCE_UNREF;
	request->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request->header.fence_id=0;
	request->resource_id=resource_id;
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
			sizeof(virtio_gpu_resource_unref_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_control_header_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		ERROR("virtio_gpu_command_resource_unref failed");
	}
	amm_dealloc(response);
}



void virtio_gpu_command_set_scanout(virtio_gpu_device_t* gpu_device,ui_display_t* display){
	virtio_gpu_set_scanout_t* request=amm_alloc(sizeof(virtio_gpu_set_scanout_t));
	request->header.type=VIRTIO_GPU_CMD_SET_SCANOUT;
	request->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request->header.fence_id=0;
	request->rect.x=0;
	request->rect.y=0;
	request->rect.width=display->framebuffer->width;
	request->rect.height=display->framebuffer->height;
	request->scanout_id=display->index;
	request->resource_id=gpu_device->framebuffer_resources[display->index];
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
			sizeof(virtio_gpu_set_scanout_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_control_header_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		ERROR("virtio_gpu_set_scanout failed");
	}
	amm_dealloc(response);
}



void virtio_gpu_command_resource_flush(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,u32 width,u32 height){
	virtio_gpu_resource_flush_t* request=amm_alloc(sizeof(virtio_gpu_resource_flush_t));
	request->header.type=VIRTIO_GPU_CMD_RESOURCE_FLUSH;
	request->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request->header.fence_id=0;
	request->rect.x=0;
	request->rect.y=0;
	request->rect.width=width;
	request->rect.height=height;
	request->resource_id=resource_id;
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
			sizeof(virtio_gpu_resource_flush_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_control_header_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		ERROR("virtio_gpu_resource_flush failed");
	}
	amm_dealloc(response);
}



void virtio_gpu_command_transfer_to_host_2d(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,u32 width,u32 height){
	virtio_gpu_transfer_to_host_2d_t* request=amm_alloc(sizeof(virtio_gpu_transfer_to_host_2d_t));
	request->header.type=VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D;
	request->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request->header.fence_id=0;
	request->rect.x=0;
	request->rect.y=0;
	request->rect.width=width;
	request->rect.height=height;
	request->offset=0;
	request->resource_id=resource_id;
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
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
	amm_dealloc(request);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		ERROR("virtio_gpu_transfer_to_host_2d failed");
	}
	amm_dealloc(response);
}



void virtio_gpu_command_resource_attach_backing(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,u64 address,u32 length){
	virtio_gpu_resource_attach_backing_t* request=amm_alloc(sizeof(virtio_gpu_resource_attach_backing_t)+sizeof(virtio_gpu_mem_entry_t));
	request->header.type=VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
	request->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request->header.fence_id=0;
	request->resource_id=resource_id;
	request->entry_count=1;
	request->entries[0].address=address;
	request->entries[0].length=length;
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
			sizeof(virtio_gpu_resource_attach_backing_t)+sizeof(virtio_gpu_mem_entry_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_control_header_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		ERROR("virtio_gpu_resource_attach_backing failed");
	}
	amm_dealloc(response);
}



void virtio_gpu_command_resource_detach_backing(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id){
	virtio_gpu_resource_detach_backing_t* request=amm_alloc(sizeof(virtio_gpu_resource_detach_backing_t));
	request->header.type=VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING;
	request->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request->header.fence_id=0;
	request->resource_id=resource_id;
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
			sizeof(virtio_gpu_resource_detach_backing_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_control_header_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		ERROR("virtio_gpu_command_resource_unref failed");
	}
	amm_dealloc(response);
}



virtio_gpu_resp_capset_info_t* virtio_gpu_command_get_capset_info(virtio_gpu_device_t* gpu_device,u32 index){
	virtio_gpu_get_capset_info_t* request=amm_alloc(sizeof(virtio_gpu_get_capset_info_t));
	request->header.type=VIRTIO_GPU_CMD_GET_CAPSET_INFO;
	request->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request->header.fence_id=0;
	request->capset_index=index;
	virtio_gpu_resp_capset_info_t* response=amm_alloc(sizeof(virtio_gpu_resp_capset_info_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
			sizeof(virtio_gpu_get_capset_info_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_resp_capset_info_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request);
	if (response->header.type==VIRTIO_GPU_RESP_OK_CAPSET_INFO){
		return response;
	}
	ERROR("virtio_gpu_command_get_capset_info failed");
	amm_dealloc(response);
	return NULL;
}



virtio_gpu_resp_capset_t* virtio_gpu_command_get_capset(virtio_gpu_device_t* gpu_device,u32 capset_id,u32 capset_version,u32 capset_size){
	virtio_gpu_get_capset_t* request=amm_alloc(sizeof(virtio_gpu_get_capset_t));
	request->header.type=VIRTIO_GPU_CMD_GET_CAPSET;
	request->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request->header.fence_id=0;
	request->capset_id=capset_id;
	request->capset_version=capset_version;
	virtio_gpu_resp_capset_t* response=amm_alloc(sizeof(virtio_gpu_resp_capset_t)+capset_size);
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
			sizeof(virtio_gpu_get_capset_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_resp_capset_t)+capset_size
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request);
	if (response->header.type==VIRTIO_GPU_RESP_OK_CAPSET){
		return response;
	}
	ERROR("virtio_gpu_command_get_capset failed");
	amm_dealloc(response);
	return NULL;
}



virtio_gpu_resp_edid_t* virtio_gpu_command_get_edid(virtio_gpu_device_t* gpu_device,u32 scanout){
	virtio_gpu_get_edid_t* request=amm_alloc(sizeof(virtio_gpu_get_edid_t));
	request->header.type=VIRTIO_GPU_CMD_GET_EDID;
	request->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request->header.fence_id=0;
	request->scanout=scanout;
	virtio_gpu_resp_edid_t* response=amm_alloc(sizeof(virtio_gpu_resp_edid_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
			sizeof(virtio_gpu_get_edid_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_resp_edid_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request);
	if (response->header.type==VIRTIO_GPU_RESP_OK_EDID){
		return response;
	}
	ERROR("virtio_gpu_command_get_edid failed");
	amm_dealloc(response);
	return NULL;
}



void virtio_gpu_command_ctx_create(virtio_gpu_device_t* gpu_device,u32 ctx,u32 type){
	virtio_gpu_ctx_create_t* request=amm_alloc(sizeof(virtio_gpu_ctx_create_t));
	request->header.type=VIRTIO_GPU_CMD_CTX_CREATE;
	request->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request->header.fence_id=0;
	request->header.ctx_id=ctx;
	request->debug_name_length=sizeof(CONTEXT_NAME)-1;
	request->context_init=type;
	memcpy(request->debug_name,CONTEXT_NAME,sizeof(CONTEXT_NAME));
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
			sizeof(virtio_gpu_ctx_create_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_control_header_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		WARN("virtio_gpu_command_ctx_create failed");
	}
	amm_dealloc(response);
}



void virtio_gpu_command_ctx_destroy(virtio_gpu_device_t* gpu_device,u32 ctx){
	virtio_gpu_control_header_t* request=amm_alloc(sizeof(virtio_gpu_control_header_t));
	request->type=VIRTIO_GPU_CMD_CTX_DESTROY;
	request->flags=VIRTIO_GPU_FLAG_FENCE;
	request->fence_id=0;
	request->ctx_id=ctx;
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
			sizeof(virtio_gpu_control_header_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_control_header_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		WARN("virtio_gpu_command_ctx_destroy failed");
	}
	amm_dealloc(response);
}



void virtio_gpu_command_ctx_attach_resource(virtio_gpu_device_t* gpu_device,u32 ctx,virtio_gpu_resource_id_t resource_id){
	panic("virtio_gpu_command_ctx_attach_resource");
}



void virtio_gpu_command_ctx_detach_resource(virtio_gpu_device_t* gpu_device,u32 ctx,virtio_gpu_resource_id_t resource_id){
	panic("virtio_gpu_command_ctx_detach_resource");
}



void virtio_gpu_command_resource_create_3d(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,u32 target,u32 format,u32 bind,u32 width,u32 height,u32 depth,u32 array_size,u32 last_level,u32 nr_samples){
	panic("virtio_gpu_command_resource_create_3d");
}



void virtio_gpu_command_transfer_to_host_3d(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,const virtio_gpu_box_t* box,u32 level,u32 stride,u32 layer_stride){
	panic("virtio_gpu_command_transfer_to_host_3d");
}



void virtio_gpu_command_transfer_from_host_3d(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,const virtio_gpu_box_t* box,u32 level,u32 stride,u32 layer_stride){
	panic("virtio_gpu_command_transfer_from_host_3d");
}



void virtio_gpu_command_submit_3d(virtio_gpu_device_t* gpu_device,u32 ctx,u64 buffer,u32 size){
	virtio_gpu_cmd_submit_3d_t* request=amm_alloc(sizeof(virtio_gpu_cmd_submit_3d_t));
	request->header.type=VIRTIO_GPU_CMD_SUBMIT_3D;
	request->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request->header.fence_id=0;
	request->header.ctx_id=ctx;
	request->size=size;
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[3]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request),
			sizeof(virtio_gpu_cmd_submit_3d_t)
		},
		{
			buffer,
			size
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_control_header_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,2,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	amm_dealloc(request);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		WARN("virtio_gpu_command_submit_3d failed");
	}
	amm_dealloc(response);
}



void virtio_gpu_command_update_cursor(virtio_gpu_device_t* gpu_device,virtio_gpu_resource_id_t resource_id,const virtio_gpu_cursor_pos_t* pos,u32 hot_x,u32 hot_y){
	panic("virtio_gpu_command_update_cursor");
}



void virtio_gpu_command_move_cursor(virtio_gpu_device_t* gpu_device,const virtio_gpu_cursor_pos_t* pos){
	panic("virtio_gpu_command_move_cursor");
}
