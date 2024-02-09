#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/syscall/syscall.h>
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



#define MAX_INLINE_WRITE_SIZE 256 // must be a multiple of 4

#define DEBUG_NAME "virgl_virtio_gpu_opengl_context"
#define CONTEXT_ID 0x00000001



static const virgl_opengl_vertex_array_element_type_t _virgl_vertex_array_element_types[]={
	{VIRGL_FORMAT_R10G10B10A2_UNORM,4,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_INT_2_10_10_10_REV,1},
	{VIRGL_FORMAT_R64_FLOAT,1,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_DOUBLE,0},
	{VIRGL_FORMAT_R64G64_FLOAT,2,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_DOUBLE,0},
	{VIRGL_FORMAT_R64G64B64_FLOAT,3,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_DOUBLE,0},
	{VIRGL_FORMAT_R64G64B64A64_FLOAT,4,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_DOUBLE,0},
	{VIRGL_FORMAT_R32_FLOAT,1,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_FLOAT,0},
	{VIRGL_FORMAT_R32G32_FLOAT,2,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_FLOAT,0},
	{VIRGL_FORMAT_R32G32B32_FLOAT,3,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_FLOAT,0},
	{VIRGL_FORMAT_R32G32B32A32_FLOAT,4,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_FLOAT,0},
	{VIRGL_FORMAT_R32_UNORM,1,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_INT,1},
	{VIRGL_FORMAT_R32G32_UNORM,2,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_INT,1},
	{VIRGL_FORMAT_R32G32B32_UNORM,3,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_INT,1},
	{VIRGL_FORMAT_R32G32B32A32_UNORM,4,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_INT,1},
	{VIRGL_FORMAT_R32_SNORM,1,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_INT,1},
	{VIRGL_FORMAT_R32G32_SNORM,2,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_INT,1},
	{VIRGL_FORMAT_R32G32B32_SNORM,3,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_INT,1},
	{VIRGL_FORMAT_R32G32B32A32_SNORM,4,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_INT,1},
	{VIRGL_FORMAT_R16_UNORM,1,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_SHORT,1},
	{VIRGL_FORMAT_R16G16_UNORM,2,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_SHORT,1},
	{VIRGL_FORMAT_R16G16B16_UNORM,3,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_SHORT,1},
	{VIRGL_FORMAT_R16G16B16A16_UNORM,4,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_SHORT,1},
	{VIRGL_FORMAT_R16_SNORM,1,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_SHORT,1},
	{VIRGL_FORMAT_R16G16_SNORM,2,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_SHORT,1},
	{VIRGL_FORMAT_R16G16B16_SNORM,3,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_SHORT,1},
	{VIRGL_FORMAT_R16G16B16A16_SNORM,4,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_SHORT,1},
	{VIRGL_FORMAT_R8_UNORM,1,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_BYTE,1},
	{VIRGL_FORMAT_R8G8_UNORM,2,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_BYTE,1},
	{VIRGL_FORMAT_R8G8B8_UNORM,3,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_BYTE,1},
	{VIRGL_FORMAT_R8G8B8A8_UNORM,4,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_BYTE,1},
	{VIRGL_FORMAT_R8_SNORM,1,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_BYTE,1},
	{VIRGL_FORMAT_R8G8_SNORM,2,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_BYTE,1},
	{VIRGL_FORMAT_R8G8B8_SNORM,3,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_BYTE,1},
	{VIRGL_FORMAT_R8G8B8A8_SNORM,4,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_BYTE,1},
	{VIRGL_FORMAT_R16_FLOAT,1,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_HALF_FLOAT,0},
	{VIRGL_FORMAT_R16G16_FLOAT,2,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_HALF_FLOAT,0},
	{VIRGL_FORMAT_R16G16B16_FLOAT,3,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_HALF_FLOAT,0},
	{VIRGL_FORMAT_R16G16B16A16_FLOAT,4,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_HALF_FLOAT,0},
	{VIRGL_FORMAT_B10G10R10A2_UNORM,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_SIZE_BGRA,OPENGL_PROTOCOL_VERTEX_ARRAY_ELEMENT_TYPE_UNSIGNED_INT_2_10_10_10_REV,1},
	{VIRGL_FORMAT_NONE,0,0,0}
};



