#ifndef _VIRTIO_FS_REGISTERS_H_
#define _VIRTIO_FS_REGISTERS_H_ 1
#include <kernel/types.h>



#define FUSE_VERSION_MAJOR 7
#define FUSE_VERSION_MINOR 38

#define FUSE_OPCODE_LOOKUP 1
#define FUSE_OPCODE_FORGET 2
#define FUSE_OPCODE_GETATTR 3
#define FUSE_OPCODE_SETATTR 4
#define FUSE_OPCODE_READLINK 5
#define FUSE_OPCODE_SYMLINK 6
#define FUSE_OPCODE_MKNOD 8
#define FUSE_OPCODE_MKDIR 9
#define FUSE_OPCODE_UNLINK 10
#define FUSE_OPCODE_RMDIR 11
#define FUSE_OPCODE_RENAME 12
#define FUSE_OPCODE_LINK 13
#define FUSE_OPCODE_OPEN 14
#define FUSE_OPCODE_READ 15
#define FUSE_OPCODE_WRITE 16
#define FUSE_OPCODE_STATFS 17
#define FUSE_OPCODE_RELEASE 18
#define FUSE_OPCODE_FSYNC 20
#define FUSE_OPCODE_SETXATTR 21
#define FUSE_OPCODE_GETXATTR 22
#define FUSE_OPCODE_LISTXATTR 23
#define FUSE_OPCODE_REMOVEXATTR 24
#define FUSE_OPCODE_FLUSH 25
#define FUSE_OPCODE_INIT 26
#define FUSE_OPCODE_OPENDIR 27
#define FUSE_OPCODE_READDIR 28
#define FUSE_OPCODE_RELEASEDIR 29
#define FUSE_OPCODE_FSYNCDIR 30
#define FUSE_OPCODE_GETLK 31
#define FUSE_OPCODE_SETLK 32
#define FUSE_OPCODE_SETLKW 33
#define FUSE_OPCODE_ACCESS 34
#define FUSE_OPCODE_CREATE 35
#define FUSE_OPCODE_INTERRUPT 36
#define FUSE_OPCODE_BMAP 37
#define FUSE_OPCODE_DESTROY 38
#define FUSE_OPCODE_IOCTL 39
#define FUSE_OPCODE_POLL 40
#define FUSE_OPCODE_NOTIFY_REPLY 41
#define FUSE_OPCODE_BATCH_FORGET 42
#define FUSE_OPCODE_FALLOCATE 43
#define FUSE_OPCODE_READDIRPLUS 44
#define FUSE_OPCODE_RENAME2 45
#define FUSE_OPCODE_LSEEK 46
#define FUSE_OPCODE_COPY_FILE_RANGE 47
#define FUSE_OPCODE_SETUPMAPPING 48
#define FUSE_OPCODE_REMOVEMAPPING 49
#define FUSE_OPCODE_SYNCFS 50
#define FUSE_OPCODE_TMPFILE 51
#define FUSE_OPCODE_STATX 52

#define FUSE_ROOT_ID 1



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
