#ifndef _VIRTIO_FS_REGISTERS_H_
#define _VIRTIO_FS_REGISTERS_H_ 1
#include <kernel/types.h>



#define FUSE_VERSION_MAJOR 7
#define FUSE_VERSION_MINOR 38

#define FUSE_INIT 26



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



typedef struct KERNEL_PACKED _FUSE_IN_HEADER{
	u32 len;
	u32 opcode;
	u64 unique;
	u64 nodeid;
	u32 uid;
	u32 gid;
	u32 pid;
	u16 total_extlen;
	u16 padding;
} fuse_in_header_t;



typedef struct KERNEL_PACKED _FUSE_OUT_HEADER{
	u32 len;
	s32 error;
	u64 unique;
} fuse_out_header_t;



typedef struct KERNEL_PACKED _FUSE_INIT_IN{
	fuse_in_header_t header;
	u32 major;
	u32 minor;
	u32 max_readahead;
	u32 flags;
	u32 flags2;
	u32 unused[11];
} fuse_init_in_t;



typedef struct KERNEL_PACKED _FUSE_INIT_OUT{
	fuse_out_header_t header;
	u32 major;
	u32 minor;
	u32 max_readahead;
	u32 flags;
	u16 max_background;
	u16 congestion_threshold;
	u32 max_write;
	u32 time_gran;
	u16 max_pages;
	u16 map_alignment;
	u32 flags2;
	u32 unused[7];
} fuse_init_out_t;



#endif
