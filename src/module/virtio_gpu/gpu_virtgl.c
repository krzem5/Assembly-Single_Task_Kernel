#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "virtio_gpu_virtgl"



void virtio_gpu_virtgl_load_opengl_from_capset(_Bool is_v2,const void* data,u32 size){
	LOG("Initializing OpenGL with virtgl%s backend...",(is_v2?"2":""));
}
