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



static void _command_buffer_extend(virgl_opengl_context_t* ctx,const u32* command,u16 command_size,_Bool flush){
	spinlock_acquire_exclusive(&(ctx->command_buffer.lock));
	if (ctx->command_buffer.size+command_size>VIRGL_OPENGL_CONTEXT_COMMAND_BUFFER_SIZE){
		virtio_gpu_command_submit_3d(ctx->gpu_device,CONTEXT_ID,ctx->command_buffer.buffer_address,ctx->command_buffer.size*sizeof(u32));
		ctx->command_buffer.size=0;
	}
	memcpy(ctx->command_buffer.buffer+ctx->command_buffer.size,command,command_size*sizeof(u32));
	ctx->command_buffer.size+=command_size;
	if (flush){
		virtio_gpu_command_submit_3d(ctx->gpu_device,CONTEXT_ID,ctx->command_buffer.buffer_address,ctx->command_buffer.size*sizeof(u32));
		ctx->command_buffer.size=0;
	}
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
	_command_buffer_extend(instance->ctx,virgl_create_sub_ctx_command,2,1);
	return 1;
}



static void _deinit_state(opengl_driver_instance_t* instance,opengl_state_t* state){
	panic("_deinit_state");
}



static void _update_render_target(opengl_driver_instance_t* instance,opengl_state_t* state){
	virgl_opengl_context_t* ctx=instance->ctx;
	u32 framebuffer_renderbuffer_resource_id=0x00000011;
	u32 framebuffer_depth_and_stencil_buffer_resource_id=0x00000022;
	u32 framebuffer_renderbuffer_surface_id=0x00000033;
	u32 framebuffer_depth_and_stencil_surface_id=0x00000044;
	u32 dsa_id=0x00000055;
	u32 rasterizer_id=0x00000066;
	u32 blend_id=0x00000077;
	virtio_gpu_command_resource_create_3d(ctx->gpu_device,framebuffer_renderbuffer_resource_id,VIRGL_TARGET_TEXTURE_2D,VIRGL_FORMAT_B8G8R8A8_UNORM,VIRGL_PROTOCOL_BIND_FLAG_RENDER_TARGET|VIRGL_PROTOCOL_BIND_FLAG_SAMPLER_VIEW|VIRGL_PROTOCOL_BIND_FLAG_GLOBAL|(1<<10),state->framebuffer->width,state->framebuffer->height,1,1,0,0);
	virtio_gpu_command_resource_attach_backing(ctx->gpu_device,framebuffer_renderbuffer_resource_id,state->framebuffer->address,state->framebuffer->size);
	virtio_gpu_command_ctx_attach_resource(ctx->gpu_device,CONTEXT_ID,framebuffer_renderbuffer_resource_id);
	virtio_gpu_command_resource_create_3d(ctx->gpu_device,framebuffer_depth_and_stencil_buffer_resource_id,VIRGL_TARGET_TEXTURE_2D,VIRGL_FORMAT_S8_UINT_Z24_UNORM,VIRGL_PROTOCOL_BIND_FLAG_DEPTH_STENCIL,state->framebuffer->width,state->framebuffer->height,1,1,0,0);
	virtio_gpu_command_ctx_attach_resource(ctx->gpu_device,CONTEXT_ID,framebuffer_depth_and_stencil_buffer_resource_id);
	u32 setup_commands[]={
		// Sub ctx
		VIRGL_PROTOCOL_COMMAND_CREATE_SUB_CTX,
		HANDLE_ID_GET_INDEX(state->handle.rb_node.key),
		VIRGL_PROTOCOL_COMMAND_SET_SUB_CTX,
		HANDLE_ID_GET_INDEX(state->handle.rb_node.key),
		// Surfaces
		VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_SURFACE,
		framebuffer_renderbuffer_surface_id,
		framebuffer_renderbuffer_resource_id,
		VIRGL_FORMAT_B8G8R8A8_UNORM,
		0,
		0,
		VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_SURFACE,
		framebuffer_depth_and_stencil_surface_id,
		framebuffer_depth_and_stencil_buffer_resource_id,
		VIRGL_FORMAT_S8_UINT_Z24_UNORM,
		0,
		0,
		// DSA
		VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_DSA,
		dsa_id,
		0,
		0,
		0,
		0,
		VIRGL_PROTOCOL_COMMAND_BIND_OBJECT_DSA,
		dsa_id,
		// Rasterizer
		VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_RASTERIZER,
		rasterizer_id,
		0x60008182,
		0x3f800000,
		0,
		0xffff,
		0x3f800000,
		0,
		0,
		0,
		VIRGL_PROTOCOL_COMMAND_BIND_OBJECT_RASTERIZER,
		rasterizer_id,
		// Blend
		VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_BLEND,
		blend_id,
		0x00000004,
		0,
		0x78000000,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		VIRGL_PROTOCOL_COMMAND_BIND_OBJECT_BLEND,
		blend_id,
		// Polygon stipple
		VIRGL_PROTOCOL_COMMAND_SET_POLYGON_STIPPLE,
		0xffffffff,0xffffffff,0xffffffff,0xffffffff,
		0xffffffff,0xffffffff,0xffffffff,0xffffffff,
		0xffffffff,0xffffffff,0xffffffff,0xffffffff,
		0xffffffff,0xffffffff,0xffffffff,0xffffffff,
		0xffffffff,0xffffffff,0xffffffff,0xffffffff,
		0xffffffff,0xffffffff,0xffffffff,0xffffffff,
		0xffffffff,0xffffffff,0xffffffff,0xffffffff,
		0xffffffff,0xffffffff,0xffffffff,0xffffffff,
		// Viewport
		VIRGL_PROTOCOL_COMMAND_SET_VIEWPORT_STATE,
		0,
		0x43160000,
		0xc3160000,
		0x3f000000,
		0x43160000,
		0x43160000,
		0x3f000000,
		// Scissors
		VIRGL_PROTOCOL_COMMAND_SET_SCISSOR_STATE,
		0,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		0x00000000,0x012c012c,
		// Framebuffer
		VIRGL_PROTOCOL_COMMAND_SET_FRAMEBUFFER_STATE,
		1,
		framebuffer_depth_and_stencil_surface_id,
		framebuffer_renderbuffer_surface_id
	};
	_command_buffer_extend(ctx,setup_commands,sizeof(setup_commands)/sizeof(u32),1);
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
				0x3f000000,
				command->tx.raw_value,
				command->ty.raw_value,
				0x3f000000
			};
			_command_buffer_extend(instance->ctx,virgl_set_viewport_state_command,8,0);
		}
		else{
			ERROR("_process_commands: unknown command '%X'",header->type);
		}
		offset+=header->length;
	}
	_command_buffer_extend(instance->ctx,NULL,0,1);
	// manually fetch the framebuffer
	u32 framebuffer_renderbuffer_resource_id=0x00000011;
	virtio_gpu_box_t box={
		0,
		0,
		0,
		state->framebuffer->width,
		state->framebuffer->height,
		1
	};
	virtio_gpu_command_transfer_from_host_3d(ctx->gpu_device,framebuffer_renderbuffer_resource_id,&box,0,0,0);
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
	virtio_gpu_command_ctx_create(gpu_device,CONTEXT_ID,VIRTIO_GPU_CAPSET_VIRGL2);
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
