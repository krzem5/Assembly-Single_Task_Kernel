#ifndef _FUSE_FUSE_H_
#define _FUSE_FUSE_H_ 1
#include <kernel/fs/fs.h>
#include <kernel/types.h>
#include <virtio/fs.h>



filesystem_t* fuse_create_filesystem(virtio_fs_device_t* fs_device);



#endif
