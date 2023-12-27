#ifndef _VIRGL_VIRTIO_GPU_H_
#define _VIRGL_VIRTIO_GPU_H_ 1
#include <kernel/types.h>
#include <virtio/gpu.h>



void virgl_load_from_virtio_gpu_capset(virtio_gpu_device_t* gpu_device,_Bool is_v2,const void* data,u32 size);



#endif
