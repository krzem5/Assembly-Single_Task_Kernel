#ifndef _VIRGL_VIRGL_H_
#define _VIRGL_VIRGL_H_ 1
#include <kernel/types.h>
#include <virgl/registers.h>
#include <virtio/gpu.h>



typedef struct _VIRGL_OPENGL_CONTEXT{
	virtio_gpu_device_t* gpu_device;
	virgl_caps_v2_t caps;
} virgl_opengl_context_t;



void virgl_init(void);



void virgl_load_from_virtio_gpu_capset(virtio_gpu_device_t* gpu_device,_Bool is_v2,const void* data,u32 size);



#endif
