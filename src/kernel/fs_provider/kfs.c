#include <kernel/drive/drive.h>
#include <kernel/fs/partition.h>
#include <kernel/log/log.h>
#include <kernel/types.h>



typedef u8 kfs_node_flags_t;



typedef u32 kfs_node_index_t;



typedef u32 kfs_large_block_index_t; // 4096-aligned



typedef struct _KFS_NODE{
	kfs_large_block_index_t block_index;
	kfs_node_index_t index;
	kfs_node_index_t parent;
	kfs_node_index_t prev_sibling;
	kfs_node_index_t next_sibling;
	kfs_node_flags_t flags;
	char name[99];
	union{
		struct{
			kfs_large_block_index_t data_block_index;
			kfs_large_block_index_t data_block_count;
		} file;
		struct{
			kfs_node_index_t first_child;
		} directory;
	} data;
} kfs_node_t;



typedef struct _KFS_NDA1_BLOCK{
	kfs_node_t nodes[32];
} kfs_nda1_block_t;



typedef struct _KFS_NDA2_BLOCK{
	kfs_large_block_index_t block_index[2];
	kfs_node_index_t node_index;
	u8 bitmap3;
	u8 _padding[19];
	u64 bitmap2[8];
	u64 bitmap1[506];
	kfs_large_block_index_t nda1[1012];
} kfs_nda2_block_t;



typedef struct _KFS_NDA3_BLOCK{
	kfs_large_block_index_t block_index;
	kfs_node_index_t node_index;
	kfs_large_block_index_t nda2[1022];
} kfs_nda3_block_t;



typedef struct _KFS_BATC_BLOCK{
	kfs_large_block_index_t block_index;
	kfs_large_block_index_t first_block_index;
	u64 bitmap3;
	u64 bitmap2[63];
	u64 bitmap1[4031];
} kfs_batc_block_t;



typedef struct _KFS_ROOT_BLOCK{
	u64 signature;
	kfs_large_block_index_t block_count;
	kfs_large_block_index_t batc_block_index;
	kfs_large_block_index_t batc_block_count;
	kfs_large_block_index_t nda3[256];
	u8 _padding[3052];
} kfs_root_block_t;



_Static_assert(sizeof(kfs_node_t)==128);
_Static_assert(sizeof(kfs_nda1_block_t)==4096);
_Static_assert(sizeof(kfs_nda2_block_t)==8192);
_Static_assert(sizeof(kfs_nda3_block_t)==4096);
_Static_assert(sizeof(kfs_batc_block_t)==32768);
_Static_assert(sizeof(kfs_root_block_t)==4096);



void kfs_load(drive_t* drive,const fs_partition_config_t* partition_config){
	//
}



_Bool kfs_format_drive(const drive_t* drive,const void* boot,u32 boot_length){
	LOG("Formatting drive '%s' as KFS...",drive->model_number);
	if (drive->block_size!=512){
		WARN("KFS requires block_size to be equal to 512 bytes");
		return 0;
	}
	return 1;
}
