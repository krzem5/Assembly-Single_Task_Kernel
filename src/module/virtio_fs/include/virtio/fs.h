#ifndef _VIRTIO_FS_H_
#define _VIRTIO_FS_H_ 1
#include <kernel/resource/resource.h>
#include <kernel/types.h>
#include <virtio/fs_registers.h>
#include <virtio/virtio.h>



typedef struct _VIRTIO_FS_DEVICE{
	virtio_device_t* device;
	virtio_queue_t* hiprioq;
	virtio_queue_t* loprioq;
} virtio_fs_device_t;



void virtio_fs_init(void);



#endif
