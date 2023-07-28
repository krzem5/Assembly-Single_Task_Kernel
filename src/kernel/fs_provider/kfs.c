#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/fs/partition.h>
#include <kernel/fs_provider/kfs.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



#define KFS_BLOCK_CACHE_NDA1_PRESENT 0x001
#define KFS_BLOCK_CACHE_NDA1_DIRTY 0x002
#define KFS_BLOCK_CACHE_NDA2_PRESENT 0x004
#define KFS_BLOCK_CACHE_NDA2_DIRTY 0x008
#define KFS_BLOCK_CACHE_NDA3_PRESENT 0x010
#define KFS_BLOCK_CACHE_NDA3_DIRTY 0x020
#define KFS_BLOCK_CACHE_BATC_PRESENT 0x040
#define KFS_BLOCK_CACHE_BATC_DIRTY 0x080
#define KFS_BLOCK_CACHE_ROOT_PRESENT 0x100
#define KFS_BLOCK_CACHE_ROOT_DIRTY 0x200

#define KFS_BATC_BLOCK_COUNT 257984

#define KFS_NODE_FLAG_DIRECTORY 0x80



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
	u8 _padding[3];
	u64 bitmap2[8];
	u64 bitmap1[507];
	kfs_large_block_index_t nda1[1014];
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
	kfs_large_block_index_t nda3[64];
	u8 _padding[3820];
} kfs_root_block_t;



_Static_assert(sizeof(kfs_node_t)==64);
_Static_assert(sizeof(kfs_nda1_block_t)==4096);
_Static_assert(sizeof(kfs_nda2_block_t)==8192);
_Static_assert(sizeof(kfs_nda3_block_t)==4096);
_Static_assert(sizeof(kfs_batc_block_t)==32768);
_Static_assert(sizeof(kfs_root_block_t)==4096);



typedef struct _KFS_BLOCK_CACHE{
	kfs_large_block_index_t nda1_block_index;
	kfs_nda1_block_t nda1;
	kfs_nda2_block_t nda2;
	kfs_nda3_block_t nda3;
	kfs_batc_block_t batc;
	kfs_root_block_t root;
	u16 flags;
	const drive_t* drive;
} kfs_block_cache_t;



typedef struct _KFS_FS_NODE{
	fs_node_t header;
	kfs_node_index_t index;
} kfs_fs_node_t;



static void _drive_read(const drive_t* drive,kfs_large_block_index_t offset,void* buffer,kfs_large_block_index_t length){
	if (drive->read_write(drive->extra_data,(offset<<(12-drive->block_size_shift)),(void*)buffer,length<<(12-drive->block_size_shift))!=(length<<(12-drive->block_size_shift))){
		ERROR("Error reading data from drive");
	}
}



static void _drive_write(const drive_t* drive,kfs_large_block_index_t offset,const void* buffer,kfs_large_block_index_t length){
	if (drive->read_write(drive->extra_data,(offset<<(12-drive->block_size_shift))|DRIVE_OFFSET_FLAG_WRITE,(void*)buffer,length<<(12-drive->block_size_shift))!=(length<<(12-drive->block_size_shift))){
		ERROR("Error writing data to drive");
	}
}



static inline void* _nda2_block_get_high_part(kfs_nda2_block_t* nda2){
	return (void*)(((u64)nda2)+4096);
}



static inline void _block_cache_flush_nda1(kfs_block_cache_t* block_cache){
	if (!(block_cache->flags&KFS_BLOCK_CACHE_NDA1_DIRTY)){
		return;
	}
	block_cache->flags&=~KFS_BLOCK_CACHE_NDA1_DIRTY;
	_drive_write(block_cache->drive,block_cache->nda1_block_index,&(block_cache->nda1),1);
}



static inline void _block_cache_flush_nda2(kfs_block_cache_t* block_cache){
	if (!(block_cache->flags&KFS_BLOCK_CACHE_NDA2_DIRTY)){
		return;
	}
	block_cache->flags&=~KFS_BLOCK_CACHE_NDA2_DIRTY;
	_drive_write(block_cache->drive,block_cache->nda2.block_index[0],&(block_cache->nda2),1);
	_drive_write(block_cache->drive,block_cache->nda2.block_index[1],_nda2_block_get_high_part(&(block_cache->nda2)),1);
}



