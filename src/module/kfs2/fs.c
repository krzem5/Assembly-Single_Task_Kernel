#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/keyring/master_key.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kfs2/crc.h>
#include <kfs2/structures.h>
#define KERNEL_LOG_NAME "kfs2"



typedef struct _KFS2_BITMAP_ALLOCATOR_CACHE_ENTRY{
	u64 offset;
	u64 block_index;
	u64* data;
} kfs2_bitmap_allocator_cache_entry_t;



typedef struct _KFS2_BITMAP_ALLOCATOR{
	spinlock_t lock;
	u64 bitmap_offset;
	u32 highest_level_length;
	u32 highest_level_offset;
	kfs2_bitmap_allocator_cache_entry_t cache[KFS2_BITMAP_LEVEL_COUNT];
} kfs2_bitmap_allocator_t;



typedef struct _KFS2_FS_EXTRA_DATA{
	kfs2_root_block_t root_block;
	u32 block_size_shift;
	kfs2_bitmap_allocator_t data_block_allocator;
	kfs2_bitmap_allocator_t inode_allocator;
} kfs2_fs_extra_data_t;



typedef struct _KFS2_VFS_NODE{
	vfs_node_t node;
	kfs2_node_t kfs2_node;
} kfs2_vfs_node_t;



static pmm_counter_descriptor_t* _kfs2_buffer_pmm_counter=NULL;
static pmm_counter_descriptor_t* _kfs2_chunk_pmm_counter=NULL;
static omm_allocator_t* _kfs2_vfs_node_allocator=NULL;
static omm_allocator_t* _kfs2_fs_extra_data_allocator=NULL;
static filesystem_descriptor_t* _kfs2_filesystem_descriptor=NULL;



static KERNEL_INLINE u8 _calculate_compressed_hash(const string_t* name){
	u16 tmp=name->hash^(name->hash>>16);
	return tmp^(tmp>>8);
}