static const u32 _virgl_texture_format_map[]={
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA32F]=VIRGL_FORMAT_R32G32B32A32_FLOAT,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA32I]=VIRGL_FORMAT_RGBA32I,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA32UI]=VIRGL_FORMAT_RGBA32UI,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA16]=VIRGL_FORMAT_R16G16B16A16_UNORM,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA16F]=VIRGL_FORMAT_R16G16B16A16_FLOAT,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA16I]=VIRGL_FORMAT_RGBA16I,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA16UI]=VIRGL_FORMAT_RGBA16UI,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA8]=VIRGL_FORMAT_R8G8B8A8_UNORM,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA8UI]=VIRGL_FORMAT_RGBA8UI,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_SRGB8_ALPHA8]=VIRGL_FORMAT_SRGB8_ALPHA8,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB10_A2]=VIRGL_FORMAT_R10G10B10A2_UNORM,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB10_A2UI]=VIRGL_FORMAT_RGB10_A2UI,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R11F_G11F_B10F]=VIRGL_FORMAT_R11F_G11F_B10F,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG32F]=VIRGL_FORMAT_R32G32_FLOAT,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG32I]=VIRGL_FORMAT_RG32I,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG32UI]=VIRGL_FORMAT_RG32UI,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG16]=VIRGL_FORMAT_R16G16_UNORM,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG16F]=VIRGL_FORMAT_R16G16_FLOAT,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB16I]=VIRGL_FORMAT_RGB16I,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB16UI]=VIRGL_FORMAT_RGB16UI,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG8]=VIRGL_FORMAT_R8G8_UNORM,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG8I]=VIRGL_FORMAT_RG8I,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG8UI]=VIRGL_FORMAT_RG8UI,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R32F]=VIRGL_FORMAT_R32_FLOAT,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R32I]=VIRGL_FORMAT_R32I,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R32UI]=VIRGL_FORMAT_R32UI,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R16F]=VIRGL_FORMAT_R16_FLOAT,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R16I]=VIRGL_FORMAT_R16I,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R16UI]=VIRGL_FORMAT_R16UI,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R8]=VIRGL_FORMAT_R8_UNORM,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R8I]=VIRGL_FORMAT_R8I,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R8UI]=VIRGL_FORMAT_R8UI,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA16_SNORM]=VIRGL_FORMAT_R16G16B16A16_SNORM,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGBA8_SNORM]=VIRGL_FORMAT_R8G8B8A8_SNORM,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB32F]=VIRGL_FORMAT_R32G32B32_FLOAT,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB32I]=VIRGL_FORMAT_RGB32I,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB32UI]=VIRGL_FORMAT_RGB32UI,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB16_SNORM]=VIRGL_FORMAT_R16G16B16_SNORM,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB16F]=VIRGL_FORMAT_R16G16B16_FLOAT,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB16]=VIRGL_FORMAT_R16G16B16_UNORM,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB8_SNORM]=VIRGL_FORMAT_R8G8B8_SNORM,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB8]=VIRGL_FORMAT_R8G8B8_UNORM,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB8I]=VIRGL_FORMAT_RGB8I,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB8UI]=VIRGL_FORMAT_RGB8UI,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_SRGB8]=VIRGL_FORMAT_SRGB8,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RGB9_E5]=VIRGL_FORMAT_RGB9_E5,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG16_SNORM]=VIRGL_FORMAT_R16G16_SNORM,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_RG8_SNORM]=VIRGL_FORMAT_R8G8_SNORM,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_COMPRESSED_RG_RGTC2]=VIRGL_FORMAT_COMPRESSED_RG_RGTC2,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_COMPRESSED_SIGNED_RG_RGTC2]=VIRGL_FORMAT_COMPRESSED_SIGNED_RG_RGTC2,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R16_SNORM]=VIRGL_FORMAT_R16_SNORM,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_R8_SNORM]=VIRGL_FORMAT_R8_SNORM,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_COMPRESSED_RED_RGTC1]=VIRGL_FORMAT_COMPRESSED_RED_RGTC1,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_COMPRESSED_SIGNED_RED_RGTC1]=VIRGL_FORMAT_COMPRESSED_SIGNED_RED_RGTC1,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_DEPTH_COMPONENT32F]=VIRGL_FORMAT_DEPTH_COMPONENT32F,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_DEPTH_COMPONENT24]=VIRGL_FORMAT_DEPTH_COMPONENT24,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_DEPTH_COMPONENT16]=VIRGL_FORMAT_DEPTH_COMPONENT16,
	// [OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_DEPTH32F_STENCIL8]=VIRGL_FORMAT_DEPTH32F_STENCIL8,
	[OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8]=VIRGL_FORMAT_S8_UINT_Z24_UNORM,
};



static pmm_counter_descriptor_t* _virgl_opengl_context_command_buffer_pmm_counter=NULL;
static pmm_counter_descriptor_t* _virgl_opengl_buffer_pmm_counter=NULL;
static omm_allocator_t* _virgl_opengl_context_allocator=NULL;
static omm_allocator_t* _virgl_opengl_state_context_allocator=NULL;
static omm_allocator_t* _virgl_opengl_shader_allocator=NULL;
static omm_allocator_t* _virgl_opengl_vertex_array_allocator=NULL;
static omm_allocator_t* _virgl_opengl_buffer_allocator=NULL;
static omm_allocator_t* _virgl_opengl_texture_allocator=NULL;
static handle_type_t _virgl_opengl_shader_handle_type=0;
static handle_type_t _virgl_opengl_vertex_array_handle_type=0;
static handle_type_t _virgl_opengl_buffer_handle_type=0;
static handle_type_t _virgl_opengl_texture_handle_type=0;



