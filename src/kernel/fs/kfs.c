#include <kernel/drive/drive.h>
#include <kernel/fs/kfs.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kernel/vfs/vfs.h>
#define KERNEL_LOG_NAME "kfs"



#define DRIVE_BLOCK_SIZE_SHIFT 9

#define DRIVE_FIRST_FREE_BLOCK_INDEX 17



#define KFS_BLOCK_CACHE_NFDA_PRESENT 0x001
#define KFS_BLOCK_CACHE_NFDA_DIRTY 0x002
#define KFS_BLOCK_CACHE_NDA1_PRESENT 0x004
#define KFS_BLOCK_CACHE_NDA1_DIRTY 0x008
#define KFS_BLOCK_CACHE_NDA2_PRESENT 0x010
#define KFS_BLOCK_CACHE_NDA2_DIRTY 0x020
#define KFS_BLOCK_CACHE_NDA3_PRESENT 0x040
#define KFS_BLOCK_CACHE_NDA3_DIRTY 0x080
#define KFS_BLOCK_CACHE_BATC_PRESENT 0x100
#define KFS_BLOCK_CACHE_BATC_DIRTY 0x200
#define KFS_BLOCK_CACHE_ROOT_PRESENT 0x400
#define KFS_BLOCK_CACHE_ROOT_DIRTY 0x800

#define KFS_BATC_BLOCK_COUNT 257984

#define KFS_NODE_ID_NONE 0xffffffff

#define KFS_MASK_NAME_LENGTH 0x7f
#define KFS_FLAG_DIRECTORY 0x80



typedef u8 kfs_flags_t;



typedef u32 kfs_index_t;



typedef u32 kfs_large_block_index_t; // 4096-aligned



typedef struct _KFS_RANGE{
	kfs_large_block_index_t block_index;
	kfs_large_block_index_t block_count;
} kfs_range_t;



typedef struct _KFS_NODE{
	kfs_large_block_index_t block_index;
	kfs_index_t index;
	kfs_index_t parent;
	kfs_index_t prev_sibling;
	kfs_index_t next_sibling;
	kfs_flags_t flags;
	char name[27];
	union{
		struct{
			u64 length;
			kfs_large_block_index_t nfda_head;
			kfs_large_block_index_t nfda_tail;
		} file;
		struct{
			kfs_index_t first_child;
		} directory;
	} data;
} kfs_node_t;



typedef struct _KFS_NFDA_BLOCK{ // Node File Data Array [1 block]
	kfs_large_block_index_t block_index;
	kfs_large_block_index_t prev_block_index;
	kfs_large_block_index_t next_block_index;
	kfs_large_block_index_t data_length;
	kfs_range_t ranges[510];
} kfs_nfda_block_t;



typedef struct _KFS_NDA1_BLOCK{ // Node Descriptor Array level1 [1 block]
	kfs_node_t nodes[64];
} kfs_nda1_block_t;



typedef struct _KFS_NDA2_BLOCK{ // Node Descriptor Array level2 [2 blocks]
	kfs_large_block_index_t block_index[2];
	kfs_index_t node_index;
	u8 bitmap3;
	u8 _padding[3];
	u64 bitmap2[8];
	u64 bitmap1[507];
	kfs_large_block_index_t nda1[507];
	u8 _padding2[2028];
} kfs_nda2_block_t;



typedef struct _KFS_NDA3_BLOCK{ // Node Descriptor Array level3 [1 block]
	kfs_large_block_index_t block_index;
	kfs_index_t node_index;
	kfs_large_block_index_t nda2[1022];
} kfs_nda3_block_t;



typedef struct _KFS_BATC_BLOCK{ // Block Allocation Table [8 blocks]
	kfs_large_block_index_t block_index;
	kfs_large_block_index_t first_block_index;
	u64 bitmap3;
	u64 bitmap2[63];
	u64 bitmap1[4031];
} kfs_batc_block_t;



typedef struct _KFS_ROOT_BLOCK{ // Root [1 block]
	u64 signature;
	kfs_large_block_index_t block_count;
	kfs_large_block_index_t batc_block_index;
	kfs_large_block_index_t nda3[128];
	u64 root_block_count;
	u64 batc_block_count;
	u64 nda3_block_count;
	u64 nda2_block_count;
	u64 nda1_block_count;
	u64 nfda_block_count;
	u64 data_block_count;
	u8 _padding[3508];
} kfs_root_block_t;



_Static_assert(sizeof(kfs_nfda_block_t)==4096);
_Static_assert(sizeof(kfs_nda1_block_t)==4096);
_Static_assert(sizeof(kfs_nda2_block_t)==8192);
_Static_assert(sizeof(kfs_nda3_block_t)==4096);
_Static_assert(sizeof(kfs_batc_block_t)==32768);
_Static_assert(sizeof(kfs_root_block_t)==4096);



typedef struct _KFS_BLOCK_CACHE{ // all of the blocks are aligned on a page boundary
	kfs_nfda_block_t nfda;
	kfs_nda1_block_t nda1;
	kfs_nda2_block_t nda2;
	kfs_nda3_block_t nda3;
	kfs_batc_block_t batc;
	kfs_root_block_t root;
	u8 empty_block[4096];
	kfs_large_block_index_t nda1_block_index;
	u16 flags;
	const drive_t* drive;
} kfs_block_cache_t;



typedef struct _KFS_FS_NODE{
	vfs_node_t header;
	kfs_large_block_index_t block_index;
	kfs_index_t id;
} kfs_fs_node_t;



static void KERNEL_CORE_CODE _drive_read(const drive_t* drive,kfs_large_block_index_t offset,void* buffer,kfs_large_block_index_t length){
	if (drive->read_write(drive->extra_data,(offset<<(12-DRIVE_BLOCK_SIZE_SHIFT)),(void*)buffer,length<<(12-DRIVE_BLOCK_SIZE_SHIFT))!=(length<<(12-DRIVE_BLOCK_SIZE_SHIFT))){
		ERROR_CORE("Error reading data from drive");
	}
}



static void KERNEL_CORE_CODE _drive_write(const drive_t* drive,kfs_large_block_index_t offset,const void* buffer,kfs_large_block_index_t length){
	if (drive->read_write(drive->extra_data,(offset<<(12-DRIVE_BLOCK_SIZE_SHIFT))|DRIVE_OFFSET_FLAG_WRITE,(void*)buffer,length<<(12-DRIVE_BLOCK_SIZE_SHIFT))!=(length<<(12-DRIVE_BLOCK_SIZE_SHIFT))){
		ERROR_CORE("Error writing data to drive");
	}
}