static inline void _block_cache_flush_nda3(kfs_block_cache_t* block_cache){
	if (!(block_cache->flags&KFS_BLOCK_CACHE_NDA3_DIRTY)){
		return;
	}
	block_cache->flags&=~KFS_BLOCK_CACHE_NDA3_DIRTY;
	_drive_write(block_cache->drive,block_cache->nda3.block_index,&(block_cache->nda3),1);
}



static inline void _block_cache_flush_batc(kfs_block_cache_t* block_cache){
	if (!(block_cache->flags&KFS_BLOCK_CACHE_BATC_DIRTY)){
		return;
	}
	block_cache->flags&=~KFS_BLOCK_CACHE_BATC_DIRTY;
	_drive_write(block_cache->drive,block_cache->batc.block_index,&(block_cache->batc),8);
}



static inline void _block_cache_flush_root(kfs_block_cache_t* block_cache){
	if (!(block_cache->flags&KFS_BLOCK_CACHE_ROOT_DIRTY)){
		return;
	}
	block_cache->flags&=~KFS_BLOCK_CACHE_ROOT_DIRTY;
	if (block_cache->drive->read_write(block_cache->drive->extra_data,1|DRIVE_OFFSET_FLAG_WRITE,&(block_cache->root),sizeof(kfs_root_block_t)>>block_cache->drive->block_size_shift)!=(sizeof(kfs_root_block_t)>>block_cache->drive->block_size_shift)){
		ERROR("Error writing data to drive");
	}
}



static kfs_large_block_index_t _block_cache_alloc_block(kfs_block_cache_t* block_cache){
	if (block_cache->batc.bitmap3){
		goto _batc_found;
	}
	_block_cache_flush_batc(block_cache);
	kfs_large_block_index_t block_index=block_cache->root.batc_block_index;
	do{
		_drive_read(block_cache->drive,block_index,&(block_cache->batc),8);
		block_index+=8;
	} while (!block_cache->batc.bitmap3);
_batc_found:
	block_cache->flags|=KFS_BLOCK_CACHE_BATC_DIRTY;
	kfs_large_block_index_t i=__builtin_ctzll(block_cache->batc.bitmap3);
	kfs_large_block_index_t j=__builtin_ctzll(block_cache->batc.bitmap2[i])|(i<<6);
	kfs_large_block_index_t k=__builtin_ctzll(block_cache->batc.bitmap1[j])|(j<<6);
	block_cache->batc.bitmap1[j]&=block_cache->batc.bitmap1[j]-1;
	if (!block_cache->batc.bitmap1[j]){
		block_cache->batc.bitmap2[i]&=block_cache->batc.bitmap2[i]-1;
		if (!block_cache->batc.bitmap2[i]){
			block_cache->batc.bitmap3&=block_cache->batc.bitmap3-1;
		}
	}
	return block_cache->batc.first_block_index+k;
}



// static void _dealloc_block(block_index_t block_index){
// 	if (block_index>=_block_cache.batc_block.first_block_index&&block_index<_block_cache.batc_block.first_block_index+257920){
// 		goto _skip_descriptor_table_load;
// 	}
// 	_flush_batc_block_cache();
// 	disk_read_blocks(block_index/257920*8+1,8,&(_block_cache.batc_block));
// _skip_descriptor_table_load:
// 	_block_cache.batc_block_has_changes=1;
// 	block_index-=_block_cache.batc_block.first_block_index;
// 	_block_cache.batc_block.bitmap1[block_index>>6]|=1ull<<(block_index&63);
// 	_block_cache.batc_block.bitmap2[block_index>>12]|=1ull<<((block_index>>6)&63);
// 	_block_cache.batc_block.bitmap3|=1ull<<(block_index>>12);
// }



static kfs_large_block_index_t _block_cache_init_nda2(kfs_block_cache_t* block_cache,kfs_node_index_t node_index){
	block_cache->nda2.block_index[0]=_block_cache_alloc_block(block_cache);
	block_cache->nda2.block_index[1]=_block_cache_alloc_block(block_cache);
	block_cache->nda2.node_index=node_index;
	block_cache->nda2.bitmap3=0xff;
	for (u8 i=0;i<7;i++){
		block_cache->nda2.bitmap2[i]=0xffffffffffffffffull;
	}
	block_cache->nda2.bitmap2[7]=0x07ffffffffffffffull;
	for (u16 i=0;i<507;i++){
		block_cache->nda2.bitmap1[i]=0xffffffffffffffffull;
	}
	for (u16 i=0;i<1014;i++){
		block_cache->nda2.nda1[i]=0;
	}
	block_cache->flags|=KFS_BLOCK_CACHE_NDA2_PRESENT|KFS_BLOCK_CACHE_NDA2_DIRTY;
	return block_cache->nda2.block_index[0];
}



