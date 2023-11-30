#ifndef _UEFI_KFS2_H_
#define _UEFI_KFS2_H_ 1
#include <stdint.h>



#define KFS2_BLOCK_SIZE 4096

#define KFS2_ROOT_BLOCK_SIGNATURE 0x544f4f523253464b
#define KFS2_BITMAP_LEVEL_COUNT 5

#define KFS2_INODE_GET_BLOCK_INDEX(inode) ((inode)/(KFS2_BLOCK_SIZE/sizeof(kfs2_node_t)))
#define KFS2_INODE_GET_NODE_INDEX(inode) ((inode)%(KFS2_BLOCK_SIZE/sizeof(kfs2_node_t)))

#define KFS2_INODE_TYPE_FILE 0x00000000
#define KFS2_INODE_TYPE_DIRECTORY 0x00000001
#define KFS2_INODE_TYPE_LINK 0x00000002
#define KFS2_INODE_TYPE_MASK 0x00000003

#define KFS2_INODE_STORAGE_MASK 0x0000001c
#define KFS2_INODE_STORAGE_TYPE_INLINE 0x00000000
#define KFS2_INODE_STORAGE_TYPE_SINGLE 0x00000004
#define KFS2_INODE_STORAGE_TYPE_DOUBLE 0x00000008
#define KFS2_INODE_STORAGE_TYPE_TRIPLE 0x0000000c
#define KFS2_INODE_STORAGE_TYPE_QUADRUPLE 0x00000010



typedef struct __attribute__((packed)) _KFS2_ROOT_BLOCK{
	uint64_t signature;
	uint8_t uuid[16];
	uint64_t block_count;
	uint64_t inode_count;
	uint64_t data_block_count;
	uint64_t first_inode_block;
	uint64_t first_data_block;
	uint64_t first_bitmap_block;
	uint64_t inode_allocation_bitmap_offsets[KFS2_BITMAP_LEVEL_COUNT];
	uint64_t data_block_allocation_bitmap_offsets[KFS2_BITMAP_LEVEL_COUNT];
	uint16_t inode_allocation_bitmap_highest_level_length;
	uint16_t data_block_allocation_bitmap_highest_level_length;
	uint32_t kernel_inode;
	uint32_t initramfs_inode;
	uint32_t crc;
} kfs2_root_block_t;



typedef struct __attribute__((packed)) _KFS2_NODE{
	uint64_t size;
	union{
		uint8_t inline_[48];
		uint64_t single[6];
		uint64_t double_;
		uint64_t triple;
		uint64_t quadruple;
	} data;
	uint32_t flags;
	uint8_t _padding[64];
	uint32_t crc;
} kfs2_node_t;



#endif
