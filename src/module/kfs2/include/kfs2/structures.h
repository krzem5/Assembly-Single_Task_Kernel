#ifndef _KFS2_STRUCTURES_H_
#define _KFS2_STRUCTURES_H_ 1
#include <kernel/types.h>



#define KFS2_BLOCK_SIZE 4096

#define KFS2_ROOT_BLOCK_SIGNATURE 0x544f4f523253464b
#define KFS2_BITMAP_LEVEL_COUNT 5

#define KFS2_INODE_GET_BLOCK_INDEX(inode) ((inode)/(KFS2_BLOCK_SIZE/sizeof(kfs2_node_t)))
#define KFS2_INODE_GET_NODE_INDEX(inode) ((inode)%(KFS2_BLOCK_SIZE/sizeof(kfs2_node_t)))

#define KFS2_INODE_TYPE_FILE 0x0000
#define KFS2_INODE_TYPE_DIRECTORY 0x0001
#define KFS2_INODE_TYPE_LINK 0x0002
#define KFS2_INODE_TYPE_MASK 0x0003

#define KFS2_INODE_STORAGE_MASK 0x001c
#define KFS2_INODE_STORAGE_TYPE_INLINE 0x0000
#define KFS2_INODE_STORAGE_TYPE_SINGLE 0x0004
#define KFS2_INODE_STORAGE_TYPE_DOUBLE 0x0008
#define KFS2_INODE_STORAGE_TYPE_TRIPLE 0x000c
#define KFS2_INODE_STORAGE_TYPE_QUADRUPLE 0x0010



typedef struct KERNEL_PACKED _KFS2_ROOT_BLOCK{
	u64 signature;
	u8 uuid[16];
	u64 block_count;
	u64 inode_count;
	u64 data_block_count;
	u64 first_inode_block;
	u64 first_data_block;
	u64 first_bitmap_block;
	u64 inode_allocation_bitmap_offsets[KFS2_BITMAP_LEVEL_COUNT];
	u64 data_block_allocation_bitmap_offsets[KFS2_BITMAP_LEVEL_COUNT];
	u16 inode_allocation_bitmap_highest_level_length;
	u16 data_block_allocation_bitmap_highest_level_length;
	u32 kernel_inode;
	u32 initramfs_inode;
	u32 crc;
} kfs2_root_block_t;



typedef struct KERNEL_PACKED _KFS2_NODE{
	u64 size;
	union{
		u8 inline_[48];
		u64 single[6];
		u64 double_;
		u64 triple;
		u64 quadruple;
	} data;
	u32 flags;
	union{
		u32 crc;
		u32 _inode;
	};
} kfs2_node_t;



typedef struct KERNEL_PACKED _KFS2_DIRECTORY_ENTRY{
	u32 inode;
	u16 size;
	u8 name_length;
	u8 name_compressed_hash;
	char name[];
} kfs2_directory_entry_t;



typedef struct _KFS2_DATA_CHUNK{
	u64 offset;
	u64* quadruple_cache;
	u64* triple_cache;
	u64* double_cache;
	void* data;
	u16 length;
} kfs2_data_chunk_t;



#endif
