#ifndef _VIRTIO_BLK_REGISTERS_H_
#define _VIRTIO_BLK_REGISTERS_H_ 1
#include <kernel/types.h>



// Features
#define VIRTIO_BLK_F_SIZE_MAX 1
#define VIRTIO_BLK_F_SEG_MAX 2
#define VIRTIO_BLK_F_BLK_SIZE 6

// Registers
#define VIRTIO_BLK_REG_CAPACITY 0x00
#define VIRTIO_BLK_REG_BLK_SIZE 0x14

// Request types
#define VIRTIO_BLK_T_IN 0
#define VIRTIO_BLK_T_OUT 1

// Status codes
#define VIRTIO_BLK_S_OK 0



typedef struct KERNEL_PACKED _VIRTIO_BLK_REQUEST_HEADER{
	u32 type;
	u32 _zero;
	u64 sector;
} virtio_blk_request_header_t;



#endif