static void _command_buffer_extend(virgl_opengl_context_t* ctx,const u32* command,u16 command_size,_Bool flush){
	spinlock_acquire_exclusive(&(ctx->command_buffer.lock));
	if (ctx->command_buffer.size+command_size>VIRGL_OPENGL_CONTEXT_COMMAND_BUFFER_SIZE){
		virtio_gpu_command_submit_3d(ctx->gpu_device,CONTEXT_ID,ctx->command_buffer.buffer_address,ctx->command_buffer.size*sizeof(u32));
		ctx->command_buffer.size=0;
	}
	if (command){
		memcpy(ctx->command_buffer.buffer+ctx->command_buffer.size,command,command_size*sizeof(u32));
		ctx->command_buffer.size+=command_size;
	}
	if (flush&&ctx->command_buffer.size){
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
	virgl_opengl_state_context_t* state_ctx=state->ctx;
	if (!state_ctx){
		state_ctx=omm_alloc(_virgl_opengl_state_context_allocator);
		state_ctx->resource_manager=resource_manager_create(1,0xffffffff);
		state->ctx=state_ctx;
	}
	virgl_resource_t framebuffer_renderbuffer_surface_id=resource_alloc(state_ctx->resource_manager);
	virgl_resource_t framebuffer_depth_and_stencil_surface_id=resource_alloc(state_ctx->resource_manager);
	virgl_resource_t dsa_id=resource_alloc(state_ctx->resource_manager);
	virgl_resource_t rasterizer_id=resource_alloc(state_ctx->resource_manager);
	virgl_resource_t blend_id=resource_alloc(state_ctx->resource_manager);
	virtio_gpu_command_ctx_attach_resource(ctx->gpu_device,CONTEXT_ID,state->framebuffer->gpu_handle);
	virtio_gpu_resource_id_t framebuffer_depth_and_stencil_buffer_resource_id=virtio_gpu_command_resource_create_3d(ctx->gpu_device,0,VIRGL_TARGET_TEXTURE_2D,VIRGL_FORMAT_S8_UINT_Z24_UNORM,VIRGL_PROTOCOL_BIND_FLAG_DEPTH_STENCIL,state->framebuffer->width,state->framebuffer->height,1,1,0,0);
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
		state->framebuffer->gpu_handle,
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
		0x60008080,
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
	virgl_opengl_state_context_t* state_ctx=state->ctx;
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
		else if (header->type==OPENGL_PROTOCOL_TYPE_CREATE_SHADER){
			opengl_protocol_create_shader_t* command=(void*)header;
			if (syscall_get_user_pointer_max_length(command->vertex_shader_data)<command->vertex_shader_size||syscall_get_user_pointer_max_length(command->fragment_shader_data)<command->fragment_shader_size){
				ERROR("_process_commands: invalid user pointers");
				goto _skip_create_shader_command;
			}
			virgl_opengl_shader_t* shader=omm_alloc(_virgl_opengl_shader_allocator);
			handle_new(shader,_virgl_opengl_shader_handle_type,&(shader->handle));
			shader->vertex_shader=resource_alloc(state_ctx->resource_manager);
			shader->fragment_shader=resource_alloc(state_ctx->resource_manager);
			u32 virgl_create_vertex_shader_command[6]={
				VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_SHADER(command->vertex_shader_size>>2),
				shader->vertex_shader,
				VIRGL_SHADER_VERTEX,
				command->vertex_shader_size,
				command->vertex_shader_size,
				0
			};
			_command_buffer_extend(instance->ctx,virgl_create_vertex_shader_command,6,0);
			_command_buffer_extend(instance->ctx,(const u32*)(command->vertex_shader_data),command->vertex_shader_size>>2,0);
			u32 virgl_create_fragment_shader_command[6]={
				VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_SHADER(command->fragment_shader_size>>2),
				shader->fragment_shader,
				VIRGL_SHADER_FRAGMENT,
				command->fragment_shader_size,
				command->fragment_shader_size,
				0
			};
			_command_buffer_extend(instance->ctx,virgl_create_fragment_shader_command,6,0);
			_command_buffer_extend(instance->ctx,(const u32*)(command->fragment_shader_data),command->fragment_shader_size>>2,0);
			handle_finish_setup(&(shader->handle));
			command->driver_handle=shader->handle.rb_node.key;
_skip_create_shader_command:
		}
		else if (header->type==OPENGL_PROTOCOL_TYPE_USE_SHADER){
			opengl_protocol_use_shader_t* command=(void*)header;
			handle_t* handle=handle_lookup_and_acquire(command->driver_handle,_virgl_opengl_shader_handle_type);
			if (!handle){
				ERROR("_process_commands: invalid shader handle: %p");
				goto _skip_use_shader;
			}
			const virgl_opengl_shader_t* shader=handle->object;
			u32 virgl_bind_shader_command[13]={
				VIRGL_PROTOCOL_COMMAND_BIND_SHADER,
				shader->vertex_shader,
				VIRGL_SHADER_VERTEX,
				VIRGL_PROTOCOL_COMMAND_BIND_SHADER,
				shader->fragment_shader,
				VIRGL_SHADER_FRAGMENT,
				VIRGL_PROTOCOL_COMMAND_LINK_SHADER,
				shader->vertex_shader,
				shader->fragment_shader,
				0,
				0,
				0,
				0
			};
			_command_buffer_extend(instance->ctx,virgl_bind_shader_command,13,0);
			handle_release(handle);
_skip_use_shader:
		}
		else if (header->type==OPENGL_PROTOCOL_TYPE_DRAW){
			opengl_protocol_draw_t* command=(void*)header;
			u32 mode=0;
			switch (command->mode){
				case OPENGL_PROTOCOL_DRAW_MODE_POINTS:
					mode=VIRGL_PRIMITIVE_POINTS;
					break;
				case OPENGL_PROTOCOL_DRAW_MODE_LINES:
					mode=VIRGL_PRIMITIVE_LINES;
					break;
				case OPENGL_PROTOCOL_DRAW_MODE_LINE_LOOP:
					mode=VIRGL_PRIMITIVE_LINE_LOOP;
					break;
				case OPENGL_PROTOCOL_DRAW_MODE_LINE_STRIP:
					mode=VIRGL_PRIMITIVE_LINE_STRIP;
					break;
				case OPENGL_PROTOCOL_DRAW_MODE_TRIANGLES:
					mode=VIRGL_PRIMITIVE_TRIANGLES;
					break;
				case OPENGL_PROTOCOL_DRAW_MODE_TRIANGLE_STRIP:
					mode=VIRGL_PRIMITIVE_TRIANGLE_STRIP;
					break;
				case OPENGL_PROTOCOL_DRAW_MODE_TRIANGLE_FAN:
					mode=VIRGL_PRIMITIVE_TRIANGLE_FAN;
					break;
				case OPENGL_PROTOCOL_DRAW_MODE_LINES_ADJACENCY:
					mode=VIRGL_PRIMITIVE_LINES_ADJACENCY;
					break;
				case OPENGL_PROTOCOL_DRAW_MODE_LINE_STRIP_ADJACENCY:
					mode=VIRGL_PRIMITIVE_LINE_STRIP_ADJACENCY;
					break;
				case OPENGL_PROTOCOL_DRAW_MODE_TRIANGLES_ADJACENCY:
					mode=VIRGL_PRIMITIVE_TRIANGLES_ADJACENCY;
					break;
				case OPENGL_PROTOCOL_DRAW_MODE_TRIANGLE_STRIP_ADJACENCY:
					mode=VIRGL_PRIMITIVE_TRIANGLE_STRIP_ADJACENCY;
					break;
				default:
					ERROR("_process_commands: unknown draw mode: %u",command->mode);
					goto _skip_draw_command;
			}
			u32 virgl_draw_vbo_command[13]={
				VIRGL_PROTOCOL_COMMAND_DRAW_VBO,
				command->start,
				command->count,
				mode,
				!!command->indexed,
				command->instance_count,
				0,
				0,
				0,
				0,
				command->start,
				command->count-command->start,
				0,
			};
			_command_buffer_extend(instance->ctx,virgl_draw_vbo_command,13,0);
_skip_draw_command:
		}
		else if (header->type==OPENGL_PROTOCOL_TYPE_UPDATE_VERTEX_ARRAY){
			opengl_protocol_update_vertex_array_t* command=(void*)header;
			if (command->count==0xffffffff){
				handle_t* vertex_array_handle=handle_lookup_and_acquire(command->driver_handle,_virgl_opengl_vertex_array_handle_type);
				if (!vertex_array_handle){
					ERROR("_process_commands: invalid vertex array handle: %p",command->driver_handle);
					goto _skip_update_vertex_array_command;
				}
				virgl_opengl_vertex_array_t* vertex_array=vertex_array_handle->object;
				u32 virgl_bind_vertex_elements_command[2]={
					VIRGL_PROTOCOL_COMMAND_BIND_OBJECT_VERTEX_ELEMENTS,
					vertex_array->resource_handle
				};
				_command_buffer_extend(instance->ctx,virgl_bind_vertex_elements_command,2,0);
				handle_release(vertex_array_handle);
			}
			else{
				handle_t* vertex_array_handle=handle_lookup_and_acquire(command->driver_handle,_virgl_opengl_vertex_array_handle_type);
				virgl_opengl_vertex_array_t* vertex_array=NULL;
				if (!vertex_array){
					if (command->driver_handle){
						ERROR("_process_commands: invalid vertex array handle: %p",command->driver_handle);
						goto _skip_update_vertex_array_command;
					}
					vertex_array=omm_alloc(_virgl_opengl_vertex_array_allocator);
					handle_new(vertex_array,_virgl_opengl_vertex_array_handle_type,&(vertex_array->handle));
					vertex_array->resource_handle=resource_alloc(state_ctx->resource_manager);
					handle_finish_setup(&(vertex_array->handle));
					handle_acquire(&(vertex_array->handle));
					vertex_array_handle=&(vertex_array->handle);
					command->driver_handle=vertex_array->handle.rb_node.key;
				}
				else{
					vertex_array=vertex_array_handle->object;
					u32 virgl_destroy_vertex_elements_command[2]={
						VIRGL_PROTOCOL_COMMAND_DESTROY_OBJECT_VERTEX_ELEMENTS,
						vertex_array->resource_handle
					};
					_command_buffer_extend(instance->ctx,virgl_destroy_vertex_elements_command,2,0);
				}
				u32 virgl_create_and_bind_vertex_elements_command[68]={
					VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_VERTEX_ELEMENTS(command->count),
					vertex_array->resource_handle,
				};
				for (u32 i=0;i<command->count;i++){
					virgl_create_and_bind_vertex_elements_command[(i<<2)+2]=(command->elements+i)->offset;
					virgl_create_and_bind_vertex_elements_command[(i<<2)+3]=(command->elements+i)->divisor;
					virgl_create_and_bind_vertex_elements_command[(i<<2)+4]=(command->elements+i)->index;
					virgl_create_and_bind_vertex_elements_command[(i<<2)+5]=0;
					if (!(command->elements+i)->size){
						goto _skip_type_resolution;
					}
					for (const virgl_opengl_vertex_array_element_type_t* entry=_virgl_vertex_array_element_types;entry->virgl_type!=VIRGL_FORMAT_NONE;entry++){
						if (entry->size==(command->elements+i)->size&&entry->type==(command->elements+i)->type&&entry->require_normalization==(command->elements+i)->require_normalization){
							virgl_create_and_bind_vertex_elements_command[(i<<2)+5]=entry->virgl_type;
							break;
						}
					}
					if (!virgl_create_and_bind_vertex_elements_command[(i<<2)+5]){
						ERROR("_process_commands: unknown vertex array element type: (%u,%u,%u)",(command->elements+i)->size,(command->elements+i)->type,(command->elements+i)->require_normalization);
						goto _skip_update_vertex_array_command;
					}
_skip_type_resolution:
				}
				virgl_create_and_bind_vertex_elements_command[(command->count<<2)+2]=VIRGL_PROTOCOL_COMMAND_BIND_OBJECT_VERTEX_ELEMENTS;
				virgl_create_and_bind_vertex_elements_command[(command->count<<2)+3]=vertex_array->resource_handle;
				_command_buffer_extend(instance->ctx,virgl_create_and_bind_vertex_elements_command,4+(command->count<<2),0);
				handle_release(vertex_array_handle);
			}
_skip_update_vertex_array_command:
		}
		else if (header->type==OPENGL_PROTOCOL_TYPE_UPDATE_BUFFER){
			opengl_protocol_update_buffer_t* command=(void*)header;
			if (command->data&&syscall_get_user_pointer_max_length(command->data)<command->size){
				ERROR("_process_commands: invalid user pointer");
				goto _skip_update_buffer_command;
			}
			if (!command->driver_handle){
				virgl_opengl_buffer_t* buffer=omm_alloc(_virgl_opengl_buffer_allocator);
				handle_new(buffer,_virgl_opengl_buffer_handle_type,&(buffer->handle));
				buffer->resource_handle=resource_alloc(state_ctx->resource_manager);
				buffer->storage_type=OPENGL_PROTOCOL_BUFFER_STORAGE_TYPE_DYNAMIC;
				buffer->address=0;
				buffer->size=0;
				handle_finish_setup(&(buffer->handle));
				command->driver_handle=buffer->handle.rb_node.key;
			}
			handle_t* buffer_handle=handle_lookup_and_acquire(command->driver_handle,_virgl_opengl_buffer_handle_type);
			if (!buffer_handle){
				ERROR("_process_commands: invalid buffer handle: %p",command->driver_handle);
				goto _skip_update_buffer_command;
			}
			virgl_opengl_buffer_t* buffer=buffer_handle->object;
			u32 size=command->size;
			if (command->storage_type!=OPENGL_PROTOCOL_BUFFER_STORAGE_TYPE_NO_CHANGE){
				buffer->storage_type=command->storage_type;
				if (buffer->size==size){
					goto _skip_buffer_resize;
				}
				if (!size){
					ERROR("_process_commands: unimplemented: delete buffer");
					goto _update_buffer_cleanup;
				}
				if (buffer->size){
					if (buffer->address){
						virtio_gpu_command_resource_detach_backing(ctx->gpu_device,buffer->resource_handle);
						buffer->address=0;
					}
					virtio_gpu_command_ctx_detach_resource(ctx->gpu_device,CONTEXT_ID,buffer->resource_handle);
					virtio_gpu_command_resource_unref(ctx->gpu_device,buffer->resource_handle);
				}
				if (size){
					buffer->resource_handle=virtio_gpu_command_resource_create_3d(ctx->gpu_device,buffer->resource_handle,VIRGL_TARGET_BUFFER,VIRGL_FORMAT_R8_UNORM,VIRGL_PROTOCOL_BIND_FLAG_VERTEX_BUFFER/*|VIRGL_PROTOCOL_BIND_FLAG_INDEX_BUFFER*/,size,1,1,1,0,0);
					virtio_gpu_command_ctx_attach_resource(ctx->gpu_device,CONTEXT_ID,buffer->resource_handle);
				}
				buffer->size=size;
			}
			if (command->offset>=buffer->size){
				size=0;
			}
			else if (command->offset+size>buffer->size){
				size=buffer->size-command->offset;
			}
_skip_buffer_resize:
			if (!size||!command->data){
				goto _update_buffer_cleanup;
			}
			// Verify user pointers!
			if (size<=MAX_INLINE_WRITE_SIZE){
				u32 virgl_resource_inline_write_command[12+MAX_INLINE_WRITE_SIZE/sizeof(u32)]={
					VIRGL_PROTOCOL_COMMAND_RESOURCE_INLINE_WRITE(size),
					buffer->resource_handle,
					0,
					0,
					0,
					0,
					command->offset,
					0,
					0,
					size,
					1,
					1,
				};
				memcpy(virgl_resource_inline_write_command+12,command->data,size);
				_command_buffer_extend(instance->ctx,virgl_resource_inline_write_command,12+(size+sizeof(u32)-1)/sizeof(u32),0);
				goto _update_buffer_cleanup;
			}
			if (!buffer->address){
				buffer->address=pmm_alloc(pmm_align_up_address(size)>>PAGE_SIZE_SHIFT,_virgl_opengl_buffer_pmm_counter,0);
				virtio_gpu_command_resource_attach_backing(ctx->gpu_device,buffer->resource_handle,buffer->address,buffer->size);
				virtio_gpu_box_t box={
					0,
					0,
					0,
					buffer->size,
					1,
					1
				};
				virtio_gpu_command_transfer_from_host_3d(ctx->gpu_device,buffer->resource_handle,&box,0,0,0);
			}
			memcpy((void*)(buffer->address+command->offset+VMM_HIGHER_HALF_ADDRESS_OFFSET),command->data,size);
			virtio_gpu_box_t box={
				command->offset,
				0,
				0,
				size,
				1,
				1
			};
			virtio_gpu_command_transfer_to_host_3d(ctx->gpu_device,buffer->resource_handle,&box,0,0,0);
			if (buffer->storage_type==OPENGL_PROTOCOL_BUFFER_STORAGE_TYPE_STATIC){
				virtio_gpu_command_resource_detach_backing(ctx->gpu_device,buffer->resource_handle);
				pmm_dealloc(buffer->address,pmm_align_up_address(buffer->size)>>PAGE_SIZE_SHIFT,_virgl_opengl_buffer_pmm_counter);
				buffer->address=0;
			}
_update_buffer_cleanup:
			handle_release(buffer_handle);
_skip_update_buffer_command:
		}
		else if (header->type==OPENGL_PROTOCOL_TYPE_SET_BUFFERS){
			opengl_protocol_set_buffers_t* command=(void*)header;
			if (command->vertex_buffer_count){
				u32 virgl_set_vertex_buffers_command[97]={
					VIRGL_PROTOCOL_COMMAND_SET_VERTEX_BUFFERS(command->vertex_buffer_count),
				};
				for (u32 i=0;i<command->vertex_buffer_count;i++){
					handle_t* handle=handle_lookup_and_acquire((command->vertex_buffers+i)->driver_handle,_virgl_opengl_buffer_handle_type);
					if ((command->vertex_buffers+i)->driver_handle&&!handle){
						ERROR("_process_commands: invalid vertex buffer handle: %p",(command->vertex_buffers+i)->driver_handle);
						goto _skip_set_vertex_buffer;
					}
					virgl_set_vertex_buffers_command[i*3+1]=(command->vertex_buffers+i)->stride;
					virgl_set_vertex_buffers_command[i*3+2]=(command->vertex_buffers+i)->offset;
					virgl_set_vertex_buffers_command[i*3+3]=(handle?((const virgl_opengl_buffer_t*)(handle->object))->resource_handle:0);
					if (handle){
						handle_release(handle);
					}
				}
				_command_buffer_extend(instance->ctx,virgl_set_vertex_buffers_command,1+command->vertex_buffer_count*3,0);
			}
_skip_set_vertex_buffer:
			if (command->index_buffer_driver_handle){
				handle_t* handle=handle_lookup_and_acquire(command->index_buffer_driver_handle,_virgl_opengl_buffer_handle_type);
				if (!handle){
					ERROR("_process_commands: invalid index buffer handle: %p",command->index_buffer_driver_handle);
					goto _skip_set_index_buffer;
				}
				virgl_opengl_buffer_t* buffer=handle->object;
				u32 virgl_set_index_buffer_command[4]={
					VIRGL_PROTOCOL_COMMAND_SET_INDEX_BUFFER,
					buffer->resource_handle,
					command->index_buffer_index_width,
					command->index_buffer_offset,
				};
				_command_buffer_extend(instance->ctx,virgl_set_index_buffer_command,4,0);
				handle_release(handle);
			}
_skip_set_index_buffer:
			if (command->uniform_buffer_data){
				u32 virgl_set_constant_buffer_command_header[3]={
					VIRGL_PROTOCOL_COMMAND_SET_CONSTANT_BUFFER(command->uniform_buffer_size),
					VIRGL_SHADER_VERTEX,
					0
				};
				_command_buffer_extend(instance->ctx,NULL,3+(command->uniform_buffer_size+sizeof(u32)-1)/sizeof(u32),0);
				_command_buffer_extend(instance->ctx,virgl_set_constant_buffer_command_header,3,0);
				_command_buffer_extend(instance->ctx,command->uniform_buffer_data,(command->uniform_buffer_size+sizeof(u32)-1)/sizeof(u32),0);
				virgl_set_constant_buffer_command_header[1]=VIRGL_SHADER_FRAGMENT;
				_command_buffer_extend(instance->ctx,NULL,3+(command->uniform_buffer_size+sizeof(u32)-1)/sizeof(u32),0);
				_command_buffer_extend(instance->ctx,virgl_set_constant_buffer_command_header,3,0);
				_command_buffer_extend(instance->ctx,command->uniform_buffer_data,(command->uniform_buffer_size+sizeof(u32)-1)/sizeof(u32),0);
			}
		}
		else if (header->type==OPENGL_PROTOCOL_TYPE_FLUSH){
			_command_buffer_extend(instance->ctx,NULL,0,1);
		}
		else if (header->type==OPENGL_PROTOCOL_TYPE_UPDATE_TEXTURE){
			opengl_protocol_update_texture_t* command=(void*)header;
			#define ELEMENT_SIZE 4
			u64 size=command->width*command->height*command->depth*ELEMENT_SIZE;
			if (command->data&&syscall_get_user_pointer_max_length(command->data)<size){
				ERROR("_process_commands: invalid user pointer");
				goto _skip_update_buffer_command;
			}
			if (!command->driver_handle){
				virgl_opengl_texture_t* texture=omm_alloc(_virgl_opengl_texture_allocator);
				handle_new(texture,_virgl_opengl_texture_handle_type,&(texture->handle));
				texture->resource_handle=0;
				handle_finish_setup(&(texture->handle));
				command->driver_handle=texture->handle.rb_node.key;
			}
			handle_t* texture_handle=handle_lookup_and_acquire(command->driver_handle,_virgl_opengl_texture_handle_type);
			if (!texture_handle){
				ERROR("_process_commands: invalid texture handle: %p",command->driver_handle);
				goto _skip_update_texture_command;
			}
			virgl_opengl_texture_t* texture=texture_handle->object;
			if (command->format!=OPENGL_PROTOCOL_BUFFER_TEXTURE_FORMAT_NONE){
				if (texture->resource_handle){
					virtio_gpu_command_ctx_detach_resource(ctx->gpu_device,CONTEXT_ID,texture->resource_handle);
					virtio_gpu_command_resource_unref(ctx->gpu_device,texture->resource_handle);
				}
				else{
					texture->resource_handle=resource_alloc(state_ctx->resource_manager);
				}
				texture->resource_handle=virtio_gpu_command_resource_create_3d(ctx->gpu_device,texture->resource_handle,VIRGL_TARGET_TEXTURE_2D,_virgl_texture_format_map[command->format],VIRGL_PROTOCOL_BIND_FLAG_RENDER_TARGET,command->width,command->height,command->depth,1,0,0);
				virtio_gpu_command_ctx_attach_resource(ctx->gpu_device,CONTEXT_ID,texture->resource_handle);
			}
			u32 AAAAAAAA[]={
				VIRGL_PROTOCOL_COMMAND_CREATE_OBJECT_SAMPLER_STATE,
				0xff8080ff,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				0,
				VIRGL_PROTOCOL_COMMAND_BIND_SAMPLER_STATES(1),
				VIRGL_SHADER_FRAGMENT,
				0,
				0xff8080ff,
			};
			_command_buffer_extend(instance->ctx,AAAAAAAA,sizeof(AAAAAAAA)/sizeof(u32),0);
			if (!command->data){
				goto _update_texture_cleanup;
			}
			if (size<=MAX_INLINE_WRITE_SIZE){
				u32 virgl_resource_inline_write_command[12+MAX_INLINE_WRITE_SIZE/sizeof(u32)]={
					VIRGL_PROTOCOL_COMMAND_RESOURCE_INLINE_WRITE(size),
					texture->resource_handle,
					command->level,
					0,
					0,
					0,
					command->x_offset,
					command->y_offset,
					command->z_offset,
					command->width,
					command->height,
					command->depth,
				};
				memcpy(virgl_resource_inline_write_command+12,command->data,size);
				_command_buffer_extend(instance->ctx,virgl_resource_inline_write_command,12+(size+sizeof(u32)-1)/sizeof(u32),0);
				goto _update_texture_cleanup;
			}
			ERROR("_process_commands: texture update via buffer");
_update_texture_cleanup:
			handle_release(texture_handle);
_skip_update_texture_command:
		}
		else{
			ERROR("_process_commands: unknown command: 0x%X",header->type);
		}
		offset+=header->length;
	}
}