static inline void* KERNEL_CORE_CODE _nda2_block_get_high_part(kfs_nda2_block_t* nda2){
	return (void*)(((u64)nda2)+4096);
}



static inline void KERNEL_CORE_CODE _block_cache_flush_nfda(kfs_block_cache_t* block_cache){
	if (!(block_cache->flags&KFS_BLOCK_CACHE_NFDA_DIRTY)){
		return;
	}
	block_cache->flags&=~KFS_BLOCK_CACHE_NFDA_DIRTY;
	_drive_write(block_cache->drive,block_cache->nfda.block_index,&(block_cache->nfda),1);
}



static inline void KERNEL_CORE_CODE _block_cache_flush_nda1(kfs_block_cache_t* block_cache){
	if (!(block_cache->flags&KFS_BLOCK_CACHE_NDA1_DIRTY)){
		return;
	}
	block_cache->flags&=~KFS_BLOCK_CACHE_NDA1_DIRTY;
	_drive_write(block_cache->drive,block_cache->nda1_block_index,&(block_cache->nda1),1);
}



static inline void KERNEL_CORE_CODE _block_cache_flush_nda2(kfs_block_cache_t* block_cache){
	if (!(block_cache->flags&KFS_BLOCK_CACHE_NDA2_DIRTY)){
		return;
	}
	block_cache->flags&=~KFS_BLOCK_CACHE_NDA2_DIRTY;
	_drive_write(block_cache->drive,block_cache->nda2.block_index[0],&(block_cache->nda2),1);
	_drive_write(block_cache->drive,block_cache->nda2.block_index[1],_nda2_block_get_high_part(&(block_cache->nda2)),1);
}



static inline void KERNEL_CORE_CODE _block_cache_flush_nda3(kfs_block_cache_t* block_cache){
	if (!(block_cache->flags&KFS_BLOCK_CACHE_NDA3_DIRTY)){
		return;
	}
	block_cache->flags&=~KFS_BLOCK_CACHE_NDA3_DIRTY;
	_drive_write(block_cache->drive,block_cache->nda3.block_index,&(block_cache->nda3),1);
}



static inline void KERNEL_CORE_CODE _block_cache_flush_batc(kfs_block_cache_t* block_cache){
	if (!(block_cache->flags&KFS_BLOCK_CACHE_BATC_DIRTY)){
		return;
	}
	block_cache->flags&=~KFS_BLOCK_CACHE_BATC_DIRTY;
	_drive_write(block_cache->drive,block_cache->batc.block_index,&(block_cache->batc),8);
}



static inline void KERNEL_CORE_CODE _block_cache_flush_root(kfs_block_cache_t* block_cache){
	if (!(block_cache->flags&KFS_BLOCK_CACHE_ROOT_DIRTY)){
		return;
	}
	block_cache->flags&=~KFS_BLOCK_CACHE_ROOT_DIRTY;
	if (block_cache->drive->read_write(block_cache->drive->extra_data,1|DRIVE_OFFSET_FLAG_WRITE,&(block_cache->root),sizeof(kfs_root_block_t)>>DRIVE_BLOCK_SIZE_SHIFT)!=(sizeof(kfs_root_block_t)>>DRIVE_BLOCK_SIZE_SHIFT)){
		ERROR_CORE("Error writing data to drive");
	}
}



static kfs_large_block_index_t KERNEL_CORE_CODE _block_cache_alloc_block(kfs_block_cache_t* block_cache){
	if ((block_cache->flags&KFS_BLOCK_CACHE_BATC_PRESENT)&&block_cache->batc.bitmap3){
		goto _batc_found;
	}
	_block_cache_flush_batc(block_cache);
	kfs_large_block_index_t block_index=block_cache->root.batc_block_index;
	do{
		_drive_read(block_cache->drive,block_index,&(block_cache->batc),8);
		block_index+=8;
	} while (!block_cache->batc.bitmap3);
	block_cache->flags|=KFS_BLOCK_CACHE_BATC_PRESENT;
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



static void KERNEL_CORE_CODE _block_cache_dealloc_block(kfs_block_cache_t* block_cache,kfs_large_block_index_t block_index){
	if ((block_cache->flags&KFS_BLOCK_CACHE_BATC_PRESENT)&&block_index>=block_cache->batc.first_block_index&&block_index<block_cache->batc.first_block_index+KFS_BATC_BLOCK_COUNT){
		goto _batc_found;
	}
	_block_cache_flush_batc(block_cache);
	_drive_read(block_cache->drive,block_index/KFS_BATC_BLOCK_COUNT*8+block_cache->root.batc_block_index,&(block_cache->batc),8);
_batc_found:
	block_cache->flags|=KFS_BLOCK_CACHE_BATC_DIRTY;
	block_index-=block_cache->batc.first_block_index;
	block_cache->batc.bitmap1[block_index>>6]|=1ull<<(block_index&63);
	block_cache->batc.bitmap2[block_index>>12]|=1ull<<((block_index>>6)&63);
	block_cache->batc.bitmap3|=1ull<<(block_index>>12);
}



static void KERNEL_CORE_CODE _block_cache_clear_block(kfs_block_cache_t* block_cache,kfs_large_block_index_t block_index){
	_drive_write(block_cache->drive,block_index,&(block_cache->empty_block),1);
}



static void _block_cache_init_nfda(kfs_block_cache_t* block_cache,kfs_large_block_index_t block_index){
	block_cache->root.nfda_block_count+=1<<(12-DRIVE_BLOCK_SIZE_SHIFT);
	_block_cache_flush_nfda(block_cache);
	block_cache->nfda.block_index=block_index;
	block_cache->nfda.prev_block_index=0;
	block_cache->nfda.next_block_index=0;
	block_cache->nfda.data_length=0;
	for (u16 i=0;i<510;i++){
		block_cache->nfda.ranges[i].block_index=0;
		block_cache->nfda.ranges[i].block_count=0;
	}
	block_cache->flags|=KFS_BLOCK_CACHE_NFDA_PRESENT|KFS_BLOCK_CACHE_NFDA_DIRTY|KFS_BLOCK_CACHE_ROOT_DIRTY;
}



static kfs_large_block_index_t KERNEL_CORE_CODE _block_cache_init_nda2(kfs_block_cache_t* block_cache,kfs_index_t node_index){
	block_cache->root.nda2_block_count+=2<<(12-DRIVE_BLOCK_SIZE_SHIFT);
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
		block_cache->nda2.nda1[i]=0;
	}
	block_cache->flags|=KFS_BLOCK_CACHE_NDA2_PRESENT|KFS_BLOCK_CACHE_NDA2_DIRTY|KFS_BLOCK_CACHE_ROOT_DIRTY;
	return block_cache->nda2.block_index[0];
}



