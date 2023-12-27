#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <opengl/opengl.h>
#include <virgl/registers.h>
#include <virgl/virgl.h>
#include <virtio/gpu.h>
#include <virtio/virtio.h>
#define KERNEL_LOG_NAME "virgl_virtio_gpu"



#define DEBUG_NAME "virgl_virtio_gpu_opengl_context"
#define CONTEXT_ID 0x00000001



static omm_allocator_t* _virgl_opengl_context_allocator=NULL;



static void _init_state(opengl_driver_instance_t* instance,opengl_state_t* state){
	panic("_init_state");
}



static void _deinit_state(opengl_driver_instance_t* instance,opengl_state_t* state){
	panic("_deinit_state");
}



static const opengl_driver_t _virgl_opengl_driver={
	"virtio_gpu_virgl",
	460,
	_init_state,
	_deinit_state
};



void virgl_init(void){
	_virgl_opengl_context_allocator=omm_init("virgl_opengl_context",sizeof(virgl_opengl_context_t),8,4,pmm_alloc_counter("omm_virgl_opengl_context"));
	spinlock_init(&(_virgl_opengl_context_allocator->lock));
}



void virgl_load_from_virtio_gpu_capset(virtio_gpu_device_t* gpu_device,_Bool is_v2,const void* data,u32 size){
	LOG("Initializing OpenGL with virgl%s backend...",(is_v2?"2":""));
	if (!is_v2){
		panic("virgl_load_from_virtio_gpu_capset: virgl v1 capset is unimplemented");
	}
	if (size<sizeof(virgl_caps_v2_t)){
		WARN("Incomplete virgl capset");
		return;
	}
	const virgl_caps_v2_t* caps=data;
	INFO("Renderer: %s",caps->renderer);
	virtio_gpu_ctx_create_t* request_ctx_create=amm_alloc(sizeof(virtio_gpu_ctx_create_t));
	request_ctx_create->header.type=VIRTIO_GPU_CMD_CTX_CREATE;
	request_ctx_create->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request_ctx_create->header.fence_id=0;
	request_ctx_create->header.ctx_id=CONTEXT_ID;
	request_ctx_create->debug_name_length=sizeof(DEBUG_NAME)-1;
	request_ctx_create->context_init=VIRTIO_GPU_CAPSET_VIRGL2;
	memcpy(request_ctx_create->debug_name,DEBUG_NAME,sizeof(DEBUG_NAME));
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_ctx_create),
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
	amm_dealloc(request_ctx_create);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		WARN("Unable to create GPU context");
		amm_dealloc(response);
		return;
	}
	amm_dealloc(response);
	virgl_opengl_context_t* ctx=omm_alloc(_virgl_opengl_context_allocator);
	ctx->gpu_device=gpu_device;
	ctx->caps=*caps;
	opengl_create_driver_instance(&_virgl_opengl_driver,caps->renderer,ctx);
}
