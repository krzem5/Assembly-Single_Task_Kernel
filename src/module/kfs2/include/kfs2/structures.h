#ifndef _KFS2_STRUCTURES_H_
#define _KFS2_STRUCTURES_H_ 1
#include <kernel/types.h>



#define KFS2_BLOCK_SIZE 4096

#define KFS2_ROOT_BLOCK_SIGNATURE 0x544f4f523253464b
#define KFS2_BITMAP_LEVEL_COUNT 5

#define KFS2_INODE_GET_BLOCK_INDEX(inode) ((inode)/(KFS2_BLOCK_SIZE/sizeof(kfs2_node_t)))
#define KFS2_INODE_GET_NODE_INDEX(inode) ((inode)%(KFS2_BLOCK_SIZE/sizeof(kfs2_node_t)))

#define KFS2_INODE_TYPE_FILE 0x00000000
#define KFS2_INODE_TYPE_DIRECTORY 0x00000001
#define KFS2_INODE_TYPE_LINK 0x0000002
#define KFS2_INODE_TYPE_MASK 0x00000003

#define KFS2_INODE_STORAGE_MASK 0x0000001c
#define KFS2_INODE_STORAGE_TYPE_INLINE 0x00000000
#define KFS2_INODE_STORAGE_TYPE_SINGLE 0x00000004
#define KFS2_INODE_STORAGE_TYPE_DOUBLE 0x00000008
#define KFS2_INODE_STORAGE_TYPE_TRIPLE 0x0000000c
#define KFS2_INODE_STORAGE_TYPE_QUADRUPLE 0x00000010

#define KFS2_INODE_PERMISSION_MASK 0x00003fe0
#define KFS2_INODE_PERMISSION_SHIFT 5



typedef struct KERNEL_PACKED _KFS2_ROOT_BLOCK{
	u64 signature;
	u8 uuid[16];
	u8 master_key[64];
	u64 block_count;
	u64 inode_count;
	u64 data_block_count;
	u64 first_inode_block;
	u64 first_data_block;
	u64 inode_allocation_bitmap_offsets[KFS2_BITMAP_LEVEL_COUNT];
	u64 data_block_allocation_bitmap_offsets[KFS2_BITMAP_LEVEL_COUNT];
	u16 inode_allocation_bitmap_highest_level_length;
	u16 data_block_allocation_bitmap_highest_level_length;
	u32 kernel_inode;
	u32 initramfs_inode;
	u32 crc;
} kfs2_root_block_t;



typedef union KERNEL_PACKED _KFS2_NODE_DATA{
	u8 inline_[48];
	u64 single[6];
	u64 double_;
	u64 triple;
	u64 quadruple;
} kfs2_node_data_t;



typedef struct KERNEL_PACKED _KFS2_NODE{
	u64 size;
	kfs2_node_data_t data;
	u32 flags;
	u64 time_access;
	u64 time_modify;
	u64 time_change;
	u64 time_birth;
	u32 gid;
	u32 uid;
	u8 _padding[24];
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