static const opengl_driver_t _virgl_opengl_driver={
	"virtio_gpu_virgl",
	"libtgsiascii.so",
	330,
	_init_state,
	_deinit_state,
	_update_render_target,
	_process_commands
};



void virgl_init(void){
	_virgl_opengl_context_command_buffer_pmm_counter=pmm_alloc_counter("virgl_opengl_context_command_buffer");
	_virgl_opengl_buffer_pmm_counter=pmm_alloc_counter("virgl_opengl_buffer");
	_virgl_opengl_context_allocator=omm_init("virgl_opengl_context",sizeof(virgl_opengl_context_t),8,4,pmm_alloc_counter("omm_virgl_opengl_context"));
	spinlock_init(&(_virgl_opengl_context_allocator->lock));
	_virgl_opengl_state_context_allocator=omm_init("virgl_opengl_state_context",sizeof(virgl_opengl_state_context_t),8,4,pmm_alloc_counter("omm_virgl_opengl_state_context"));
	spinlock_init(&(_virgl_opengl_state_context_allocator->lock));
	_virgl_opengl_shader_allocator=omm_init("virgl_opengl_shader",sizeof(virgl_opengl_shader_t),8,4,pmm_alloc_counter("omm_virgl_opengl_shader"));
	spinlock_init(&(_virgl_opengl_shader_allocator->lock));
	_virgl_opengl_vertex_array_allocator=omm_init("virgl_opengl_vertex_array",sizeof(virgl_opengl_vertex_array_t),8,4,pmm_alloc_counter("omm_virgl_opengl_vertex_array"));
	spinlock_init(&(_virgl_opengl_vertex_array_allocator->lock));
	_virgl_opengl_buffer_allocator=omm_init("virgl_opengl_buffer",sizeof(virgl_opengl_buffer_t),8,4,pmm_alloc_counter("omm_virgl_opengl_buffer"));
	spinlock_init(&(_virgl_opengl_buffer_allocator->lock));
	_virgl_opengl_texture_allocator=omm_init("virgl_opengl_texture",sizeof(virgl_opengl_texture_t),8,4,pmm_alloc_counter("omm_virgl_opengl_texture"));
	spinlock_init(&(_virgl_opengl_texture_allocator->lock));
	_virgl_opengl_shader_handle_type=handle_alloc("virgl_opengl_shader",NULL);
	_virgl_opengl_vertex_array_handle_type=handle_alloc("virgl_opengl_vertex_array",NULL);
	_virgl_opengl_buffer_handle_type=handle_alloc("virgl_opengl_buffer",NULL);
	_virgl_opengl_texture_handle_type=handle_alloc("virgl_opengl_texture",NULL);
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
	ctx->command_buffer.buffer_address=pmm_alloc(pmm_align_up_address(VIRGL_OPENGL_CONTEXT_COMMAND_BUFFER_SIZE*sizeof(u32))>>PAGE_SIZE_SHIFT,_virgl_opengl_context_command_buffer_pmm_counter,0);
	ctx->command_buffer.buffer=(void*)(ctx->command_buffer.buffer_address+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	ctx->command_buffer.size=0;
	opengl_create_driver_instance(&_virgl_opengl_driver,caps->renderer,ctx);
}