static kfs_large_block_index_t _block_cache_init_nda3(kfs_block_cache_t* block_cache,kfs_node_index_t node_index){
	block_cache->nda3.block_index=_block_cache_alloc_block(block_cache);
	block_cache->nda3.node_index=node_index;
	block_cache->nda3.nda2[0]=_block_cache_init_nda2(block_cache,node_index<<10);
	for (unsigned int i=1;i<1022;i++){
		block_cache->nda3.nda2[i]=0;
	}
	block_cache->flags|=KFS_BLOCK_CACHE_NDA3_PRESENT|KFS_BLOCK_CACHE_NDA3_DIRTY;
	return block_cache->nda3.block_index;
}



static kfs_node_t* _get_node_by_index(kfs_block_cache_t* block_cache,kfs_node_index_t index){
	return NULL;
}



static void _node_to_fs_node(kfs_node_t* node,kfs_fs_node_t* out){
	if (!node){
		WARN("_node_to_fs_node called with a NULL-node");
		return;
	}
	out->index=node->index;
}



static _Bool _nda3_find_free_nda2(kfs_block_cache_t* block_cache,kfs_large_block_index_t checked_nda2_index){
	u16 free_nda2_index=0xffff;
	for (u16 i=0;i<1022;i++){
		kfs_large_block_index_t block_index=block_cache->nda3.nda2[i];
		if (!block_index){
			if (free_nda2_index==0xffff){
				free_nda2_index=i;
			}
			continue;
		}
		if (block_index==checked_nda2_index){
			continue;
		}
		_drive_read(block_cache->drive,block_index,&(block_cache->nda2),1);
		if (block_cache->nda2.bitmap3){
			_drive_read(block_cache->drive,block_cache->nda2.block_index[1],_nda2_block_get_high_part(&(block_cache->nda2)),1);
			return 1;
		}
	}
	if (free_nda2_index==0xffff){
		return 0;
	}
	block_cache->nda3.nda2[free_nda2_index]=_block_cache_init_nda2(block_cache,(block_cache->nda3.node_index<<10)|free_nda2_index);
	block_cache->flags|=KFS_BLOCK_CACHE_NDA3_DIRTY;
	return 1;
}



static kfs_node_t* _alloc_node(kfs_block_cache_t* block_cache,kfs_node_flags_t type,const char* name,u8 name_length){
	kfs_large_block_index_t checked_nda2_index=0;
	if (!(block_cache->flags&KFS_BLOCK_CACHE_NDA2_PRESENT)){
		goto _nda2_empty;
	}
	if (block_cache->nda2.bitmap3){
		goto _nda2_found;
	}
	checked_nda2_index=block_cache->nda2.block_index[0];
	_block_cache_flush_nda2(block_cache);
_nda2_empty:
	kfs_large_block_index_t checked_nda3_index=0;
	if (!(block_cache->flags&KFS_BLOCK_CACHE_NDA3_PRESENT)){
		goto _nda3_empty;
	}
	if (_nda3_find_free_nda2(block_cache,checked_nda2_index)){
		goto _nda2_found;
	}
	checked_nda3_index=block_cache->nda3.block_index;
	_block_cache_flush_nda3(block_cache);
_nda3_empty:
	u8 free_nda3_index=0xff;
	for (u8 i=0;i<64;i++){
		kfs_large_block_index_t nda3_block_index=block_cache->root.nda3[i];
		if (!nda3_block_index){
			if (free_nda3_index==0xff){
				free_nda3_index=i;
			}
			continue;
		}
		if (i==checked_nda3_index){
			continue;
		}
		_drive_read(block_cache->drive,nda3_block_index,&(block_cache->nda3),1);
		if (_nda3_find_free_nda2(block_cache,0)){
			goto _nda2_found;
		}
	}
	if (free_nda3_index==0xff){
		return NULL;
	}
	block_cache->root.nda3[free_nda3_index]=_block_cache_init_nda3(block_cache,free_nda3_index);
	block_cache->flags|=KFS_BLOCK_CACHE_ROOT_DIRTY;
_nda2_found:
	ERROR("Unimplemented");
	return NULL;
}