static kfs_large_block_index_t KERNEL_CORE_CODE _block_cache_init_nda3(kfs_block_cache_t* block_cache,kfs_index_t node_index){
	block_cache->root.nda3_block_count+=1<<(12-DRIVE_BLOCK_SIZE_SHIFT);
	block_cache->nda3.block_index=_block_cache_alloc_block(block_cache);
	block_cache->nda3.node_index=node_index;
	block_cache->nda3.nda2[0]=_block_cache_init_nda2(block_cache,node_index<<10);
	for (unsigned int i=1;i<1022;i++){
		block_cache->nda3.nda2[i]=0;
	}
	block_cache->flags|=KFS_BLOCK_CACHE_NDA3_PRESENT|KFS_BLOCK_CACHE_NDA3_DIRTY|KFS_BLOCK_CACHE_ROOT_DIRTY;
	return block_cache->nda3.block_index;
}



static void KERNEL_CORE_CODE _block_cache_load_nfda(kfs_block_cache_t* block_cache,kfs_large_block_index_t block_index){
	if ((block_cache->flags&KFS_BLOCK_CACHE_NFDA_PRESENT)&&block_cache->nfda.block_index==block_index){
		return;
	}
	_block_cache_flush_nfda(block_cache);
	_drive_read(block_cache->drive,block_index,&(block_cache->nfda),1);
	block_cache->flags|=KFS_BLOCK_CACHE_NFDA_PRESENT;
}



static void KERNEL_CORE_CODE _block_cache_load_nda1(kfs_block_cache_t* block_cache,kfs_large_block_index_t block_index){
	if ((block_cache->flags&KFS_BLOCK_CACHE_NDA1_PRESENT)&&block_cache->nda1_block_index==block_index){
		return;
	}
	_block_cache_flush_nda1(block_cache);
	block_cache->nda1_block_index=block_index;
	_drive_read(block_cache->drive,block_index,&(block_cache->nda1),1);
	block_cache->flags|=KFS_BLOCK_CACHE_NDA1_PRESENT;
}



static kfs_node_t* KERNEL_CORE_CODE _get_node_by_index(kfs_block_cache_t* block_cache,kfs_index_t index){
	if ((block_cache->flags&KFS_BLOCK_CACHE_NDA2_PRESENT)&&block_cache->nda2.node_index==(index>>15)){
		goto _nda2_found;
	}
	_block_cache_flush_nda2(block_cache);
	if ((block_cache->flags&KFS_BLOCK_CACHE_NDA3_PRESENT)&&block_cache->nda3.node_index==(index>>25)){
		goto _nda3_found;
	}
	_block_cache_flush_nda3(block_cache);
	if (!block_cache->root.nda3[index>>25]){
		return 0;
	}
	_drive_read(block_cache->drive,block_cache->root.nda3[index>>25],&(block_cache->nda3),1);
	block_cache->flags|=KFS_BLOCK_CACHE_NDA3_PRESENT;
_nda3_found:
	if (!block_cache->nda3.nda2[(index>>15)&0x3ff]){
		return 0;
	}
	_drive_read(block_cache->drive,block_cache->nda3.nda2[(index>>15)&0x3ff],&(block_cache->nda2),1);
	_drive_read(block_cache->drive,block_cache->nda2.block_index[1],_nda2_block_get_high_part(&(block_cache->nda2)),1);
	block_cache->flags|=KFS_BLOCK_CACHE_NDA2_PRESENT;
_nda2_found:
	if (!block_cache->nda2.nda1[(index>>6)&0x1ff]){
		return 0;
	}
	_block_cache_load_nda1(block_cache,block_cache->nda2.nda1[(index>>6)&0x1ff]);
	return block_cache->nda1.nodes+(index&63);
}



static void KERNEL_CORE_CODE _node_to_fs_node(kfs_node_t* node,kfs_fs_node_t* out){
	out->header.type=((node->flags&KFS_FLAG_DIRECTORY)?VFS_NODE_TYPE_DIRECTORY:VFS_NODE_TYPE_FILE);
	out->header.parent=(node->parent==KFS_NODE_ID_NONE?VFS_NODE_ID_EMPTY:VFS_NODE_ID_UNKNOWN);
	out->header.prev_sibling=(node->prev_sibling==KFS_NODE_ID_NONE?VFS_NODE_ID_EMPTY:VFS_NODE_ID_UNKNOWN);
	out->header.next_sibling=(node->next_sibling==KFS_NODE_ID_NONE?VFS_NODE_ID_EMPTY:VFS_NODE_ID_UNKNOWN);
	out->header.first_child=(!(node->flags&KFS_FLAG_DIRECTORY)||node->data.directory.first_child==KFS_NODE_ID_NONE?VFS_NODE_ID_EMPTY:VFS_NODE_ID_UNKNOWN);
	out->block_index=node->block_index;
	out->id=node->index;
}



