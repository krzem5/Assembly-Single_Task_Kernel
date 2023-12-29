#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <opengl/opengl.h>
#include <opengl/protocol.h>
#include <virgl/protocol.h>
#include <virgl/registers.h>
#include <virgl/virgl.h>
#include <virtio/gpu.h>
#include <virtio/virtio.h>
#define KERNEL_LOG_NAME "virgl_virtio_gpu"



#define DEBUG_NAME "virgl_virtio_gpu_opengl_context"
#define CONTEXT_ID 0x00000001
#define FRAMEBUFFER_SURFACE_ID 0xcceeccee
#define FRAMEBUFFER_RESOURCE_ID 0xaabbccdd



static pmm_counter_descriptor_t* _virgl_opengl_context_commabd_buffer_pmm_counter=NULL;
static omm_allocator_t* _virgl_opengl_context_allocator=NULL;



static void _command_buffer_extend(virgl_opengl_context_t* ctx,const u32* command,u16 command_size,_Bool flush_before){
	spinlock_acquire_exclusive(&(ctx->command_buffer.lock));
	if (ctx->command_buffer.size+command_size>VIRGL_OPENGL_CONTEXT_COMMAND_BUFFER_SIZE||flush_before){
		virtio_gpu_cmd_submit_t* request_submit_3d=amm_alloc(sizeof(virtio_gpu_cmd_submit_t));
		request_submit_3d->header.type=VIRTIO_GPU_CMD_SUBMIT_3D;
		request_submit_3d->header.flags=VIRTIO_GPU_FLAG_FENCE;
		request_submit_3d->header.fence_id=0;
		request_submit_3d->header.ctx_id=CONTEXT_ID;
		request_submit_3d->size=ctx->command_buffer.size*sizeof(u32);
		virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
		virtio_buffer_t buffers[3]={
			{
				vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_submit_3d),
				sizeof(virtio_gpu_cmd_submit_t)
			},
			{
				ctx->command_buffer.buffer_address,
				ctx->command_buffer.size*sizeof(u32)
			},
			{
				vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
				sizeof(virtio_gpu_control_header_t)
			}
		};
		virtio_queue_transfer(ctx->gpu_device->controlq,buffers,2,1);
		virtio_queue_wait(ctx->gpu_device->controlq);
		virtio_queue_pop(ctx->gpu_device->controlq,NULL);
		amm_dealloc(request_submit_3d);
		if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
			WARN("Unable to submit commands");
		}
		amm_dealloc(response);
		ctx->command_buffer.size=0;
	}
	memcpy(ctx->command_buffer.buffer+ctx->command_buffer.size,command,command_size*sizeof(u32));
	ctx->command_buffer.size+=command_size;
	spinlock_release_exclusive(&(ctx->command_buffer.lock));
}



static _Bool _init_state(opengl_driver_instance_t* instance,opengl_state_t* state){
	if (HANDLE_ID_GET_INDEX(state->handle.rb_node.key)>>32){
		ERROR("Too many OpenGL states created over lifetime");
		return 0;
	}
	u32 virgl_create_sub_ctx_command[2]={
		VIRGL_PROTOCOL_COMMAND_CREATE_SUB_CTX,
		HANDLE_ID_GET_INDEX(state->handle.rb_node.key)
	};
	_command_buffer_extend(instance->ctx,virgl_create_sub_ctx_command,2,0);
	_command_buffer_extend(instance->ctx,NULL,0,1);
	return 1;
}



static void _deinit_state(opengl_driver_instance_t* instance,opengl_state_t* state){
	panic("_deinit_state");
}



