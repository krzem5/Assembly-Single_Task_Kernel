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
	u32 command[2]={
		VIRGL_PROTOCOL_COMMAND_CREATE_SUB_CTX,
		HANDLE_ID_GET_INDEX(state->handle.rb_node.key)
	};
	_command_buffer_extend(instance->ctx,command,2,0);
	_command_buffer_extend(instance->ctx,NULL,0,1);
	return 1;
}



static void _deinit_state(opengl_driver_instance_t* instance,opengl_state_t* state){
	panic("_deinit_state");
}



static void _process_commands(opengl_driver_instance_t* instance,opengl_state_t* state,void* command_buffer,u32 command_buffer_size){
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
				flags|=0x03fc;
			}
			if (command->flags&OPENGL_PROTOCOL_CLEAR_FLAG_DEPTH){
				flags|=0x0001;
			}
			if (command->flags&OPENGL_PROTOCOL_CLEAR_FLAG_STENCIL){
				flags|=0x0002;
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
	_command_buffer_extend(instance->ctx,NULL,0,1);
}



static const opengl_driver_t _virgl_opengl_driver={
	"virtio_gpu_virgl",
	330,
	_init_state,
	_deinit_state,
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
