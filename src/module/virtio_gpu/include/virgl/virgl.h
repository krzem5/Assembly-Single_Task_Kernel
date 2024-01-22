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
	virtio_gpu_resource_id_t framebuffer_resource_id;
} virgl_opengl_state_context_t;



typedef struct _VIRGL_OPENGL_SHADER{
	handle_t handle;
	resource_t vertex_shader;
	resource_t fragment_shader;
} virgl_opengl_shader_t;



void virgl_init(void);



void virgl_load_from_virtio_gpu_capset(virtio_gpu_device_t* gpu_device,_Bool is_v2,const void* data,u32 size);



#endif