static void _update_render_target(opengl_driver_instance_t* instance,opengl_state_t* state){
	virgl_opengl_context_t* ctx=instance->ctx;
	virtio_gpu_resource_create_3d_t* request_resource_create_3d=amm_alloc(sizeof(virtio_gpu_resource_create_3d_t));
	request_resource_create_3d->header.type=VIRTIO_GPU_CMD_RESOURCE_CREATE_3D;
	request_resource_create_3d->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request_resource_create_3d->header.fence_id=0;
	request_resource_create_3d->header.ctx_id=CONTEXT_ID;
	request_resource_create_3d->resource_id=FRAMEBUFFER_RESOURCE_ID;
	request_resource_create_3d->target=VIRGL_TARGET_TEXTURE_2D;
	request_resource_create_3d->format=VIRGL_FORMAT_B8G8R8X8_UNORM;
	request_resource_create_3d->bind=VIRGL_PROTOCOL_BIND_FLAG_RENDER_TARGET;
	request_resource_create_3d->width=state->framebuffer->width;
	request_resource_create_3d->height=state->framebuffer->height;
	request_resource_create_3d->depth=1;
	request_resource_create_3d->array_size=1;
	request_resource_create_3d->last_level=0;
	request_resource_create_3d->nr_samples=0;
	request_resource_create_3d->flags=0;
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_resource_create_3d),
			sizeof(virtio_gpu_resource_create_3d_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_control_header_t)
		}
	};
	virtio_queue_transfer(ctx->gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(ctx->gpu_device->controlq);
	virtio_queue_pop(ctx->gpu_device->controlq,NULL);
	amm_dealloc(request_resource_create_3d);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		WARN("Unable to create framebuffer texture");
		amm_dealloc(response);
		return;
	}
	virtio_gpu_resource_attach_backing_t* request_resource_attach_backing=amm_alloc(sizeof(virtio_gpu_resource_attach_backing_t)+sizeof(virtio_gpu_mem_entry_t));
	request_resource_attach_backing->header.type=VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
	request_resource_attach_backing->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request_resource_attach_backing->header.fence_id=0;
	request_resource_attach_backing->resource_id=FRAMEBUFFER_RESOURCE_ID;
	request_resource_attach_backing->entry_count=1;
	request_resource_attach_backing->entries[0].address=state->framebuffer->address;
	request_resource_attach_backing->entries[0].length=pmm_align_up_address(state->framebuffer->size);
	buffers[0].address=vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_resource_attach_backing);
	buffers[0].length=sizeof(virtio_gpu_resource_attach_backing_t)+sizeof(virtio_gpu_mem_entry_t);
	virtio_queue_transfer(ctx->gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(ctx->gpu_device->controlq);
	virtio_queue_pop(ctx->gpu_device->controlq,NULL);
	amm_dealloc(request_resource_attach_backing);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		WARN("Unable to attach framebuffer texture backing");
		amm_dealloc(response);
		return;
	}
	virtio_gpu_ctx_attach_resource_t* request_ctx_attach_resource=amm_alloc(sizeof(virtio_gpu_ctx_attach_resource_t));
	request_ctx_attach_resource->header.type=VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE;
	request_ctx_attach_resource->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request_ctx_attach_resource->header.fence_id=0;
	request_ctx_attach_resource->resource_id=FRAMEBUFFER_RESOURCE_ID;
	buffers[0].address=vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_ctx_attach_resource);
	buffers[0].length=sizeof(virtio_gpu_ctx_attach_resource_t);
	virtio_queue_transfer(ctx->gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(ctx->gpu_device->controlq);
	virtio_queue_pop(ctx->gpu_device->controlq,NULL);
	amm_dealloc(request_ctx_attach_resource);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		WARN("Unable to attach framebuffer texture backing");
		amm_dealloc(response);
		return;
	}
	amm_dealloc(response);
	u32 virgl_create_surface_and_set_framebuffer_state_command[10]={
		VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_SURFACE,
		FRAMEBUFFER_SURFACE_ID,
		FRAMEBUFFER_RESOURCE_ID,
		VIRGL_FORMAT_B8G8R8X8_UNORM,
		0,
		0,
		VIRGL_PROTOCOL_COMMAND_SET_FRAMEBUFFER_STATE,
		1,
		0,
		FRAMEBUFFER_SURFACE_ID
	};
	_command_buffer_extend(ctx,virgl_create_surface_and_set_framebuffer_state_command,10,0);
	_command_buffer_extend(ctx,NULL,0,1);
}



