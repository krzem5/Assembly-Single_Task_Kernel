#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/fs/partition.h>
#include <kernel/fs_provider/kfs.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



#define KFS_BLOCK_CACHE_NDA2_PRESENT 0x01
#define KFS_BLOCK_CACHE_NDA2_DIRTY 0x02
#define KFS_BLOCK_CACHE_NDA3_PRESENT 0x04
#define KFS_BLOCK_CACHE_NDA3_DIRTY 0x08
#define KFS_BLOCK_CACHE_BATC_PRESENT 0x10
#define KFS_BLOCK_CACHE_BATC_DIRTY 0x20
#define KFS_BLOCK_CACHE_ROOT_PRESENT 0x40
#define KFS_BLOCK_CACHE_ROOT_DIRTY 0x80

#define KFS_BATC_BLOCK_COUNT 257984



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
	char name[35];
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
	kfs_node_t nodes[64];
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



_Static_assert(sizeof(kfs_node_t)==64);
_Static_assert(sizeof(kfs_nda1_block_t)==4096);
_Static_assert(sizeof(kfs_nda2_block_t)==8192);
_Static_assert(sizeof(kfs_nda3_block_t)==4096);
_Static_assert(sizeof(kfs_batc_block_t)==32768);
_Static_assert(sizeof(kfs_root_block_t)==4096);



typedef struct _KFS_BLOCK_CACHE{
	kfs_nda3_block_t nda2;
	kfs_nda3_block_t nda3;
	kfs_batc_block_t batc;
	kfs_root_block_t root;
	u8 flags;
} kfs_block_cache_t;



typedef struct _KFS_FS_NODE{
	fs_node_t header;
	kfs_node_index_t index;
} kfs_fs_node_t;



static void _drive_write(const drive_t* drive,kfs_large_block_index_t offset,const void* buffer,kfs_large_block_index_t length){
	if (drive->read_write(drive->extra_data,(offset<<(12-drive->block_size_shift))|DRIVE_OFFSET_FLAG_WRITE,(void*)buffer,length<<(12-drive->block_size_shift))!=(length<<(12-drive->block_size_shift))){
		ERROR("Error writing data to drive");
	}
}



void kfs_load(const drive_t* drive,const fs_partition_config_t* partition_config){
	// kfs_block_cache_t* block_cache=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(sizeof(kfs_block_cache_t))));
	// block_cache->flags=0;
}



_Bool kfs_format_drive(const drive_t* drive,const void* boot,u32 boot_length){
	LOG("Formatting drive '%s' as KFS...",drive->model_number);
	if (drive->block_size>4096){
		WARN("KFS requires block_size to be less than or equal to 4096 bytes");
		return 0;
	}
	u64 block_count=drive->block_count>>(12-drive->block_size_shift);
	INFO("%lu total blocks, %lu BATC blocks",block_count,(block_count+KFS_BATC_BLOCK_COUNT-1)/KFS_BATC_BLOCK_COUNT);
	u64 first_free_block_index=2;
	if (boot_length){
		ERROR("Unimplemented: kfs_format_drive.boot");
	}
	kfs_root_block_t root={
		KFS_SIGNATURE,
		block_count,
		(block_count+KFS_BATC_BLOCK_COUNT-1)/KFS_BATC_BLOCK_COUNT,
		first_free_block_index
	};
	for (u16 i=0;i<256;i++){
		root.nda3[i]=0;
	}
	INFO("Writing ROOT block...");
	if (drive->read_write(drive->extra_data,1|DRIVE_OFFSET_FLAG_WRITE,&root,sizeof(kfs_root_block_t)>>drive->block_size_shift)!=(sizeof(kfs_root_block_t)>>drive->block_size_shift)){
		ERROR("Error writing data to drive");
		return 0;
	}
	INFO("Writing BATC blocks...");
	kfs_batc_block_t batc;
	batc.bitmap3=0x7fffffffffffffffull;
	for (u8 i=0;i<62;i++){
		batc.bitmap2[i]=0xffffffffffffffffull;
	}
	batc.bitmap2[62]=0x7fffffffffffffffull;
	for (u16 i=0;i<4031;i++){
		batc.bitmap1[i]=0xffffffffffffffffull;
	}
	for (u64 processed_block_count=0;processed_block_count<block_count;processed_block_count+=KFS_BATC_BLOCK_COUNT){
		batc.block_index=first_free_block_index;
		batc.first_block_index=processed_block_count;
		first_free_block_index+=8;
		if (processed_block_count+KFS_BATC_BLOCK_COUNT>block_count){
			batc.bitmap3=0;
			for (u8 i=0;i<63;i++){
				batc.bitmap2[i]=0;
			}
			for (u16 i=0;i<4031;i++){
				batc.bitmap1[i]=0;
			}
			for (u32 i=0;i<block_count-processed_block_count;i++){
				batc.bitmap1[i>>6]|=1ull<<(i&63);
			}
			for (u16 i=0;i<4030;i++){
				if (!batc.bitmap1[i]){
					break;
				}
				batc.bitmap2[i>>6]|=1ull<<(i&63);
			}
			for (u8 i=0;i<63;i++){
				if (!batc.bitmap2[i]){
					break;
				}
				batc.bitmap3|=1ull<<i;
			}
		}
		_drive_write(drive,batc.block_index,&batc,8);
	}
	LOG("Drive '%s' successfully formatted as KFS",drive->model_number);
	return 1;
}
