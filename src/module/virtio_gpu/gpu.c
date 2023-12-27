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
#include <virtio/registers.h>
#include <virtio/virtio.h>
#define KERNEL_LOG_NAME "virtio_gpu"



static omm_allocator_t* _virtio_gpu_device_allocator=NULL;



static _Bool _fetch_display_info(virtio_gpu_device_t* gpu_device){
	virtio_write(gpu_device->device->device_field+VIRTIO_GPU_REG_EVENTS_CLEAR,4,VIRTIO_GPU_EVENT_DISPLAY);
	virtio_gpu_control_header_t __attribute__((aligned(sizeof(virtio_gpu_control_header_t)+8))) request={ // alignment prevents splits across stack pages
		.type=VIRTIO_GPU_CMD_GET_DISPLAY_INFO,
		.flags=VIRTIO_GPU_FLAG_FENCE
	};
	virtio_gpu_resp_display_info_t* resp=amm_alloc(sizeof(virtio_gpu_resp_display_info_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)(&request)),
			sizeof(virtio_gpu_control_header_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)resp),
			sizeof(virtio_gpu_resp_display_info_t)
		}
	};
	virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(gpu_device->controlq);
	virtio_queue_pop(gpu_device->controlq,NULL);
	_Bool out=0;
	if (resp->header.type!=VIRTIO_GPU_RESP_OK_DISPLAY_INFO){
		goto _cleanup;
	}
	for (u32 i=0;i<gpu_device->scanout_count;i++){
		WARN("[%u]: (%u, %u), (%u, %u)",i,(resp->displays+i)->rect.x,(resp->displays+i)->rect.y,(resp->displays+i)->rect.width,(resp->displays+i)->rect.height);
	}
	out=1;
_cleanup:
	amm_dealloc(resp);
	return out;
}



static _Bool _display_resize(ui_display_t* display){
	panic("_display_resize");
}



static const ui_display_driver_t _virtio_gpu_display_driver={
	"VirtIO GPU",
	_display_resize
};



static void _fetch_edid_data(virtio_gpu_device_t* gpu_device){
	virtio_gpu_get_edid_t __attribute__((aligned(sizeof(virtio_gpu_get_edid_t)))) request={ // alignment prevents splits across stack pages
		.header.type=VIRTIO_GPU_CMD_GET_EDID,
		.header.flags=VIRTIO_GPU_FLAG_FENCE
	};
	virtio_gpu_resp_edid_t* resp=amm_alloc(sizeof(virtio_gpu_resp_edid_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)(&request)),
			sizeof(virtio_gpu_get_edid_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)resp),
			sizeof(virtio_gpu_resp_edid_t)
		}
	};
	for (u32 i=0;i<gpu_device->scanout_count;i++){
		request.scanout=i;
		virtio_queue_transfer(gpu_device->controlq,buffers,1,1);
		virtio_queue_wait(gpu_device->controlq);
		virtio_queue_pop(gpu_device->controlq,NULL);
		if (resp->header.type!=VIRTIO_GPU_RESP_OK_EDID){
			WARN("No EDID response from display #%u",i);
			continue;
		}
		ui_display_add(&_virtio_gpu_display_driver,gpu_device,i,resp->edid,1024);
	}
	amm_dealloc(resp);
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
	virtio_write(device->common_field+VIRTIO_REG_DEVICE_STATUS,1,VIRTIO_DEVICE_STATUS_FLAG_ACKNOWLEDGE|VIRTIO_DEVICE_STATUS_FLAG_DRIVER|VIRTIO_DEVICE_STATUS_FLAG_DRIVER_OK|VIRTIO_DEVICE_STATUS_FLAG_FEATURES_OK);
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