static void _process_commands(opengl_driver_instance_t* instance,opengl_state_t* state,void* command_buffer,u32 command_buffer_size){
	virgl_opengl_context_t* ctx=instance->ctx;
	u32 virgl_set_sub_ctx_command[2]={
		VIRGL_PROTOCOL_COMMAND_SET_SUB_CTX,
		HANDLE_ID_GET_INDEX(state->handle.rb_node.key)
	};
	_command_buffer_extend(instance->ctx,virgl_set_sub_ctx_command,2,0);
	for (u32 offset=0;offset+sizeof(opengl_protocol_header_t)<=command_buffer_size;){
		opengl_protocol_header_t* header=command_buffer+offset;
		if ((header->length&(sizeof(u32)-1))||offset+header->length>command_buffer_size){
			ERROR("_process_commands: invalid command size");
			return;
		}
		else if (header->type==OPENGL_PROTOCOL_TYPE_CREATE_RESOURCE){
			panic("OPENGL_PROTOCOL_TYPE_CREATE_RESOURCE");
		}
		else if (header->type==OPENGL_PROTOCOL_TYPE_DELETE_RESOURCE){
			panic("OPENGL_PROTOCOL_TYPE_DELETE_RESOURCE");
		}
		else if (header->type==OPENGL_PROTOCOL_TYPE_CLEAR){
			opengl_protocol_clear_t* command=(void*)header;
			u32 flags=0;
			if (command->flags&OPENGL_PROTOCOL_CLEAR_FLAG_COLOR){
				flags|=VIRGL_PROTOCOL_CLEAR_FLAG_COLOR;
			}
			if (command->flags&OPENGL_PROTOCOL_CLEAR_FLAG_DEPTH){
				flags|=VIRGL_PROTOCOL_CLEAR_FLAG_DEPTH;
			}
			if (command->flags&OPENGL_PROTOCOL_CLEAR_FLAG_STENCIL){
				flags|=VIRGL_PROTOCOL_CLEAR_FLAG_STENCIL;
			}
			u32 virgl_set_viewport_state_command[9]={
				VIRGL_PROTOCOL_COMMAND_CLEAR,
				flags,
				command->color[0].raw_value,
				command->color[1].raw_value,
				command->color[2].raw_value,
				command->color[3].raw_value,
				command->depth.raw_value,
				command->depth.raw_value>>32,
				command->stencil
			};
			_command_buffer_extend(instance->ctx,virgl_set_viewport_state_command,9,0);
		}
		else if (header->type==OPENGL_PROTOCOL_TYPE_SET_VIEWPORT){
			opengl_protocol_set_viewport_t* command=(void*)header;
			u32 virgl_set_viewport_state_command[8]={
				VIRGL_PROTOCOL_COMMAND_SET_VIEWPORT_STATE,
				0,
				command->sx.raw_value,
				command->sy.raw_value,
				0x3f800000,
				command->tx.raw_value,
				command->ty.raw_value,
				0x00000000
			};
			_command_buffer_extend(instance->ctx,virgl_set_viewport_state_command,8,0);
		}
		else{
			ERROR("_process_commands: unknown command '%X'",header->type);
		}
		offset+=header->length;
	}
	u32 virgl_memory_barrier_command[2]={
		VIRGL_PROTOCOL_COMMAND_MEMORY_BARRIER,
		0x3fff
	};
	_command_buffer_extend(instance->ctx,virgl_memory_barrier_command,2,0);
	_command_buffer_extend(instance->ctx,NULL,0,1);
	// manually fetch the framebuffer
	virtio_gpu_transfer_from_host_3d_t* request_transfer_from_host_3d=amm_alloc(sizeof(virtio_gpu_transfer_from_host_3d_t));
	request_transfer_from_host_3d->header.type=VIRTIO_GPU_CMD_TRANSFER_FROM_HOST_3D;
	request_transfer_from_host_3d->header.flags=VIRTIO_GPU_FLAG_FENCE;
	request_transfer_from_host_3d->header.fence_id=0;
	request_transfer_from_host_3d->header.ctx_id=CONTEXT_ID;
	request_transfer_from_host_3d->box.x=0;
	request_transfer_from_host_3d->box.y=0;
	request_transfer_from_host_3d->box.z=0;
	request_transfer_from_host_3d->box.width=state->framebuffer->width;
	request_transfer_from_host_3d->box.height=state->framebuffer->height;
	request_transfer_from_host_3d->box.depth=1;
	request_transfer_from_host_3d->offset=0;
	request_transfer_from_host_3d->resource_id=FRAMEBUFFER_RESOURCE_ID;
	request_transfer_from_host_3d->level=0;
	request_transfer_from_host_3d->stride=0;
	request_transfer_from_host_3d->layer_stride=0;
	virtio_gpu_control_header_t* response=amm_alloc(sizeof(virtio_gpu_control_header_t));
	virtio_buffer_t buffers[2]={
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)request_transfer_from_host_3d),
			sizeof(virtio_gpu_transfer_from_host_3d_t)
		},
		{
			vmm_virtual_to_physical(&vmm_kernel_pagemap,(u64)response),
			sizeof(virtio_gpu_control_header_t)
		}
	};
	virtio_queue_transfer(ctx->gpu_device->controlq,buffers,1,1);
	virtio_queue_wait(ctx->gpu_device->controlq);
	virtio_queue_pop(ctx->gpu_device->controlq,NULL);
	amm_dealloc(request_transfer_from_host_3d);
	if (response->type!=VIRTIO_GPU_RESP_OK_NODATA){
		WARN("Unable to create transfer framebuffer from host");
	}
	amm_dealloc(response);
}



static const opengl_driver_t _virgl_opengl_driver={
	"virtio_gpu_virgl",
	330,
	_init_state,
	_deinit_state,
	_update_render_target,
	_process_commands
};



void virgl_init(void){
	_virgl_opengl_context_commabd_buffer_pmm_counter=pmm_alloc_counter("virgl_opengl_context_command_buffer");
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
	spinlock_init(&(ctx->command_buffer.lock));
	INFO("Allocating command buffer..");
	ctx->command_buffer.buffer_address=pmm_alloc(pmm_align_up_address(VIRGL_OPENGL_CONTEXT_COMMAND_BUFFER_SIZE*sizeof(u32))>>PAGE_SIZE_SHIFT,_virgl_opengl_context_commabd_buffer_pmm_counter,0);
	ctx->command_buffer.buffer=(void*)(ctx->command_buffer.buffer_address+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	ctx->command_buffer.size=0;
	opengl_create_driver_instance(&_virgl_opengl_driver,caps->renderer,ctx);
}
