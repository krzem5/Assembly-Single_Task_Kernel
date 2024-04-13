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
#include <kfs2/bitmap.h>
#include <kfs2/chunk.h>
#include <kfs2/crc.h>
#include <kfs2/io.h>
#include <kfs2/structures.h>
#define KERNEL_LOG_NAME "kfs2"



static pmm_counter_descriptor_t* _kfs2_resize_buffer_pmm_counter=NULL;
static pmm_counter_descriptor_t* _kfs2_root_block_buffer_pmm_counter=NULL;
static omm_allocator_t* _kfs2_vfs_node_allocator=NULL;
static omm_allocator_t* _kfs2_fs_extra_data_allocator=NULL;
static filesystem_descriptor_t* _kfs2_filesystem_descriptor=NULL;



static KERNEL_INLINE u8 _calculate_compressed_hash(const string_t* name){
	u16 tmp=name->hash^(name->hash>>16);
	return tmp^(tmp>>8);
}



static void _node_resize_data_inline_inline(kfs2_vfs_node_t* node,u64 size){
	panic("_node_resize_data_inline_inline");
}



static void _node_resize_data_inline_single(kfs2_vfs_node_t* node,u64 size){
	kfs2_node_data_t old_data=node->kfs2_node.data;
	kfs2_fs_extra_data_t* extra_data=node->node.fs->extra_data;
	node->kfs2_node.data.single[0]=kfs2_bitmap_alloc(node->node.fs,&(extra_data->data_block_allocator));
	for (u32 i=1;i<6;i++){
		node->kfs2_node.data.single[i]=0;
	}
	void* buffer=(void*)(pmm_alloc(1,_kfs2_resize_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	mem_copy(buffer,old_data.inline_,node->kfs2_node.size);
	kfs2_io_data_block_write(node->node.fs,node->kfs2_node.data.single[0],buffer);
	pmm_dealloc(((u64)buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,_kfs2_resize_buffer_pmm_counter);
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
		_node_resize_data_inline_inline(node,size);
	}
	else if (old_storage_type==KFS2_INODE_STORAGE_TYPE_INLINE&&new_storage_type==KFS2_INODE_STORAGE_TYPE_SINGLE){
		_node_resize_data_inline_single(node,size);
	}
	else{
		panic("_node_resize");
	}
	node->kfs2_node.size=size;
	node->kfs2_node.flags=(node->kfs2_node.flags&(~KFS2_INODE_STORAGE_MASK))|new_storage_type;
	kfs2_io_inode_write(node);
}



static void _attach_child(kfs2_vfs_node_t* parent,const string_t* name,kfs2_vfs_node_t* child){
	u16 new_entry_size=(sizeof(kfs2_directory_entry_t)+name->length+3)&0xfffc;
	kfs2_data_chunk_t chunk;
	kfs2_chunk_init(&chunk);
	for (u64 offset=0;offset<parent->kfs2_node.size;){
		if (offset-chunk.offset>=chunk.length){
			kfs2_chunk_read(parent,offset,&chunk);
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
		kfs2_chunk_write(parent,&chunk);
		kfs2_chunk_deinit(&chunk);
		return;
	}
	kfs2_chunk_deinit(&chunk);
	_node_resize(parent,(parent->kfs2_node.size==48?KFS2_BLOCK_SIZE:parent->kfs2_node.size+KFS2_BLOCK_SIZE));
	kfs2_chunk_read(parent,parent->kfs2_node.size-KFS2_BLOCK_SIZE,&chunk);
	if (parent->kfs2_node.size>KFS2_BLOCK_SIZE){
		panic("_attach_child: update large parent");
		kfs2_chunk_deinit(&chunk);
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
	kfs2_chunk_write(parent,&chunk);
	kfs2_chunk_deinit(&chunk);
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
	out->kfs2_node._inode=kfs2_bitmap_alloc(fs,&(extra_data->inode_allocator));
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
	kfs2_data_chunk_t chunk;
	kfs2_chunk_init(&chunk);
	u64 offset=0;
	u8 compressed_hash=_calculate_compressed_hash(name);
	while (offset<kfs2_node->kfs2_node.size){
		if (offset-chunk.offset>=chunk.length){
			kfs2_chunk_read(kfs2_node,offset,&chunk);
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
		kfs2_chunk_deinit(&chunk);
		return kfs2_io_inode_read(node->fs,name,inode);
_skip_entry:
		offset+=entry->size;
	}
	kfs2_chunk_deinit(&chunk);
	return NULL;
}



static u64 _kfs2_iterate(vfs_node_t* node,u64 pointer,string_t** out){
	kfs2_vfs_node_t* kfs2_node=(kfs2_vfs_node_t*)node;
	if (kfs2_node->kfs2_node._inode==0xffffffff||pointer>=kfs2_node->kfs2_node.size||(kfs2_node->kfs2_node.flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_DIRECTORY){
		return 0;
	}
	kfs2_data_chunk_t chunk;
	kfs2_chunk_init(&chunk);
	while (pointer<kfs2_node->kfs2_node.size){
		if (pointer-chunk.offset>=chunk.length){
			kfs2_chunk_read(kfs2_node,pointer,&chunk);
		}
		kfs2_directory_entry_t* entry=(kfs2_directory_entry_t*)(chunk.data+pointer-chunk.offset);
		pointer+=entry->size;
		if (entry->name_length){
			*out=smm_alloc(entry->name,entry->name_length);
			kfs2_chunk_deinit(&chunk);
			return pointer;
		}
	}
	kfs2_chunk_deinit(&chunk);
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
	kfs2_data_chunk_t chunk;
	kfs2_chunk_init(&chunk);
	size+=offset;
	while (offset<size){
		if (offset-chunk.offset>=chunk.length){
			kfs2_chunk_read(kfs2_node,offset,&chunk);
		}
		u64 padding=offset-chunk.offset;
		u64 read_size=(chunk.length-padding>size-offset?size-offset:chunk.length-padding);
		mem_copy(buffer,chunk.data+padding,read_size);
		buffer+=read_size;
		offset+=chunk.length-padding;
	}
	kfs2_chunk_deinit(&chunk);
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
		size+=kfs2_node->kfs2_node.size;
	}
	if (size<0){
		size=0;
	}
	_node_resize(kfs2_node,size);
	return kfs2_node->kfs2_node.size;
}



static void _kfs2_flush(vfs_node_t* node){
	if (!(node->flags&VFS_NODE_FLAG_DIRTY)){
		return;
	}
	kfs2_vfs_node_t* kfs2_node=(kfs2_vfs_node_t*)node;
	kfs2_node->kfs2_node.flags&=~(KFS2_INODE_TYPE_MASK|KFS2_INODE_PERMISSION_MASK);
	if ((kfs2_node->node.flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_FILE){
		kfs2_node->kfs2_node.flags|=KFS2_INODE_TYPE_FILE;
	}
	else if ((kfs2_node->node.flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_DIRECTORY){
		kfs2_node->kfs2_node.flags|=KFS2_INODE_TYPE_DIRECTORY;
	}
	else if ((kfs2_node->node.flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK){
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
	kfs2_io_inode_write(kfs2_node);
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
	kfs2_bitmap_init(out,&(extra_data->data_block_allocator),extra_data->root_block.data_block_allocation_bitmap_offsets,extra_data->root_block.data_block_allocation_bitmap_highest_level_length);
	kfs2_bitmap_init(out,&(extra_data->inode_allocator),extra_data->root_block.inode_allocation_bitmap_offsets,extra_data->root_block.inode_allocation_bitmap_highest_level_length);
	SMM_TEMPORARY_STRING root_name=smm_alloc("",0);
	out->root=kfs2_io_inode_read(out,root_name,0);
	out->root->flags|=VFS_NODE_FLAG_PERMANENT;
	mem_copy(out->guid,extra_data->root_block.uuid,16);
	return out;
}



static void _kfs2_fs_mount(filesystem_t* fs,const char* path){
	if (!path||!str_equal(path,"/")){
		return;
	}
	kfs2_fs_extra_data_t* extra_data=fs->extra_data;
	keyring_master_key_get_encrypted(extra_data->root_block.master_key,sizeof(extra_data->root_block.master_key));
	kfs2_insert_crc(&(extra_data->root_block),sizeof(kfs2_root_block_t));
	void* buffer=(void*)(pmm_alloc(1,_kfs2_root_block_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	mem_copy(buffer,&(extra_data->root_block),sizeof(kfs2_root_block_t));
	drive_write(fs->partition->drive,fs->partition->start_lba,buffer,1);
	pmm_dealloc(((u64)buffer)-VMM_HIGHER_HALF_ADDRESS_OFFSET,1,_kfs2_root_block_buffer_pmm_counter);
}



static const filesystem_descriptor_config_t _kfs2_filesystem_descriptor_config={
	"kfs2",
	_kfs2_fs_deinit,
	_kfs2_fs_load,
	_kfs2_fs_mount
};



void kfs2_register_fs(void){
	_kfs2_filesystem_descriptor=fs_register_descriptor(&_kfs2_filesystem_descriptor_config);
	_kfs2_resize_buffer_pmm_counter=pmm_alloc_counter("kfs2_resize_buffer");
	_kfs2_root_block_buffer_pmm_counter=pmm_alloc_counter("kfs2_root_block_buffer");
	_kfs2_vfs_node_allocator=omm_init("kfs2_node",sizeof(kfs2_vfs_node_t),8,4,pmm_alloc_counter("omm_kfs2_node"));
	spinlock_init(&(_kfs2_vfs_node_allocator->lock));
	_kfs2_fs_extra_data_allocator=omm_init("kfs2_extra_data",sizeof(kfs2_fs_extra_data_t),8,1,pmm_alloc_counter("omm_kfs2_extra_data"));
	spinlock_init(&(_kfs2_fs_extra_data_allocator->lock));
}
