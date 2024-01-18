#ifndef _VIRTIO_FS_REGISTERS_H_
#define _VIRTIO_FS_REGISTERS_H_ 1
#include <kernel/types.h>



typedef struct KERNEL_PACKED _VIRTIO_FS_CONFIG{
	union KERNEL_PACKED{
		u32 raw_data[11];
		struct KERNEL_PACKED{
			char tag[36];
			u32 num_request_queues;
			u32 notify_buf_size;
		};
	};
} virtio_fs_config_t;



#endif
