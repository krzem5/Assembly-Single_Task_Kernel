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



typedef struct _KFS2_FS_EXTRA_DATA{
	kfs2_root_block_t root_block;
	u32 block_size_shift;
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
	kfs2_node_t* node=(void*)(buffer+KFS2_INODE_GET_NODE_INDEX(inode)*sizeof(kfs2_node_t));
	if (drive_read(drive,partition->start_lba+((extra_data->root_block.first_inode_block+KFS2_INODE_GET_BLOCK_INDEX(inode))<<extra_data->block_size_shift),buffer,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)||!kfs2_verify_crc(node,sizeof(kfs2_node_t))){
		goto _cleanup;
	}
	out=(kfs2_vfs_node_t*)vfs_node_create(fs,name);
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
_cleanup:
	pmm_dealloc(((u64)buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,_kfs2_buffer_pmm_counter);
	return (vfs_node_t*)out;
}



static void _read_data_block(filesystem_t* fs,u64 block_index,void* buffer){
	kfs2_fs_extra_data_t* extra_data=fs->extra_data;
	partition_t* partition=fs->partition;
	drive_t* drive=partition->drive;
	if (drive_read(drive,partition->start_lba+((extra_data->root_block.first_data_block+block_index)<<extra_data->block_size_shift),buffer,1<<extra_data->block_size_shift)!=(1<<extra_data->block_size_shift)){
		panic("_read_data_block: I/O error");
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
				_read_data_block(node->node.fs,node->kfs2_node.data.single[index],out->data);
				out->length=KFS2_BLOCK_SIZE;
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
				_read_data_block(node->node.fs,out->double_cache[index],out->data);
				out->length=KFS2_BLOCK_SIZE;
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



static vfs_node_t* _kfs2_create(vfs_node_t* parent,const string_t* name,u32 flags){
	if (parent||name){
		panic("_kfs2_create: unimplemented");
	}
	kfs2_vfs_node_t* out=omm_alloc(_kfs2_vfs_node_allocator);
	out->kfs2_node._inode=0xffffffff;
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
	panic("_kfs2_flush");
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