static vfs_node_t* _load_inode(filesystem_t* fs,const string_t* name,u32 inode){
	void* buffer=(void*)(pmm_alloc(1,_kfs2_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	partition_t* partition=fs->partition;
	drive_t* drive=partition->drive;
	kfs2_fs_extra_data_t* extra_data=fs->extra_data;
	kfs2_vfs_node_t* out=NULL;
	if (drive_read(drive,partition->start_lba+((extra_data->root_block.first_inode_block+KFS2_INODE_GET_BLOCK_INDEX(inode))<<extra_data->block_size_shift),buffer,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		panic("_load_inode: I/O error");
	}
	kfs2_node_t* node=(void*)(buffer+KFS2_INODE_GET_NODE_INDEX(inode)*sizeof(kfs2_node_t));
	if (!kfs2_verify_crc(node,sizeof(kfs2_node_t))){
		panic("_load_inode: invlaid CRC");
	}
	out=(kfs2_vfs_node_t*)vfs_node_create(fs,NULL,name,0);
	if ((node->flags&KFS2_INODE_TYPE_MASK)==KFS2_INODE_TYPE_DIRECTORY){
		out->node.flags|=VFS_NODE_TYPE_DIRECTORY;
	}
	else if ((node->flags&KFS2_INODE_TYPE_MASK)==KFS2_INODE_TYPE_LINK){
		out->node.flags|=VFS_NODE_TYPE_LINK;
	}
	else{
		out->node.flags|=VFS_NODE_TYPE_FILE;
	}
	out->node.flags|=((node->flags&KFS2_INODE_PERMISSION_MASK)>>KFS2_INODE_PERMISSION_SHIFT)<<VFS_NODE_PERMISSION_SHIFT;
	out->node.time_access=node->time_access;
	out->node.time_modify=node->time_modify;
	out->node.time_change=node->time_change;
	out->node.time_birth=node->time_birth;
	out->node.gid=node->gid;
	out->node.uid=node->uid;
	out->kfs2_node=*node;
	out->kfs2_node._inode=inode;
	pmm_dealloc(((u64)buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,_kfs2_buffer_pmm_counter);
	return (vfs_node_t*)out;
}



static void _store_inode(kfs2_vfs_node_t* node){
	void* buffer=(void*)(pmm_alloc(1,_kfs2_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	partition_t* partition=node->node.fs->partition;
	drive_t* drive=partition->drive;
	kfs2_fs_extra_data_t* extra_data=node->node.fs->extra_data;
	if (drive_read(drive,partition->start_lba+((extra_data->root_block.first_inode_block+KFS2_INODE_GET_BLOCK_INDEX(node->kfs2_node._inode))<<extra_data->block_size_shift),buffer,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		panic("_store_inode: I/O error");
	}
	kfs2_node_t* tmp_node=(void*)(buffer+KFS2_INODE_GET_NODE_INDEX(node->kfs2_node._inode)*sizeof(kfs2_node_t));
	*tmp_node=node->kfs2_node;
	kfs2_insert_crc(tmp_node,sizeof(kfs2_node_t));
	if (drive_write(drive,partition->start_lba+((extra_data->root_block.first_inode_block+KFS2_INODE_GET_BLOCK_INDEX(node->kfs2_node._inode))<<extra_data->block_size_shift),buffer,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		panic("_store_inode: I/O error");
	}
	pmm_dealloc(((u64)buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,_kfs2_buffer_pmm_counter);
}



static void _read_data_block(filesystem_t* fs,u64 block_index,void* buffer){
	kfs2_fs_extra_data_t* extra_data=fs->extra_data;
	partition_t* partition=fs->partition;
	drive_t* drive=partition->drive;
	if (drive_read(drive,partition->start_lba+((extra_data->root_block.first_data_block+block_index)<<extra_data->block_size_shift),buffer,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		panic("_read_data_block: I/O error");
	}
}



static void _write_data_block(filesystem_t* fs,u64 block_index,const void* buffer){
	kfs2_fs_extra_data_t* extra_data=fs->extra_data;
	partition_t* partition=fs->partition;
	drive_t* drive=partition->drive;
	if (drive_write(drive,partition->start_lba+((extra_data->root_block.first_data_block+block_index)<<extra_data->block_size_shift),buffer,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		panic("_write_data_block: I/O error");
	}
}



static void _node_get_chunk_at_offset(kfs2_vfs_node_t* node,u64 offset,kfs2_data_chunk_t* out){
	switch (node->kfs2_node.flags&KFS2_INODE_STORAGE_MASK){
		case KFS2_INODE_STORAGE_TYPE_INLINE:
			if (offset>=48){
				panic("_node_get_chunk_at_offset: invalid offset");
			}
			out->offset=0;
			out->data=node->kfs2_node.data.inline_;
			out->length=48;
			out->data_offset=0;
			break;
		case KFS2_INODE_STORAGE_TYPE_SINGLE:
			{
				u64 index=offset/KFS2_BLOCK_SIZE;
				if (index>=6){
					panic("_node_get_chunk_at_offset: invalid offset");
				}
				if (!out->data){
					out->data=(void*)(pmm_alloc(1,_kfs2_chunk_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
				}
				out->offset=offset&(-KFS2_BLOCK_SIZE);
				out->data_offset=node->kfs2_node.data.single[index];
				out->length=KFS2_BLOCK_SIZE;
				_read_data_block(node->node.fs,out->data_offset,out->data);
				break;
			}
		case KFS2_INODE_STORAGE_TYPE_DOUBLE:
			{
				u64 index=offset/KFS2_BLOCK_SIZE;
				if (index>=KFS2_BLOCK_SIZE/sizeof(u64)){
					panic("_node_get_chunk_at_offset: invalid offset");
				}
				if (!out->double_cache){
					out->double_cache=(void*)(pmm_alloc(1,_kfs2_chunk_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
					_read_data_block(node->node.fs,node->kfs2_node.data.double_,out->double_cache);
				}
				if (!out->data){
					out->data=(void*)(pmm_alloc(1,_kfs2_chunk_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
				}
				out->offset=offset&(-KFS2_BLOCK_SIZE);
				out->data_offset=out->double_cache[index];
				out->length=KFS2_BLOCK_SIZE;
				_read_data_block(node->node.fs,out->data_offset,out->data);
				break;
			}
		case KFS2_INODE_STORAGE_TYPE_TRIPLE:
			panic("KFS2_INODE_STORAGE_TYPE_TRIPLE");
			break;
		case KFS2_INODE_STORAGE_TYPE_QUADRUPLE:
			panic("KFS2_INODE_STORAGE_TYPE_QUADRUPLE");
			break;
	}
}



static void _node_set_chunk(kfs2_vfs_node_t* node,kfs2_data_chunk_t* chunk){
	if ((node->kfs2_node.flags&KFS2_INODE_STORAGE_MASK)==KFS2_INODE_STORAGE_TYPE_INLINE){
		_store_inode(node);
	}
	else{
		_write_data_block(node->node.fs,chunk->data_offset,chunk->data);
	}
}



static void _node_dealloc_chunk(kfs2_data_chunk_t* chunk){
	chunk->offset=0;
	if (chunk->quadruple_cache){
		pmm_dealloc(((u64)(chunk->quadruple_cache))-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,_kfs2_chunk_pmm_counter);
		chunk->quadruple_cache=NULL;
	}
	if (chunk->triple_cache){
		pmm_dealloc(((u64)(chunk->triple_cache))-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,_kfs2_chunk_pmm_counter);
		chunk->triple_cache=NULL;
	}
	if (chunk->double_cache){
		pmm_dealloc(((u64)(chunk->double_cache))-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,_kfs2_chunk_pmm_counter);
		chunk->double_cache=NULL;
	}
	if (chunk->data&&chunk->length==KFS2_BLOCK_SIZE){
		pmm_dealloc(((u64)(chunk->data))-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,_kfs2_chunk_pmm_counter);
	}
	chunk->data=NULL;
	chunk->length=0;
}



static void _bitmap_allocator_store_data(filesystem_t* fs,kfs2_bitmap_allocator_t* allocator,u32 level){
	kfs2_fs_extra_data_t* extra_data=fs->extra_data;
	partition_t* partition=fs->partition;
	drive_t* drive=partition->drive;
	if (drive_write(drive,partition->start_lba+((allocator->cache+level)->block_index<<extra_data->block_size_shift),(allocator->cache+level)->data,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		panic("_bitmap_allocator_store_data: I/O error");
	}
}



static u64* _bitmap_allocator_fetch_data(filesystem_t* fs,kfs2_bitmap_allocator_t* allocator,u32 level,u64 offset){
	u64 block_index=(allocator->cache+level)->offset+offset*sizeof(u64)/KFS2_BLOCK_SIZE;
	if ((allocator->cache+level)->block_index!=block_index){
		kfs2_fs_extra_data_t* extra_data=fs->extra_data;
		partition_t* partition=fs->partition;
		drive_t* drive=partition->drive;
		if (drive_read(drive,partition->start_lba+(block_index<<extra_data->block_size_shift),(allocator->cache+level)->data,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
			panic("_bitmap_allocator_fetch_data: I/O error");
		}
		(allocator->cache+level)->block_index=block_index;
	}
	return (allocator->cache+level)->data+(offset&(KFS2_BLOCK_SIZE/sizeof(u64)-1));
}



static void _bitmap_allocator_find_next_highest_level_offset(filesystem_t* fs,kfs2_bitmap_allocator_t* allocator){
	for (;allocator->highest_level_offset<allocator->highest_level_length&&!(*_bitmap_allocator_fetch_data(fs,allocator,KFS2_BITMAP_LEVEL_COUNT-1,allocator->highest_level_offset));allocator->highest_level_offset++);
}



static void _bitmap_allocator_init(filesystem_t* fs,kfs2_bitmap_allocator_t* allocator,const u64* bitmap_offsets,u32 highest_level_length){
	spinlock_init(&(allocator->lock));
	allocator->highest_level_length=highest_level_length;
	allocator->highest_level_offset=0;
	for (u32 i=0;i<KFS2_BITMAP_LEVEL_COUNT;i++){
		(allocator->cache+i)->offset=bitmap_offsets[i];
		(allocator->cache+i)->block_index=0xffffffffffffffffull;
		(allocator->cache+i)->data=(void*)(pmm_alloc(1,_kfs2_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	}
	_bitmap_allocator_find_next_highest_level_offset(fs,allocator);
}



static u64 _bitmap_allocator_alloc(filesystem_t* fs,kfs2_bitmap_allocator_t* allocator){
	spinlock_acquire_exclusive(&(allocator->lock));
	if (allocator->highest_level_offset==allocator->highest_level_length){
		spinlock_release_exclusive(&(allocator->lock));
		return 0;
	}
	u64 out=allocator->highest_level_offset;
	u64* ptr[KFS2_BITMAP_LEVEL_COUNT];
	for (u32 i=KFS2_BITMAP_LEVEL_COUNT;i;){
		i--;
		ptr[i]=_bitmap_allocator_fetch_data(fs,allocator,i,out);
		out=(out<<6)|(__builtin_ffsll(*(ptr[i]))-1);
	}
	u32 i=0;
	for (;i<KFS2_BITMAP_LEVEL_COUNT;i++){
		*(ptr[i])&=(*(ptr[i]))-1;
		_bitmap_allocator_store_data(fs,allocator,i);
		if (*(ptr[i])){
			break;
		}
	}
	if (i==KFS2_BITMAP_LEVEL_COUNT){
		allocator->highest_level_offset++;
		_bitmap_allocator_find_next_highest_level_offset(fs,allocator);
	}
	spinlock_release_exclusive(&(allocator->lock));
	return out;
}



static void _node_migrate_data_inline_inline(kfs2_vfs_node_t* node,u64 size){
	panic("_node_migrate_data_inline_inline");
}



static void _node_migrate_data_inline_single(kfs2_vfs_node_t* node,u64 size){
	kfs2_node_data_t old_data=node->kfs2_node.data;
	kfs2_fs_extra_data_t* extra_data=node->node.fs->extra_data;
	node->kfs2_node.data.single[0]=_bitmap_allocator_alloc(node->node.fs,&(extra_data->data_block_allocator));
	for (u32 i=1;i<6;i++){
		node->kfs2_node.data.single[i]=0;
	}
	void* buffer=(void*)(pmm_alloc(1,_kfs2_chunk_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	mem_copy(buffer,old_data.inline_,node->kfs2_node.size);
	_write_data_block(node->node.fs,node->kfs2_node.data.single[0],buffer);
	pmm_dealloc(((u64)buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,_kfs2_chunk_pmm_counter);
}



static void _node_resize(kfs2_vfs_node_t* node,u64 size){
	if (node->kfs2_node.size==size){
		return;
	}
	u32 new_storage_type;
	if (size<=48){
		new_storage_type=KFS2_INODE_STORAGE_TYPE_INLINE;
	}
	else if (size<=6*KFS2_BLOCK_SIZE){
		new_storage_type=KFS2_INODE_STORAGE_TYPE_SINGLE;
	}
	else if (size<=((u64)KFS2_BLOCK_SIZE)*KFS2_BLOCK_SIZE/8){
		new_storage_type=KFS2_INODE_STORAGE_TYPE_DOUBLE;
	}
	else if (size<=((u64)KFS2_BLOCK_SIZE)*KFS2_BLOCK_SIZE*KFS2_BLOCK_SIZE/64){
		new_storage_type=KFS2_INODE_STORAGE_TYPE_TRIPLE;
	}
	else if (size<=((u64)KFS2_BLOCK_SIZE)*KFS2_BLOCK_SIZE*KFS2_BLOCK_SIZE*KFS2_BLOCK_SIZE/512){
		new_storage_type=KFS2_INODE_STORAGE_TYPE_QUADRUPLE;
	}
	else{
		panic("_node_resize: File too large");
	}
	u32 old_storage_type=node->kfs2_node.flags&KFS2_INODE_STORAGE_MASK;
	if (old_storage_type==KFS2_INODE_STORAGE_TYPE_INLINE&&new_storage_type==KFS2_INODE_STORAGE_TYPE_INLINE){
		_node_migrate_data_inline_inline(node,size);
	}
	else if (old_storage_type==KFS2_INODE_STORAGE_TYPE_INLINE&&new_storage_type==KFS2_INODE_STORAGE_TYPE_SINGLE){
		_node_migrate_data_inline_single(node,size);
	}
	else{
		panic("_node_resize");
	}
	node->kfs2_node.size=size;
	node->kfs2_node.flags=(node->kfs2_node.flags&(~KFS2_INODE_STORAGE_MASK))|new_storage_type;
	_store_inode(node);
}



static void _attach_child(kfs2_vfs_node_t* parent,const string_t* name,kfs2_vfs_node_t* child){
	u16 new_entry_size=(sizeof(kfs2_directory_entry_t)+name->length+3)&0xfffc;
	kfs2_data_chunk_t chunk={
		0,
		NULL,
		NULL,
		NULL,
		NULL,
		0,
		0
	};
	for (u64 offset=0;offset<parent->kfs2_node.size;){
		if (offset-chunk.offset>=chunk.length){
			_node_get_chunk_at_offset(parent,offset,&chunk);
		}
		kfs2_directory_entry_t* entry=(kfs2_directory_entry_t*)(chunk.data+offset-chunk.offset);
		if (entry->name_length||entry->size<new_entry_size){
			offset+=entry->size;
			continue;
		}
		entry->inode=child->kfs2_node._inode;
		entry->name_length=name->length;
		entry->name_compressed_hash=_calculate_compressed_hash(name);
		mem_copy(entry->name,name->data,name->length);
		if (entry->size-new_entry_size>=12){
			kfs2_directory_entry_t* next_entry=(kfs2_directory_entry_t*)(chunk.data+offset-chunk.offset+new_entry_size);
			next_entry->inode=0;
			next_entry->size=entry->size-new_entry_size;
			next_entry->name_length=0;
			entry->size=new_entry_size;
		}
		_node_set_chunk(parent,&chunk);
		_node_dealloc_chunk(&chunk);
		return;
	}
	_node_dealloc_chunk(&chunk);
	_node_resize(parent,(parent->kfs2_node.size==48?KFS2_BLOCK_SIZE:parent->kfs2_node.size+KFS2_BLOCK_SIZE));
	_node_get_chunk_at_offset(parent,parent->kfs2_node.size-KFS2_BLOCK_SIZE,&chunk);
	if (parent->kfs2_node.size>KFS2_BLOCK_SIZE){
		panic("_attach_child: update large parent");
		_node_dealloc_chunk(&chunk);
		return;
	}
	kfs2_directory_entry_t* entry;
	u64 offset=0;
	while (offset<48){
		entry=(kfs2_directory_entry_t*)(chunk.data+offset);
		offset+=entry->size;
	}
	if (!entry->name_length){
		offset-=entry->size;
		entry->size+=KFS2_BLOCK_SIZE-offset;
	}
	else{
		entry=(kfs2_directory_entry_t*)(chunk.data+offset);
		entry->size=KFS2_BLOCK_SIZE-offset;
	}
	entry->inode=child->kfs2_node._inode;
	entry->name_length=name->length;
	entry->name_compressed_hash=_calculate_compressed_hash(name);
	mem_copy(entry->name,name->data,name->length);
	kfs2_directory_entry_t* next_entry=(kfs2_directory_entry_t*)(chunk.data+offset+new_entry_size);
	next_entry->inode=0;
	next_entry->size=entry->size-new_entry_size;
	next_entry->name_length=0;
	entry->size=new_entry_size;
	_node_set_chunk(parent,&chunk);
	_node_dealloc_chunk(&chunk);
}



static vfs_node_t* _kfs2_create(vfs_node_t* parent,const string_t* name,u32 flags){
	kfs2_vfs_node_t* out=omm_alloc(_kfs2_vfs_node_allocator);
	out->kfs2_node._inode=0xffffffff;
	if (!(flags&VFS_NODE_FLAG_CREATE)){
		return (vfs_node_t*)out;
	}
	if ((parent->flags&VFS_NODE_TYPE_MASK)!=VFS_NODE_TYPE_DIRECTORY||name->length>255){
		omm_dealloc(_kfs2_vfs_node_allocator,out);
		return NULL;
	}
	filesystem_t* fs=parent->fs;
	kfs2_fs_extra_data_t* extra_data=fs->extra_data;
	out->kfs2_node._inode=_bitmap_allocator_alloc(fs,&(extra_data->inode_allocator));
	if (!out->kfs2_node._inode){
		omm_dealloc(_kfs2_vfs_node_allocator,out);
		return NULL;
	}
	if (flags&VFS_NODE_TYPE_DIRECTORY){
		out->kfs2_node.size=48;
		out->kfs2_node.flags=KFS2_INODE_TYPE_DIRECTORY|KFS2_INODE_STORAGE_TYPE_INLINE;
		kfs2_directory_entry_t* entry=(kfs2_directory_entry_t*)(out->kfs2_node.data.inline_);
		entry->inode=0;
		entry->size=48;
		entry->name_length=0;
	}
	else{
		out->kfs2_node.size=0;
		out->kfs2_node.flags=KFS2_INODE_TYPE_FILE|KFS2_INODE_STORAGE_TYPE_INLINE;
	}
	out->kfs2_node.time_access=0;
	out->kfs2_node.time_modify=0;
	out->kfs2_node.time_change=0;
	out->kfs2_node.time_birth=0;
	out->kfs2_node.gid=0;
	out->kfs2_node.uid=0;
	_attach_child((kfs2_vfs_node_t*)parent,name,out);
	return (vfs_node_t*)out;
}



static void _kfs2_delete(vfs_node_t* node){
	omm_dealloc(_kfs2_vfs_node_allocator,node);
}



static vfs_node_t* _kfs2_lookup(vfs_node_t* node,const string_t* name){
	kfs2_vfs_node_t* kfs2_node=(kfs2_vfs_node_t*)node;
	if (kfs2_node->kfs2_node._inode==0xffffffff||!kfs2_node->kfs2_node.size||(kfs2_node->kfs2_node.flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_DIRECTORY){
		return NULL;
	}
	kfs2_data_chunk_t chunk={
		0,
		NULL,
		NULL,
		NULL,
		NULL,
		0,
		0
	};
	u64 offset=0;
	u8 compressed_hash=_calculate_compressed_hash(name);
	while (offset<kfs2_node->kfs2_node.size){
		if (offset-chunk.offset>=chunk.length){
			_node_get_chunk_at_offset(kfs2_node,offset,&chunk);
		}
		kfs2_directory_entry_t* entry=(kfs2_directory_entry_t*)(chunk.data+offset-chunk.offset);
		if (entry->name_length!=name->length||entry->name_compressed_hash!=compressed_hash){
			goto _skip_entry;
		}
		for (u16 i=0;i<name->length;i++){
			if (entry->name[i]!=name->data[i]){
				goto _skip_entry;
			}
		}
		u64 inode=entry->inode;
		_node_dealloc_chunk(&chunk);
		return _load_inode(node->fs,name,inode);
_skip_entry:
		offset+=entry->size;
	}
	_node_dealloc_chunk(&chunk);
	return NULL;
}



static u64 _kfs2_iterate(vfs_node_t* node,u64 pointer,string_t** out){
	kfs2_vfs_node_t* kfs2_node=(kfs2_vfs_node_t*)node;
	if (kfs2_node->kfs2_node._inode==0xffffffff||pointer>=kfs2_node->kfs2_node.size||(kfs2_node->kfs2_node.flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_DIRECTORY){
		return 0;
	}
	kfs2_data_chunk_t chunk={
		0,
		NULL,
		NULL,
		NULL,
		NULL,
		0,
		0
	};
	while (pointer<kfs2_node->kfs2_node.size){
		if (pointer-chunk.offset>=chunk.length){
			_node_get_chunk_at_offset(kfs2_node,pointer,&chunk);
		}
		kfs2_directory_entry_t* entry=(kfs2_directory_entry_t*)(chunk.data+pointer-chunk.offset);
		pointer+=entry->size;
		if (entry->name_length){
			*out=smm_alloc(entry->name,entry->name_length);
			_node_dealloc_chunk(&chunk);
			return pointer;
		}
	}
	_node_dealloc_chunk(&chunk);
	return 0;
}



static _Bool _kfs2_link(vfs_node_t* node,vfs_node_t* parent){
	panic("_kfs2_link");
}



static _Bool _kfs2_unlink(vfs_node_t* node){
	panic("_kfs2_unlink");
}



static u64 _kfs2_read(vfs_node_t* node,u64 offset,void* buffer,u64 size,u32 flags){
	kfs2_vfs_node_t* kfs2_node=(kfs2_vfs_node_t*)node;
	if (kfs2_node->kfs2_node._inode==0xffffffff||offset>=kfs2_node->kfs2_node.size){
		return 0;
	}
	if (offset+size>=kfs2_node->kfs2_node.size){
		size=kfs2_node->kfs2_node.size-offset;
	}
	if (!size){
		return 0;
	}
	u64 out=size;
	kfs2_data_chunk_t chunk={
		0,
		NULL,
		NULL,
		NULL,
		NULL,
		0,
		0
	};
	size+=offset;
	while (offset<size){
		if (offset-chunk.offset>=chunk.length){
			_node_get_chunk_at_offset(kfs2_node,offset,&chunk);
		}
		u64 padding=offset-chunk.offset;
		u64 read_size=(chunk.length-padding>size-offset?size-offset:chunk.length-padding);
		mem_copy(buffer,chunk.data+padding,read_size);
		buffer+=read_size;
		offset+=chunk.length-padding;
	}
	_node_dealloc_chunk(&chunk);
	return out;
}



static u64 _kfs2_write(vfs_node_t* node,u64 offset,const void* buffer,u64 size,u32 flags){
	panic("_kfs2_write");
}



static u64 _kfs2_resize(vfs_node_t* node,s64 size,u32 flags){
	kfs2_vfs_node_t* kfs2_node=(kfs2_vfs_node_t*)node;
	if (kfs2_node->kfs2_node._inode==0xffffffff){
		return 0;
	}
	if (flags&VFS_NODE_FLAG_RESIZE_RELATIVE){
		if (!size){
			return kfs2_node->kfs2_node.size;
		}
	}
	panic("_kfs2_resize");
}



static void _kfs2_flush(vfs_node_t* node){
	if (!(node->flags&VFS_NODE_FLAG_DIRTY)){
		return;
	}
	kfs2_vfs_node_t* kfs2_node=(kfs2_vfs_node_t*)node;
	kfs2_node->kfs2_node.flags&=~(KFS2_INODE_TYPE_MASK|KFS2_INODE_PERMISSION_MASK);
	if ((kfs2_node->node.flags&KFS2_INODE_TYPE_MASK)==VFS_NODE_TYPE_FILE){
		kfs2_node->kfs2_node.flags|=KFS2_INODE_TYPE_FILE;
	}
	else if ((kfs2_node->node.flags&KFS2_INODE_TYPE_MASK)==VFS_NODE_TYPE_DIRECTORY){
		kfs2_node->kfs2_node.flags|=KFS2_INODE_TYPE_DIRECTORY;
	}
	else if ((kfs2_node->node.flags&KFS2_INODE_TYPE_MASK)==VFS_NODE_TYPE_LINK){
		kfs2_node->kfs2_node.flags|=KFS2_INODE_TYPE_LINK;
	}
	else{
		panic("_kfs2_flush: invalid node type");
	}
	kfs2_node->kfs2_node.flags|=((kfs2_node->node.flags&VFS_NODE_PERMISSION_MASK)>>VFS_NODE_PERMISSION_SHIFT)<<KFS2_INODE_PERMISSION_SHIFT;
	kfs2_node->kfs2_node.time_access=kfs2_node->node.time_access;
	kfs2_node->kfs2_node.time_modify=kfs2_node->node.time_modify;
	kfs2_node->kfs2_node.time_change=kfs2_node->node.time_change;
	kfs2_node->kfs2_node.time_birth=kfs2_node->node.time_birth;
	kfs2_node->kfs2_node.gid=kfs2_node->node.gid;
	kfs2_node->kfs2_node.uid=kfs2_node->node.uid;
	_store_inode(kfs2_node);
	node->flags&=~VFS_NODE_FLAG_DIRTY;
}



static const vfs_functions_t _kfs2_functions={
	_kfs2_create,
	_kfs2_delete,
	_kfs2_lookup,
	_kfs2_iterate,
	_kfs2_link,
	_kfs2_unlink,
	_kfs2_read,
	_kfs2_write,
	_kfs2_resize,
	_kfs2_flush
};



static void _kfs2_fs_deinit(filesystem_t* fs){
	omm_dealloc(_kfs2_fs_extra_data_allocator,fs->extra_data);
	panic("_kfs2_deinit_callback");
}



static filesystem_t* _kfs2_fs_load(partition_t* partition){
	drive_t* drive=partition->drive;
	if (drive->block_size>4096){
		return NULL;
	}
	u8 buffer[4096];
	if (drive_read(drive,partition->start_lba,buffer,1)!=1){
		return NULL;
	}
	kfs2_root_block_t* root_block=(kfs2_root_block_t*)buffer;
	if (root_block->signature!=KFS2_ROOT_BLOCK_SIGNATURE||!kfs2_verify_crc(root_block,sizeof(kfs2_root_block_t))){
		return NULL;
	}
	kfs2_fs_extra_data_t* extra_data=omm_alloc(_kfs2_fs_extra_data_allocator);
	extra_data->root_block=*root_block;
	extra_data->block_size_shift=63-__builtin_clzll(KFS2_BLOCK_SIZE/drive->block_size);
	filesystem_t* out=fs_create(_kfs2_filesystem_descriptor);
	out->functions=&_kfs2_functions;
	out->partition=partition;
	out->extra_data=extra_data;
	_bitmap_allocator_init(out,&(extra_data->data_block_allocator),extra_data->root_block.data_block_allocation_bitmap_offsets,extra_data->root_block.data_block_allocation_bitmap_highest_level_length);
	_bitmap_allocator_init(out,&(extra_data->inode_allocator),extra_data->root_block.inode_allocation_bitmap_offsets,extra_data->root_block.inode_allocation_bitmap_highest_level_length);
	SMM_TEMPORARY_STRING root_name=smm_alloc("",0);
	out->root=_load_inode(out,root_name,0);
	out->root->flags|=VFS_NODE_FLAG_PERMANENT;
	mem_copy(out->guid,root_block->uuid,16);
	return out;
}



static void _kfs2_fs_mount(filesystem_t* fs,const char* path){
	if (!path||!str_equal(path,"/")){
		return;
	}
	kfs2_fs_extra_data_t* extra_data=fs->extra_data;
	keyring_master_key_get_encrypted(extra_data->root_block.master_key,sizeof(extra_data->root_block.master_key));
	kfs2_insert_crc(&(extra_data->root_block),sizeof(kfs2_root_block_t));
	drive_write(fs->partition->drive,fs->partition->start_lba,&(extra_data->root_block),1);
}



static const filesystem_descriptor_config_t _kfs2_filesystem_descriptor_config={
	"kfs2",
	_kfs2_fs_deinit,
	_kfs2_fs_load,
	_kfs2_fs_mount
};



void kfs2_register_fs(void){
	_kfs2_filesystem_descriptor=fs_register_descriptor(&_kfs2_filesystem_descriptor_config);
	_kfs2_buffer_pmm_counter=pmm_alloc_counter("kfs2_buffer");
	_kfs2_chunk_pmm_counter=pmm_alloc_counter("kfs2_chunk");
	_kfs2_vfs_node_allocator=omm_init("kfs2_node",sizeof(kfs2_vfs_node_t),8,4,pmm_alloc_counter("omm_kfs2_node"));
	spinlock_init(&(_kfs2_vfs_node_allocator->lock));
	_kfs2_fs_extra_data_allocator=omm_init("kfs2_extra_data",sizeof(kfs2_fs_extra_data_t),8,1,pmm_alloc_counter("omm_kfs2_extra_data"));
	spinlock_init(&(_kfs2_fs_extra_data_allocator->lock));
}



KERNEL_PUBLIC void kfs2_update_root_block_master_key(filesystem_t* fs){
	panic("kfs2_update_root_block_master_key");
}