static _Bool KERNEL_CORE_CODE _nda3_find_free_nda2(kfs_block_cache_t* block_cache,kfs_large_block_index_t checked_nda2_index){
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



static kfs_node_t* KERNEL_CORE_CODE _alloc_node(kfs_block_cache_t* block_cache,kfs_flags_t type,const char* name,u8 name_length){
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
	for (u8 i=0;i<128;i++){
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
	block_cache->flags|=KFS_BLOCK_CACHE_NDA2_DIRTY;
	kfs_index_t i=__builtin_ctzll(block_cache->nda2.bitmap3);
	kfs_index_t j=__builtin_ctzll(block_cache->nda2.bitmap2[i])|(i<<6);
	kfs_index_t k=0;
	if (!block_cache->nda2.nda1[j]){
		block_cache->root.nda1_block_count+=1<<(12-DRIVE_BLOCK_SIZE_SHIFT);
		block_cache->nda2.nda1[j]=_block_cache_alloc_block(block_cache);
		block_cache->nda2.bitmap1[j]=0xfffffffffffffffeull;
		block_cache->flags|=KFS_BLOCK_CACHE_ROOT_DIRTY;
	}
	else{
		k=__builtin_ctzll(block_cache->nda2.bitmap1[j]);
		block_cache->nda2.bitmap1[j]&=block_cache->nda2.bitmap1[j]-1;
		if (!block_cache->nda2.bitmap1[j]){
			block_cache->nda2.bitmap2[i]&=block_cache->nda2.bitmap2[i]-1;
			if (!block_cache->nda2.bitmap2[i]){
				block_cache->nda2.bitmap3&=block_cache->nda2.bitmap3-1;
			}
		}
	}
	_block_cache_load_nda1(block_cache,block_cache->nda2.nda1[j]);
	block_cache->flags|=KFS_BLOCK_CACHE_NDA1_DIRTY;
	kfs_node_t* out=block_cache->nda1.nodes+k;
	out->block_index=block_cache->nda2.nda1[j];
	out->index=k|(j<<6)|(block_cache->nda2.node_index<<15);
	out->parent=KFS_NODE_ID_NONE;
	out->prev_sibling=KFS_NODE_ID_NONE;
	out->next_sibling=KFS_NODE_ID_NONE;
	if (name_length>33){
		name_length=33;
	}
	out->flags=type|name_length;
	memcpy(out->name,name,name_length);
	if (type==KFS_FLAG_DIRECTORY){
		out->data.directory.first_child=KFS_NODE_ID_NONE;
	}
	else{
		out->data.file.length=0;
		out->data.file.nfda_head=0;
		out->data.file.nfda_tail=0;
	}
	return out;
}



static u64 KERNEL_CORE_CODE _get_nfda_and_range_index(kfs_block_cache_t* block_cache,kfs_node_t* node,u64 offset){
	u64 extra=offset&4095;
	offset>>=12;
	kfs_large_block_index_t block_index=node->data.file.nfda_head;
	while (block_index){
		_block_cache_load_nfda(block_cache,block_index);
		block_index=block_cache->nfda.next_block_index;
		if (offset>=block_cache->nfda.data_length){
			offset-=block_cache->nfda.data_length;
			continue;
		}
		u64 out=0;
		while (offset>block_cache->nfda.ranges[out].block_count){
			offset-=block_cache->nfda.ranges[out].block_count;
			out++;
		}
		return out|(((offset<<12)|extra)<<9);
	}
	panic("Unreachable statement",0);
	return 0xffffffffffffffffull;
}



static _Bool _resize_node_down(kfs_block_cache_t* block_cache,kfs_node_t* node,u64 size){
	u64 underflow=((node->data.file.length+4095)>>12)-((size+4095)>>12);
	if (!underflow){
		return 1;
	}
	block_cache->root.data_block_count-=underflow<<(12-DRIVE_BLOCK_SIZE_SHIFT);
	block_cache->flags|=KFS_BLOCK_CACHE_ROOT_DIRTY;
	if (!node->data.file.nfda_tail){
		return 0;
	}
	_block_cache_load_nfda(block_cache,node->data.file.nfda_tail);
	u16 range_index=0;
	while (range_index<510&&block_cache->nfda.ranges[range_index+1].block_index){
		range_index++;
	}
	do{
		u64 count=block_cache->nfda.ranges[range_index].block_count;
		if (count>underflow){
			count=underflow;
		}
		underflow-=count;
		while (count){
			count--;
			_block_cache_dealloc_block(block_cache,block_cache->nfda.ranges[range_index].block_index+count);
		}
		block_cache->nfda.ranges[range_index].block_index=0;
		block_cache->nfda.ranges[range_index].block_count=0;
		if (range_index){
			range_index--;
			continue;
		}
		block_cache->root.nfda_block_count-=1<<(12-DRIVE_BLOCK_SIZE_SHIFT);
		block_cache->flags|=KFS_BLOCK_CACHE_ROOT_DIRTY;
		u64 prev_block_index=block_cache->nfda.prev_block_index;
		_block_cache_dealloc_block(block_cache,block_cache->nfda.block_index);
		if (!prev_block_index){
			node->data.file.nfda_head=0;
			node->data.file.nfda_tail=0;
			return 1;
		}
		node->data.file.nfda_tail=prev_block_index;
		_block_cache_load_nfda(block_cache,prev_block_index);
	} while (underflow);
	return 1;
}



static _Bool _resize_node_up(kfs_block_cache_t* block_cache,kfs_node_t* node,u64 size){
	u64 overflow=((size+4095)>>12)-((node->data.file.length+4095)>>12);
	if (!overflow){
		return 1;
	}
	block_cache->root.data_block_count+=overflow<<(12-DRIVE_BLOCK_SIZE_SHIFT);
	block_cache->flags|=KFS_BLOCK_CACHE_ROOT_DIRTY;
	if (!node->data.file.nfda_tail){
		node->data.file.nfda_head=_block_cache_alloc_block(block_cache);
		node->data.file.nfda_tail=node->data.file.nfda_head;
		_block_cache_init_nfda(block_cache,node->data.file.nfda_tail);
	}
	else{
		_block_cache_load_nfda(block_cache,node->data.file.nfda_tail);
	}
	u16 range_index=0;
	while (range_index<510&&block_cache->nfda.ranges[range_index+1].block_index){
		range_index++;
	}
	kfs_large_block_index_t new_block_index;
	do{
		overflow--;
		new_block_index=_block_cache_alloc_block(block_cache);
		_block_cache_clear_block(block_cache,new_block_index);
		if (new_block_index==block_cache->nfda.ranges[range_index].block_index+block_cache->nfda.ranges[range_index].block_count){
			block_cache->nfda.ranges[range_index].block_count++;
		}
		else{
			if (block_cache->nfda.ranges[range_index].block_index){
				range_index++;
				if (range_index==510){
					ERROR("Unimplemented: allocate new NFDA block");
					return 0;
				}
			}
			block_cache->nfda.ranges[range_index].block_index=new_block_index;
			block_cache->nfda.ranges[range_index].block_count=1;
		}
		block_cache->nfda.data_length++;
		block_cache->flags|=KFS_BLOCK_CACHE_NFDA_DIRTY;
	} while (overflow);
	block_cache->flags|=KFS_BLOCK_CACHE_NDA1_DIRTY;
	return 1;
}



static vfs_node_t* _kfs_create(partition_t* fs,_Bool is_directory,const char* name,u8 name_length){
	kfs_node_t* kfs_node=_alloc_node(fs->extra_data,(is_directory?KFS_FLAG_DIRECTORY:0),name,name_length);
	if (!kfs_node){
		return NULL;
	}
	vfs_node_t* out=vfs_alloc(fs,name,name_length);
	_node_to_fs_node(kfs_node,(kfs_fs_node_t*)out);
	return out;
}



static _Bool _kfs_delete(partition_t* fs,vfs_node_t* node){
	kfs_block_cache_t* block_cache=fs->extra_data;
	kfs_fs_node_t* kfs_fs_node=(kfs_fs_node_t*)node;
	kfs_node_t* kfs_node=_get_node_by_index(block_cache,kfs_fs_node->id); // loads both NDA1 and NDA2
	if (!kfs_node){
		return 0;
	}
	if (!(kfs_node->flags&KFS_FLAG_DIRECTORY)&&kfs_node->data.file.nfda_head){
		kfs_large_block_index_t block_index=kfs_node->data.file.nfda_head;
		do{
			_block_cache_load_nfda(block_cache,block_index);
			_block_cache_dealloc_block(block_cache,block_index);
			block_cache->root.nfda_block_count-=1<<(12-DRIVE_BLOCK_SIZE_SHIFT);
			block_cache->root.data_block_count-=block_cache->nfda.data_length<<(12-DRIVE_BLOCK_SIZE_SHIFT);
			block_index=block_cache->nfda.next_block_index;
			for (u16 i=0;i<510;i++){
				kfs_large_block_index_t base=block_cache->nfda.ranges[i].block_index;
				if (!base){
					break;
				}
				for (kfs_large_block_index_t j=0;j<block_cache->nfda.ranges[i].block_count;j++){
					_block_cache_dealloc_block(block_cache,base+j);
				}
			}
			block_cache->flags&=~(KFS_BLOCK_CACHE_NFDA_PRESENT|KFS_BLOCK_CACHE_NFDA_DIRTY);
		} while (block_index);
		block_cache->flags|=KFS_BLOCK_CACHE_ROOT_DIRTY;
	}
	block_cache->flags|=KFS_BLOCK_CACHE_NDA2_DIRTY;
	kfs_index_t node_index=kfs_node->index;
	u64 mask=1ull<<(node_index&63);
	node_index>>=6;
	block_cache->nda2.bitmap1[node_index]|=mask;
	if (block_cache->nda2.bitmap1[node_index]==0xffffffffffffffffull){
		block_cache->root.nda1_block_count-=1<<(12-DRIVE_BLOCK_SIZE_SHIFT);
		_block_cache_dealloc_block(block_cache,block_cache->nda2.nda1[node_index]);
		block_cache->nda2.nda1[node_index]=0;
		block_cache->flags|=KFS_BLOCK_CACHE_ROOT_DIRTY;
	}
	block_cache->nda2.bitmap2[node_index>>6]|=1ull<<(node_index&63);
	block_cache->nda2.bitmap3|=1<<(node_index>>6);
	return 1;
}



static vfs_node_t* KERNEL_CORE_CODE _kfs_get_relative(partition_t* fs,vfs_node_t* node,u8 relative){
	kfs_block_cache_t* block_cache=fs->extra_data;
	kfs_fs_node_t* kfs_fs_node=(kfs_fs_node_t*)node;
	_block_cache_load_nda1(block_cache,kfs_fs_node->block_index);
	kfs_node_t* kfs_node=block_cache->nda1.nodes+(kfs_fs_node->id&63);
	kfs_index_t out_id;
	switch (relative){
		case VFS_RELATIVE_PARENT:
			out_id=kfs_node->parent;
			break;
		case VFS_RELATIVE_PREV_SIBLING:
			out_id=kfs_node->prev_sibling;
			break;
		case VFS_RELATIVE_NEXT_SIBLING:
			out_id=kfs_node->next_sibling;
			break;
		case VFS_RELATIVE_FIRST_CHILD:
			if (!(kfs_node->flags&KFS_FLAG_DIRECTORY)){
				return NULL;
			}
			out_id=kfs_node->data.directory.first_child;
			break;
		default:
			return NULL;
	}
	if (out_id==KFS_NODE_ID_NONE){
		return NULL;
	}
	if (!out_id){
		return fs->root;
	}
	kfs_node_t* kfs_out=_get_node_by_index(block_cache,out_id);
	vfs_node_t* out=vfs_alloc(fs,kfs_out->name,kfs_out->flags&KFS_MASK_NAME_LENGTH);
	_node_to_fs_node(kfs_out,(kfs_fs_node_t*)out);
	return out;
}



static _Bool _kfs_set_relative(partition_t* fs,vfs_node_t* node,u8 relative,vfs_node_t* other){
	kfs_block_cache_t* block_cache=fs->extra_data;
	kfs_fs_node_t* kfs_fs_node=(kfs_fs_node_t*)node;
	kfs_index_t other_id=(other?((kfs_fs_node_t*)other)->id:KFS_NODE_ID_NONE);
	_block_cache_load_nda1(block_cache,kfs_fs_node->block_index);
	block_cache->flags|=KFS_BLOCK_CACHE_NDA1_DIRTY;
	kfs_node_t* kfs_node=block_cache->nda1.nodes+(kfs_fs_node->id&63);
	switch (relative){
		case VFS_RELATIVE_PARENT:
			kfs_node->parent=other_id;
			break;
		case VFS_RELATIVE_PREV_SIBLING:
			kfs_node->prev_sibling=other_id;
			break;
		case VFS_RELATIVE_NEXT_SIBLING:
			kfs_node->next_sibling=other_id;
			break;
		case VFS_RELATIVE_FIRST_CHILD:
			if (!(kfs_node->flags&KFS_FLAG_DIRECTORY)){
				return 0;
			}
			kfs_node->data.directory.first_child=other_id;
			break;
		default:
			return 0;
	}
	return 1;
}



static _Bool _kfs_move_file(partition_t* fs,vfs_node_t* src_node,vfs_node_t* dst_node){
	kfs_block_cache_t* block_cache=fs->extra_data;
	kfs_fs_node_t* kfs_fs_src_node=(kfs_fs_node_t*)src_node;
	_block_cache_load_nda1(block_cache,kfs_fs_src_node->block_index);
	kfs_node_t* kfs_src_node=block_cache->nda1.nodes+(kfs_fs_src_node->id&63);
	u64 length=kfs_src_node->data.file.length;
	kfs_large_block_index_t nfda_head=kfs_src_node->data.file.nfda_head;
	kfs_large_block_index_t nfda_tail=kfs_src_node->data.file.nfda_tail;
	kfs_src_node->data.file.length=0;
	kfs_src_node->data.file.nfda_head=0;
	kfs_src_node->data.file.nfda_tail=0;
	kfs_fs_node_t* kfs_fs_dst_node=(kfs_fs_node_t*)dst_node;
	_block_cache_load_nda1(block_cache,kfs_fs_dst_node->block_index);
	kfs_node_t* kfs_dst_node=block_cache->nda1.nodes+(kfs_fs_dst_node->id&63);
	kfs_dst_node->data.file.length=length;
	kfs_dst_node->data.file.nfda_head=nfda_head;
	kfs_dst_node->data.file.nfda_tail=nfda_tail;
	return 1;
}



static u64 KERNEL_CORE_CODE _kfs_read(partition_t* fs,vfs_node_t* node,u64 offset,u8* buffer,u64 count){
	kfs_block_cache_t* block_cache=fs->extra_data;
	kfs_fs_node_t* kfs_fs_node=(kfs_fs_node_t*)node;
	_block_cache_load_nda1(block_cache,kfs_fs_node->block_index);
	kfs_node_t* kfs_node=block_cache->nda1.nodes+(kfs_fs_node->id&63);
	if ((kfs_node->flags&KFS_FLAG_DIRECTORY)||offset>=kfs_node->data.file.length){
		return 0;
	}
	if (offset+count>kfs_node->data.file.length){
		count=kfs_node->data.file.length-offset;
	}
	u64 range_index_and_offset=_get_nfda_and_range_index(block_cache,kfs_node,offset);
	u16 range_index=range_index_and_offset&0x1ff;
	offset=range_index_and_offset>>9;
	u64 extra=offset&((1<<DRIVE_BLOCK_SIZE_SHIFT)-1);
	u64 out=0;
	if (extra){
		extra=(1<<DRIVE_BLOCK_SIZE_SHIFT)-extra;
		if (extra>count){
			extra=count;
		}
		u8 chunk[1<<DRIVE_BLOCK_SIZE_SHIFT];
		if (block_cache->drive->read_write(block_cache->drive->extra_data,(block_cache->nfda.ranges[range_index].block_index<<(12-DRIVE_BLOCK_SIZE_SHIFT))+(offset>>DRIVE_BLOCK_SIZE_SHIFT),chunk,1)!=1){
			ERROR("Error reading data from drive");
			return 0;
		}
		memcpy(buffer,chunk+(offset&((1<<DRIVE_BLOCK_SIZE_SHIFT)-1)),extra);
		out+=extra;
		buffer+=extra;
		count-=extra;
		offset+=(1<<DRIVE_BLOCK_SIZE_SHIFT)-1;
	}
	offset>>=DRIVE_BLOCK_SIZE_SHIFT;
	while (1){
		if (offset>=(block_cache->nfda.ranges[range_index].block_count<<(12-DRIVE_BLOCK_SIZE_SHIFT))){
			offset=0;
			range_index++;
			if (range_index==510){
				panic("Unimplemented: load next NFDA block",0);
				return 0;
			}
		}
		if (count<(1<<DRIVE_BLOCK_SIZE_SHIFT)){
			break;
		}
		u32 transfer_size=(block_cache->nfda.ranges[range_index].block_count<<(12-DRIVE_BLOCK_SIZE_SHIFT))-offset;
		if ((count>>DRIVE_BLOCK_SIZE_SHIFT)<transfer_size){
			transfer_size=count>>DRIVE_BLOCK_SIZE_SHIFT;
		}
		transfer_size=block_cache->drive->read_write(block_cache->drive->extra_data,(block_cache->nfda.ranges[range_index].block_index<<(12-DRIVE_BLOCK_SIZE_SHIFT))+offset,(void*)buffer,transfer_size);
		out+=transfer_size<<DRIVE_BLOCK_SIZE_SHIFT;
		buffer+=transfer_size<<DRIVE_BLOCK_SIZE_SHIFT;
		count-=transfer_size<<DRIVE_BLOCK_SIZE_SHIFT;
		offset+=transfer_size;
	}
	if (count){
		u8 chunk[1<<DRIVE_BLOCK_SIZE_SHIFT];
		if (block_cache->drive->read_write(block_cache->drive->extra_data,(block_cache->nfda.ranges[range_index].block_index<<(12-DRIVE_BLOCK_SIZE_SHIFT))+offset,chunk,1)!=1){
			ERROR_CORE("Error reading data from drive");
			return 0;
		}
		memcpy(buffer,chunk,count);
		out+=count;
	}
	return out;
}



static u64 _kfs_write(partition_t* fs,vfs_node_t* node,u64 offset,const u8* buffer,u64 count){
	kfs_block_cache_t* block_cache=fs->extra_data;
	kfs_fs_node_t* kfs_fs_node=(kfs_fs_node_t*)node;
	_block_cache_load_nda1(block_cache,kfs_fs_node->block_index);
	kfs_node_t* kfs_node=block_cache->nda1.nodes+(kfs_fs_node->id&63);
	if (kfs_node->flags&KFS_FLAG_DIRECTORY){
		return 0;
	}
	if (offset+count>kfs_node->data.file.length){
		if (!_resize_node_up(block_cache,kfs_node,offset+count)){
			return 0;
		}
		kfs_node->data.file.length=offset+count;
		block_cache->flags|=KFS_BLOCK_CACHE_NDA1_DIRTY;
	}
	u64 range_index_and_offset=_get_nfda_and_range_index(block_cache,kfs_node,offset);
	u16 range_index=range_index_and_offset&0x1ff;
	offset=range_index_and_offset>>9;
	u64 extra=offset&((1<<DRIVE_BLOCK_SIZE_SHIFT)-1);
	u64 out=0;
	if (extra){
		extra=(1<<DRIVE_BLOCK_SIZE_SHIFT)-extra;
		if (extra>count){
			extra=count;
		}
		u8 chunk[1<<DRIVE_BLOCK_SIZE_SHIFT];
		if (block_cache->drive->read_write(block_cache->drive->extra_data,(block_cache->nfda.ranges[range_index].block_index<<(12-DRIVE_BLOCK_SIZE_SHIFT))+(offset>>DRIVE_BLOCK_SIZE_SHIFT),chunk,1)!=1){
			ERROR("Error reading data from drive");
			return 0;
		}
		memcpy(chunk+(offset&((1<<DRIVE_BLOCK_SIZE_SHIFT)-1)),buffer,extra);
		if (block_cache->drive->read_write(block_cache->drive->extra_data,((block_cache->nfda.ranges[range_index].block_index<<(12-DRIVE_BLOCK_SIZE_SHIFT))+(offset>>DRIVE_BLOCK_SIZE_SHIFT))|DRIVE_OFFSET_FLAG_WRITE,chunk,1)!=1){
			ERROR("Error writing data to drive");
			return 0;
		}
		out+=extra;
		buffer+=extra;
		count-=extra;
		offset+=(1<<DRIVE_BLOCK_SIZE_SHIFT)-1;
	}
	offset>>=DRIVE_BLOCK_SIZE_SHIFT;
	while (1){
		if (offset>=(block_cache->nfda.ranges[range_index].block_count<<(12-DRIVE_BLOCK_SIZE_SHIFT))){
			offset=0;
			range_index++;
			if (range_index==510){
				panic("Unimplemented: load next NFDA block",0);
				return 0;
			}
		}
		if (count<(1<<DRIVE_BLOCK_SIZE_SHIFT)){
			break;
		}
		u32 transfer_size=(block_cache->nfda.ranges[range_index].block_count<<(12-DRIVE_BLOCK_SIZE_SHIFT))-offset;
		if ((count>>DRIVE_BLOCK_SIZE_SHIFT)<transfer_size){
			transfer_size=count>>DRIVE_BLOCK_SIZE_SHIFT;
		}
		transfer_size=block_cache->drive->read_write(block_cache->drive->extra_data,((block_cache->nfda.ranges[range_index].block_index<<(12-DRIVE_BLOCK_SIZE_SHIFT))+offset)|DRIVE_OFFSET_FLAG_WRITE,(void*)buffer,transfer_size);
		out+=transfer_size<<DRIVE_BLOCK_SIZE_SHIFT;
		buffer+=transfer_size<<DRIVE_BLOCK_SIZE_SHIFT;
		count-=transfer_size<<DRIVE_BLOCK_SIZE_SHIFT;
		offset+=transfer_size;
	}
	if (count){
		u8 chunk[1<<DRIVE_BLOCK_SIZE_SHIFT];
		if (block_cache->drive->read_write(block_cache->drive->extra_data,(block_cache->nfda.ranges[range_index].block_index<<(12-DRIVE_BLOCK_SIZE_SHIFT))+offset,chunk,1)!=1){
			ERROR("Error reading data from drive");
			return 0;
		}
		memcpy(chunk,buffer,count);
		if (block_cache->drive->read_write(block_cache->drive->extra_data,((block_cache->nfda.ranges[range_index].block_index<<(12-DRIVE_BLOCK_SIZE_SHIFT))+offset)|DRIVE_OFFSET_FLAG_WRITE,chunk,1)!=1){
			ERROR("Error writing data to drive");
			return 0;
		}
		out+=count;
	}
	return out;
}



static u64 _kfs_get_size(partition_t* fs,vfs_node_t* node){
	kfs_block_cache_t* block_cache=fs->extra_data;
	kfs_fs_node_t* kfs_fs_node=(kfs_fs_node_t*)node;
	_block_cache_load_nda1(block_cache,kfs_fs_node->block_index);
	kfs_node_t* kfs_node=block_cache->nda1.nodes+(kfs_fs_node->id&63);
	if (kfs_node->flags&KFS_FLAG_DIRECTORY){
		return 0;
	}
	return kfs_node->data.file.length;
}



static _Bool _kfs_set_size(partition_t* fs,vfs_node_t* node,u64 size){
	kfs_block_cache_t* block_cache=fs->extra_data;
	kfs_fs_node_t* kfs_fs_node=(kfs_fs_node_t*)node;
	_block_cache_load_nda1(block_cache,kfs_fs_node->block_index);
	kfs_node_t* kfs_node=block_cache->nda1.nodes+(kfs_fs_node->id&63);
	if (kfs_node->flags&KFS_FLAG_DIRECTORY){
		return 0;
	}
	if (((size+4095)>>12)==((kfs_node->data.file.length+4095)>>12)){
		goto _change_size;
	}
	if (size>kfs_node->data.file.length){
		if (!_resize_node_up(block_cache,kfs_node,size)){
			return 0;
		}
	}
	else{
		if (!_resize_node_down(block_cache,kfs_node,size)){
			return 0;
		}
	}
_change_size:
	kfs_node->data.file.length=size;
	block_cache->flags|=KFS_BLOCK_CACHE_NDA1_DIRTY;
	return 1;
}



static void _kfs_flush_cache(partition_t* fs){
	kfs_block_cache_t* block_cache=fs->extra_data;
	_block_cache_flush_nfda(block_cache);
	_block_cache_flush_nda1(block_cache);
	_block_cache_flush_nda2(block_cache);
	_block_cache_flush_nda3(block_cache);
	_block_cache_flush_batc(block_cache);
	_block_cache_flush_root(block_cache);
	fs->drive->stats->root_block_count=block_cache->root.root_block_count;
	fs->drive->stats->batc_block_count=block_cache->root.batc_block_count;
	fs->drive->stats->nda3_block_count=block_cache->root.nda3_block_count;
	fs->drive->stats->nda2_block_count=block_cache->root.nda2_block_count;
	fs->drive->stats->nda1_block_count=block_cache->root.nda1_block_count;
	fs->drive->stats->nfda_block_count=block_cache->root.nfda_block_count;
	fs->drive->stats->data_block_count=block_cache->root.data_block_count;
}



static const partition_file_system_config_t KERNEL_CORE_RDATA _kfs_fs_config={
	sizeof(kfs_fs_node_t),
	0,
	_kfs_create,
	_kfs_delete,
	_kfs_get_relative,
	_kfs_set_relative,
	_kfs_move_file,
	_kfs_read,
	_kfs_write,
	_kfs_get_size,
	_kfs_set_size,
	_kfs_flush_cache
};



void KERNEL_CORE_CODE kfs_load(const drive_t* drive,const partition_config_t* partition_config){
	LOG_CORE("Loading KFS file system from drive '%s'...",drive->model_number);
	INFO_CORE("Allocating block cache...");
	kfs_block_cache_t* block_cache=(void*)pmm_alloc(pmm_align_up_address(sizeof(kfs_block_cache_t))>>PAGE_SIZE_SHIFT,PMM_COUNTER_KFS,0);
	block_cache->flags=0;
	block_cache->drive=drive;
	memset(block_cache->empty_block,0,4096);
	INFO_CORE("Reading ROOT block...");
	if (drive->read_write(drive->extra_data,1,&(block_cache->root),sizeof(kfs_root_block_t)>>DRIVE_BLOCK_SIZE_SHIFT)!=(sizeof(kfs_root_block_t)>>DRIVE_BLOCK_SIZE_SHIFT)){
		ERROR_CORE("Error reading ROOT block from drive");
		return;
	}
	block_cache->flags|=KFS_BLOCK_CACHE_ROOT_PRESENT;
	kfs_fs_node_t* root=partition_add(drive,partition_config,&_kfs_fs_config,block_cache);
	kfs_node_t* kfs_root=_get_node_by_index(block_cache,0);
	if (!kfs_root){
		kfs_root=_alloc_node(block_cache,KFS_FLAG_DIRECTORY,NULL,0);
	}
	_node_to_fs_node(kfs_root,root);
	drive->stats->root_block_count=block_cache->root.root_block_count;
	drive->stats->batc_block_count=block_cache->root.batc_block_count;
	drive->stats->nda3_block_count=block_cache->root.nda3_block_count;
	drive->stats->nda2_block_count=block_cache->root.nda2_block_count;
	drive->stats->nda1_block_count=block_cache->root.nda1_block_count;
	drive->stats->nfda_block_count=block_cache->root.nfda_block_count;
	drive->stats->data_block_count=block_cache->root.data_block_count;
}



_Bool kfs_format_drive(const drive_t* drive,const void* boot,u32 boot_length){
	LOG("Formatting drive '%s' as KFS...",drive->model_number);
	if (DRIVE_BLOCK_SIZE_SHIFT!=DRIVE_BLOCK_SIZE_SHIFT){
		WARN("KFS requires block_size to be equal to %u bytes",1<<DRIVE_BLOCK_SIZE_SHIFT);
		return 0;
	}
	u64 block_count=drive->block_count>>(12-DRIVE_BLOCK_SIZE_SHIFT);
	if (block_count>0xffffffff){
		block_count=0xffffffff;
	}
	INFO("%lu total blocks, %lu BATC blocks",block_count,(block_count+KFS_BATC_BLOCK_COUNT-1)/KFS_BATC_BLOCK_COUNT);
	if (boot_length){
		INFO("Writing %v of boot code...",((boot_length+(1<<DRIVE_BLOCK_SIZE_SHIFT)-1)>>DRIVE_BLOCK_SIZE_SHIFT)<<DRIVE_BLOCK_SIZE_SHIFT);
		if (drive->read_write(drive->extra_data,DRIVE_OFFSET_FLAG_WRITE,(void*)boot,(boot_length+(1<<DRIVE_BLOCK_SIZE_SHIFT)-1)>>DRIVE_BLOCK_SIZE_SHIFT)!=((boot_length+(1<<DRIVE_BLOCK_SIZE_SHIFT)-1)>>DRIVE_BLOCK_SIZE_SHIFT)){
			ERROR("Error writing boot code to drive");
			return 0;
		}
	}
	kfs_large_block_index_t first_free_block_index=DRIVE_FIRST_FREE_BLOCK_INDEX;
	kfs_root_block_t* root=(void*)pmm_alloc_zero(1,PMM_COUNTER_KFS,0);
	root->signature=KFS_SIGNATURE;
	root->block_count=block_count;
	root->batc_block_index=first_free_block_index;
	root->root_block_count=1<<(12-DRIVE_BLOCK_SIZE_SHIFT);
	root->batc_block_count=((block_count+KFS_BATC_BLOCK_COUNT-1)/KFS_BATC_BLOCK_COUNT)<<(15-DRIVE_BLOCK_SIZE_SHIFT);
	INFO("Writing BATC blocks...");
	kfs_batc_block_t* batc=(void*)pmm_alloc_zero(8,PMM_COUNTER_KFS,0);
	batc->bitmap3=0x7fffffffffffffffull;
	for (u8 i=0;i<62;i++){
		batc->bitmap2[i]=0xffffffffffffffffull;
	}
	batc->bitmap2[62]=0x7fffffffffffffffull;
	for (u16 i=0;i<4031;i++){
		batc->bitmap1[i]=0xffffffffffffffffull;
	}
	for (kfs_large_block_index_t processed_block_count=0;processed_block_count<block_count;processed_block_count+=KFS_BATC_BLOCK_COUNT){
		batc->block_index=first_free_block_index;
		batc->first_block_index=processed_block_count;
		first_free_block_index+=8;
		if (processed_block_count+KFS_BATC_BLOCK_COUNT>block_count){
			batc->bitmap3=0;
			for (u8 i=0;i<63;i++){
				batc->bitmap2[i]=0;
			}
			for (u16 i=0;i<4031;i++){
				batc->bitmap1[i]=0;
			}
			for (u32 i=0;i<block_count-processed_block_count;i++){
				batc->bitmap1[i>>6]|=1ull<<(i&63);
			}
			for (u16 i=0;i<4030;i++){
				if (!batc->bitmap1[i]){
					break;
				}
				batc->bitmap2[i>>6]|=1ull<<(i&63);
			}
			for (u8 i=0;i<63;i++){
				if (!batc->bitmap2[i]){
					break;
				}
				batc->bitmap3|=1ull<<i;
			}
		}
		_drive_write(drive,batc->block_index,batc,8);
	}
	INFO("Reserving header blocks...");
	_drive_read(drive,root->batc_block_index,batc,8);
	while (first_free_block_index){
		first_free_block_index--;
		batc->bitmap1[first_free_block_index>>6]&=~(1ull<<(first_free_block_index&63));
	}
	for (u16 i=0;i<4030;i++){
		if (batc->bitmap1[i]){
			break;
		}
		batc->bitmap2[i>>6]&=~(1ull<<(i&63));
	}
	for (u8 i=0;i<63;i++){
		if (batc->bitmap2[i]){
			break;
		}
		batc->bitmap3&=~(1ull<<i);
	}
	_drive_write(drive,root->batc_block_index,batc,8);
	pmm_dealloc((u64)batc,8,PMM_COUNTER_KFS);
	INFO("Writing ROOT block...");
	u64 write_count=drive->read_write(drive->extra_data,1|DRIVE_OFFSET_FLAG_WRITE,root,sizeof(kfs_root_block_t)>>DRIVE_BLOCK_SIZE_SHIFT);
	pmm_dealloc((u64)root,1,PMM_COUNTER_KFS);
	if (write_count!=(sizeof(kfs_root_block_t)>>DRIVE_BLOCK_SIZE_SHIFT)){
		ERROR("Error writing data to drive");
		return 0;
	}
	LOG("Drive '%s' successfully formatted as KFS",drive->model_number);
	return 1;
}
