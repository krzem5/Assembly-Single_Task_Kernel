#ifndef _VIRTIO_FS_H_
#define _VIRTIO_FS_H_ 1
#include <fuse/fuse_registers.h>
#include <kernel/types.h>
#include <virtio/virtio.h>



typedef struct _VIRTIO_FS_DEVICE{
	virtio_device_t* device;
	virtio_queue_t* hiprioq;
	virtio_queue_t* loprioq;
} virtio_fs_device_t;



void virtio_fs_fuse_init(virtio_fs_device_t* fs_device);



fuse_getattr_out_t* virtio_fs_fuse_getattr(virtio_fs_device_t* fs_device,fuse_node_id_t fuse_node_id,fuse_file_handle_t fuse_file_handle);



fuse_file_handle_t virtio_fs_fuse_open(virtio_fs_device_t* fs_device,fuse_node_id_t fuse_node_id);



void virtio_fs_fuse_read(virtio_fs_device_t* fs_device,fuse_node_id_t fuse_node_id,fuse_file_handle_t fuse_file_handle,u64 offset,fuse_read_out_t* buffer,u32 buffer_size,u32 type);



fuse_lookup_out_t* virtio_fs_fuse_lookup(virtio_fs_device_t* fs_device,fuse_node_id_t fuse_node_id,const char* name,u32 name_length);



#endif