static fs_node_t* _kfs_get_relative(fs_file_system_t* fs,fs_node_t* node,u8 relative){
	return NULL;
}



static _Bool _kfs_set_relative(fs_file_system_t* fs,fs_node_t* node,u8 relative,fs_node_t* other){
	return 0;
}



static u64 _kfs_read(fs_file_system_t* fs,fs_node_t* node,u64 offset,u8* buffer,u64 count){
	return 0;
}



static u64 _kfs_write(fs_file_system_t* fs,fs_node_t* node,u64 offset,const u8* buffer,u64 count){
	return 0;
}



static u64 _kfs_get_size(fs_file_system_t* fs,fs_node_t* node){
	return 0;
}



static const fs_file_system_config_t _kfs_fs_config={
	sizeof(kfs_fs_node_t),
	_kfs_get_relative,
	_kfs_set_relative,
	_kfs_read,
	_kfs_write,
	_kfs_get_size
};



void kfs_load(const drive_t* drive,const fs_partition_config_t* partition_config){
	LOG("Loading KFS file system from drive '%s'...",drive->model_number);
	INFO("Allocating block cache...");
	kfs_block_cache_t* block_cache=VMM_TRANSLATE_ADDRESS(pmm_alloc(pmm_align_up_address(sizeof(kfs_block_cache_t))));
	block_cache->flags=0;
	block_cache->drive=drive;
	INFO("Reading ROOT block...");
	if (drive->read_write(drive->extra_data,1,&(block_cache->root),sizeof(kfs_root_block_t)>>drive->block_size_shift)!=(sizeof(kfs_root_block_t)>>drive->block_size_shift)){
		ERROR("Error reading ROOT block from drive");
		return;
	}
	block_cache->flags|=KFS_BLOCK_CACHE_ROOT_PRESENT;
	kfs_fs_node_t* root=fs_create_file_system(drive,partition_config,&_kfs_fs_config,block_cache);
	kfs_node_t* kfs_root=_get_node_by_index(block_cache,0);
	if (!kfs_root){
		kfs_root=_alloc_node(block_cache,KFS_NODE_FLAG_DIRECTORY,"",0);
	}
	_node_to_fs_node(kfs_root,root);
}



_Bool kfs_format_drive(const drive_t* drive,const void* boot,u32 boot_length){
	LOG("Formatting drive '%s' as KFS...",drive->model_number);
	if (drive->block_size>4096){
		WARN("KFS requires block_size to be less than or equal to 4096 bytes");
		return 0;
	}
	u64 block_count=drive->block_count>>(12-drive->block_size_shift);
	if (block_count>0xffffffff){
		block_count=0xffffffff;
	}
	INFO("%lu total blocks, %lu BATC blocks",block_count,(block_count+KFS_BATC_BLOCK_COUNT-1)/KFS_BATC_BLOCK_COUNT);
	kfs_large_block_index_t first_free_block_index=2;
	if (boot_length){
		// adjust first_free_block_index
		ERROR("Unimplemented: kfs_format_drive.boot");
	}
	kfs_root_block_t root={
		KFS_SIGNATURE,
		block_count,
		(block_count+KFS_BATC_BLOCK_COUNT-1)/KFS_BATC_BLOCK_COUNT,
		first_free_block_index
	};
	for (u16 i=0;i<64;i++){
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
	for (kfs_large_block_index_t processed_block_count=0;processed_block_count<block_count;processed_block_count+=KFS_BATC_BLOCK_COUNT){
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
	INFO("Reserving header blocks...");
	_drive_read(drive,root.batc_block_index,&batc,8);
	while (first_free_block_index){
		first_free_block_index--;
		batc.bitmap1[first_free_block_index>>6]&=~(1ull<<(first_free_block_index&63));
	}
	for (u16 i=0;i<4030;i++){
		if (batc.bitmap1[i]){
			break;
		}
		batc.bitmap2[i>>6]&=~(1ull<<(i&63));
	}
	for (u8 i=0;i<63;i++){
		if (batc.bitmap2[i]){
			break;
		}
		batc.bitmap3&=~(1ull<<i);
	}
	_drive_write(drive,root.batc_block_index,&batc,8);
	LOG("Drive '%s' successfully formatted as KFS",drive->model_number);
	return 1;
}
