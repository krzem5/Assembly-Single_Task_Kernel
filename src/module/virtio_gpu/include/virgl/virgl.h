#ifndef _VIRGL_VIRGL_H_
#define _VIRGL_VIRGL_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/resource/resource.h>
#include <kernel/types.h>
#include <virgl/registers.h>
#include <virtio/gpu.h>
#include <virtio/gpu_registers.h>



#define VIRGL_OPENGL_CONTEXT_COMMAND_BUFFER_SIZE 0x100000



typedef u32 virgl_resource_t;



typedef struct _VIRGL_OPENGL_CONTEXT_COMMAND_BUFFER{
	spinlock_t lock;
	u32* buffer;
	u64 buffer_address;
	u32 size;
} virgl_opengl_context_buffer_t;



typedef struct _VIRGL_OPENGL_CONTEXT{
	virtio_gpu_device_t* gpu_device;
	virgl_caps_v2_t caps;
	virgl_opengl_context_buffer_t command_buffer;
} virgl_opengl_context_t;



typedef struct _VIRGL_OPENGL_STATE_CONTEXT{
	resource_manager_t* resource_manager;
} virgl_opengl_state_context_t;



typedef struct _VIRGL_OPENGL_SHADER{
	handle_t handle;
	resource_t vertex_shader;
	resource_t fragment_shader;
} virgl_opengl_shader_t;



typedef struct _VIRGL_OPENGL_VERTEX_ARRAY{
	handle_t handle;
	resource_t resource_handle;
} virgl_opengl_vertex_array_t;



typedef struct _VIRGL_OPENGL_VERTEX_ARRAY_ELEMENT_TYPE{
	u32 virgl_type;
	u8 size;
	u8 type;
	bool require_normalization;
} virgl_opengl_vertex_array_element_type_t;



typedef struct _VIRGL_OPENGL_BUFFER{
	handle_t handle;
	resource_t resource_handle;
	u32 storage_type;
	u64 address;
	u64 size;
} virgl_opengl_buffer_t;



typedef struct _VIRGL_OPENGL_TEXTURE{
	handle_t handle;
	resource_t resource_handle;
	u32 format;
} virgl_opengl_texture_t;



typedef struct _VIRGL_OPENGL_SAMPLER{
	handle_t handle;
	resource_t view_resource_handle;
	resource_t state_resource_handle;
} virgl_opengl_sampler_t;



void virgl_load_from_virtio_gpu_capset(virtio_gpu_device_t* gpu_device,bool is_v2,const void* data,u32 size);



#endif
