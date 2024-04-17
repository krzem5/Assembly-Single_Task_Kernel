#include <kernel/drive/drive.h>
#include <kernel/fs/fs.h>
#include <kernel/keyring/master_key.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/smm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/memory.h>
#include <kernel/util/string.h>
#include <kernel/util/util.h>
#include <kernel/vfs/node.h>
#include <kfs2/api.h>
#include <kfs2/bitmap.h>
#include <kfs2/chunk.h>
#include <kfs2/crc.h>
#include <kfs2/io.h>
#include <kfs2/structures.h>
#define KERNEL_LOG_NAME "kfs2"



#define FNV_OFFSET_BASIS 0x811c9dc5
#define FNV_PRIME 0x01000193



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _kfs2_resize_buffer_pmm_counter=NULL;
static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _kfs2_root_block_buffer_pmm_counter=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _kfs2_vfs_node_allocator=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _kfs2_filesystem_allocator=NULL;
static filesystem_descriptor_t* KERNEL_INIT_WRITE _kfs2_filesystem_descriptor=NULL;



static KERNEL_INLINE u8 _calculate_compressed_hash(const char* name,u32 name_length){
	u32 hash=FNV_OFFSET_BASIS;
	for (u32 i=0;i<name_length;i++){
		hash=(hash^name[i])*FNV_PRIME;
	}
	hash^=hash>>16;
	return hash^(hash>>8);
}



static void _node_resize_data_inline_inline(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 size){
	if (size<node->size){
		return;
	}
	mem_fill(node->data.inline_+node->size,size-node->size,0);
}



static void _node_resize_data_inline_single(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 size){
	kfs2_node_data_t old_data=node->data;
	node->data.single[0]=kfs2_bitmap_alloc(fs,&(fs->data_block_allocator));
	for (u32 i=1;i<6;i++){
		node->data.single[i]=0;
	}
	void* buffer=fs->config.alloc_callback(1);
	mem_copy(buffer,old_data.inline_,node->size);
	kfs2_io_data_block_write(fs,node->data.single[0],buffer);
	fs->config.dealloc_callback(buffer,1);
}



static void _node_resize(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 size){
	if (node->size==size){
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
	u32 old_storage_type=node->flags&KFS2_INODE_STORAGE_MASK;
	if (old_storage_type==KFS2_INODE_STORAGE_TYPE_INLINE&&new_storage_type==KFS2_INODE_STORAGE_TYPE_INLINE){
		_node_resize_data_inline_inline(fs,node,size);
	}
	else if (old_storage_type==KFS2_INODE_STORAGE_TYPE_INLINE&&new_storage_type==KFS2_INODE_STORAGE_TYPE_SINGLE){
		_node_resize_data_inline_single(fs,node,size);
	}
	else{
		panic("_node_resize");
	}
	node->size=size;
	node->flags=(node->flags&(~KFS2_INODE_STORAGE_MASK))|new_storage_type;
	kfs2_io_inode_write(fs,node);
}



static void _attach_child(kfs2_filesystem_t* fs,kfs2_node_t* parent,const char* name,u32 name_length,kfs2_node_t* child){
	u16 new_entry_size=(sizeof(kfs2_directory_entry_t)+name_length+3)&0xfffc;
	kfs2_data_chunk_t chunk;
	kfs2_chunk_init(&chunk);
	for (u64 offset=0;offset<parent->size;){
		if (offset-chunk.offset>=chunk.length){
			kfs2_chunk_read(fs,parent,offset,1,&chunk);
		}
		kfs2_directory_entry_t* entry=(kfs2_directory_entry_t*)(chunk.data+offset-chunk.offset);
		if (entry->name_length||entry->size<new_entry_size){
			offset+=entry->size;
			continue;
		}
		entry->inode=child->_inode;
		entry->name_length=name_length;
		entry->name_compressed_hash=_calculate_compressed_hash(name,name_length);
		mem_copy(entry->name,name,name_length);
		if (entry->size-new_entry_size>=12){
			kfs2_directory_entry_t* next_entry=(kfs2_directory_entry_t*)(chunk.data+offset-chunk.offset+new_entry_size);
			next_entry->inode=0;
			next_entry->size=entry->size-new_entry_size;
			next_entry->name_length=0;
			entry->size=new_entry_size;
		}
		kfs2_chunk_write(fs,parent,&chunk);
		kfs2_chunk_deinit(fs,&chunk);
		return;
	}
	kfs2_chunk_deinit(fs,&chunk);
	_node_resize(fs,parent,(parent->size==48?KFS2_BLOCK_SIZE:parent->size+KFS2_BLOCK_SIZE));
	kfs2_chunk_read(fs,parent,parent->size-KFS2_BLOCK_SIZE,1,&chunk);
	if (parent->size>KFS2_BLOCK_SIZE){
		panic("_attach_child: update large parent");
		kfs2_chunk_deinit(fs,&chunk);
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
	entry->inode=child->_inode;
	entry->name_length=name_length;
	entry->name_compressed_hash=_calculate_compressed_hash(name,name_length);
	mem_copy(entry->name,name,name_length);
	kfs2_directory_entry_t* next_entry=(kfs2_directory_entry_t*)(chunk.data+offset+new_entry_size);
	next_entry->inode=0;
	next_entry->size=entry->size-new_entry_size;
	next_entry->name_length=0;
	entry->size=new_entry_size;
	kfs2_chunk_write(fs,parent,&chunk);
	kfs2_chunk_deinit(fs,&chunk);
}



_Bool kfs2_node_create(kfs2_filesystem_t* fs,kfs2_node_t* parent,const char* name,u32 name_length,u32 flags,kfs2_node_t* out){
	if ((parent->flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_DIRECTORY||name_length>255){
		return 0;
	}
	u32 inode=kfs2_bitmap_alloc(fs,&(fs->inode_allocator));
	if (!inode){
		return 0;
	}
	if ((flags&KFS2_INODE_TYPE_MASK)==KFS2_INODE_TYPE_DIRECTORY){
		out->size=48;
		out->flags=KFS2_INODE_TYPE_DIRECTORY|KFS2_INODE_STORAGE_TYPE_INLINE;
		kfs2_directory_entry_t* entry=(kfs2_directory_entry_t*)(out->data.inline_);
		entry->inode=0;
		entry->size=48;
		entry->name_length=0;
	}
	else{
		out->size=0;
		out->flags=(flags&KFS2_INODE_TYPE_MASK)|KFS2_INODE_STORAGE_TYPE_INLINE;
	}
	out->time_access=0;
	out->time_modify=0;
	out->time_change=0;
	out->time_birth=0;
	out->gid=0;
	out->uid=0;
	out->_inode=inode;
	_attach_child(fs,parent,name,name_length,out);
	return 1;
}



_Bool kfs2_node_lookup(kfs2_filesystem_t* fs,kfs2_node_t* parent,const char* name,u32 name_length,kfs2_node_t* out){
	if (parent->_inode==0xffffffff||!parent->size||(parent->flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_DIRECTORY){
		return 0;
	}
	kfs2_data_chunk_t chunk;
	kfs2_chunk_init(&chunk);
	u64 offset=0;
	u8 compressed_hash=_calculate_compressed_hash(name,name_length);
	while (offset<parent->size){
		if (offset-chunk.offset>=chunk.length){
			kfs2_chunk_read(fs,parent,offset,1,&chunk);
		}
		kfs2_directory_entry_t* entry=(kfs2_directory_entry_t*)(chunk.data+offset-chunk.offset);
		if (entry->name_length!=name_length||entry->name_compressed_hash!=compressed_hash){
			goto _skip_entry;
		}
		for (u16 i=0;i<name_length;i++){
			if (entry->name[i]!=name[i]){
				goto _skip_entry;
			}
		}
		u32 inode=entry->inode;
		kfs2_chunk_deinit(fs,&chunk);
		kfs2_io_inode_read(fs,inode,out);
		return 1;
	_skip_entry:
		offset+=entry->size;
	}
	kfs2_chunk_deinit(fs,&chunk);
	return 0;
}



u64 kfs2_node_iterate(kfs2_filesystem_t* fs,kfs2_node_t* parent,u64 pointer,char* buffer,u32* buffer_length);



_Bool kfs2_node_link(kfs2_filesystem_t* fs,kfs2_node_t* parent,kfs2_node_t* child);



_Bool kfs2_node_unlink(kfs2_filesystem_t* fs,kfs2_node_t* parent,kfs2_node_t* child);



u64 kfs2_node_read(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 offset,void* buffer,u64 size,u32 flags);



u64 kfs2_node_write(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 offset,const void* buffer,u64 size,u32 flags);



u64 kfs2_node_resize(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 size){
	_node_resize(fs,node,size);
	return node->size;
}



static vfs_node_t* _create_node_from_kfs_node(filesystem_t* fs,const string_t* name,kfs2_node_t* kfs2_node){
	kfs2_vfs_node_t* out=(kfs2_vfs_node_t*)vfs_node_create(fs,NULL,name,0);
	if ((kfs2_node->flags&KFS2_INODE_TYPE_MASK)==KFS2_INODE_TYPE_DIRECTORY){
		out->node.flags|=VFS_NODE_TYPE_DIRECTORY;
	}
	else if ((kfs2_node->flags&KFS2_INODE_TYPE_MASK)==KFS2_INODE_TYPE_LINK){
		out->node.flags|=VFS_NODE_TYPE_LINK;
	}
	else{
		out->node.flags|=VFS_NODE_TYPE_FILE;
	}
	out->node.flags|=((kfs2_node->flags&KFS2_INODE_PERMISSION_MASK)>>KFS2_INODE_PERMISSION_SHIFT)<<VFS_NODE_PERMISSION_SHIFT;
	out->node.time_access=kfs2_node->time_access;
	out->node.time_modify=kfs2_node->time_modify;
	out->node.time_change=kfs2_node->time_change;
	out->node.time_birth=kfs2_node->time_birth;
	out->node.gid=kfs2_node->gid;
	out->node.uid=kfs2_node->uid;
	out->kfs2_node=*kfs2_node;
	return (vfs_node_t*)out;
}



static vfs_node_t* _kfs2_create(vfs_node_t* parent,const string_t* name,u32 flags){
	if (!(flags&VFS_NODE_FLAG_CREATE)){
		kfs2_vfs_node_t* out=omm_alloc(_kfs2_vfs_node_allocator);
		out->kfs2_node._inode=0xffffffff;
		return (vfs_node_t*)out;
	}
	u32 kfs2_flags=0;
	if ((flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_DIRECTORY){
		kfs2_flags|=KFS2_INODE_TYPE_DIRECTORY;
	}
	else if ((flags&VFS_NODE_TYPE_MASK)==VFS_NODE_TYPE_LINK){
		kfs2_flags|=KFS2_INODE_TYPE_LINK;
	}
	else{
		kfs2_flags|=KFS2_INODE_TYPE_FILE;
	}
	kfs2_node_t ret;
	if (!kfs2_node_create(parent->fs->extra_data,&(((kfs2_vfs_node_t*)parent)->kfs2_node),name->data,name->length,kfs2_flags,&ret)){
		return NULL;
	}
	return _create_node_from_kfs_node(parent->fs,name,&ret);
}



static void _kfs2_delete(vfs_node_t* node){
	omm_dealloc(_kfs2_vfs_node_allocator,node);
}



static vfs_node_t* _kfs2_lookup(vfs_node_t* node,const string_t* name){
	kfs2_node_t ret;
	if (!kfs2_node_lookup(node->fs->extra_data,&(((kfs2_vfs_node_t*)node)->kfs2_node),name->data,name->length,&ret)){
		return NULL;
	}
	return _create_node_from_kfs_node(node->fs,name,&ret);
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
			kfs2_chunk_read(kfs2_node->node.fs->extra_data,&(kfs2_node->kfs2_node),pointer,1,&chunk);
		}
		kfs2_directory_entry_t* entry=(kfs2_directory_entry_t*)(chunk.data+pointer-chunk.offset);
		pointer+=entry->size;
		if (entry->name_length){
			*out=smm_alloc(entry->name,entry->name_length);
			kfs2_chunk_deinit(kfs2_node->node.fs->extra_data,&chunk);
			return pointer;
		}
	}
	kfs2_chunk_deinit(kfs2_node->node.fs->extra_data,&chunk);
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
			kfs2_chunk_read(kfs2_node->node.fs->extra_data,&(kfs2_node->kfs2_node),offset,1,&chunk);
		}
		u64 padding=offset-chunk.offset;
		u64 read_size=(chunk.length-padding>size-offset?size-offset:chunk.length-padding);
		mem_copy(buffer,chunk.data+padding,read_size);
		buffer+=read_size;
		offset+=chunk.length-padding;
	}
	kfs2_chunk_deinit(kfs2_node->node.fs->extra_data,&chunk);
	return out;
}



static u64 _kfs2_write(vfs_node_t* node,u64 offset,const void* buffer,u64 size,u32 flags){
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
		kfs2_chunk_read(kfs2_node->node.fs->extra_data,&(kfs2_node->kfs2_node),offset,(offset&(KFS2_BLOCK_SIZE-1))||(size-offset<KFS2_BLOCK_SIZE),&chunk);
		u64 padding=offset-chunk.offset;
		u64 write_size=(chunk.length-padding>size-offset?size-offset:chunk.length-padding);
		mem_copy(chunk.data+padding,buffer,write_size);
		buffer+=write_size;
		offset+=write_size;
		kfs2_chunk_write(kfs2_node->node.fs->extra_data,&(kfs2_node->kfs2_node),&chunk);
	}
	kfs2_chunk_deinit(kfs2_node->node.fs->extra_data,&chunk);
	return out;
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
	return kfs2_node_resize(node->fs->extra_data,&(kfs2_node->kfs2_node),(size<0?0:size));
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
	kfs2_node_flush(kfs2_node->node.fs->extra_data,&(kfs2_node->kfs2_node));
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
	omm_dealloc(_kfs2_filesystem_allocator,fs->extra_data);
	panic("_kfs2_deinit_callback");
}



static void* _alloc_page(u64 count){
	return (void*)(pmm_alloc(count,_kfs2_root_block_buffer_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
}



static void _dealloc_page(void* ptr,u64 count){
	pmm_dealloc(((u64)ptr)-VMM_HIGHER_HALF_ADDRESS_OFFSET,count,_kfs2_root_block_buffer_pmm_counter);
}



static filesystem_t* _kfs2_fs_load(partition_t* partition){
	drive_t* drive=partition->drive;
	kfs2_filesystem_t* extra_data=omm_alloc(_kfs2_filesystem_allocator);
	kfs2_filesystem_config_t config={
		drive,
		(kfs2_filesystem_block_read_callback_t)drive_read,
		(kfs2_filesystem_block_write_callback_t)drive_write,
		_alloc_page,
		_dealloc_page,
		drive->block_size,
		partition->start_lba,
		partition->end_lba,
	};
	if (!kfs2_filesystem_init(&config,extra_data)){
		omm_dealloc(_kfs2_filesystem_allocator,extra_data);
		return NULL;
	}
	filesystem_t* out=fs_create(_kfs2_filesystem_descriptor);
	out->functions=&_kfs2_functions;
	out->partition=partition;
	out->extra_data=extra_data;
	kfs2_node_t root_node;
	kfs2_filesystem_get_root(extra_data,&root_node);
	SMM_TEMPORARY_STRING root_name=smm_alloc("",0);
	out->root=_create_node_from_kfs_node(out,root_name,&root_node);
	out->root->flags|=VFS_NODE_FLAG_PERMANENT;
	mem_copy(out->guid,extra_data->root_block.uuid,16);
	return out;
}



static void _kfs2_fs_mount(filesystem_t* fs,const char* path){
	if (!path||!str_equal(path,"/")){
		return;
	}
	kfs2_filesystem_t* extra_data=fs->extra_data;
	keyring_master_key_get_encrypted(extra_data->root_block.master_key,sizeof(extra_data->root_block.master_key));
	kfs2_filesystem_flush_root_block(extra_data);
}



static _Bool _kfs2_fs_format(partition_t* partition){
	panic("_kfs2_fs_format");
}



static const filesystem_descriptor_config_t _kfs2_filesystem_descriptor_config={
	"kfs2",
	_kfs2_fs_deinit,
	_kfs2_fs_load,
	_kfs2_fs_mount,
	_kfs2_fs_format
};



MODULE_INIT(){
	_kfs2_resize_buffer_pmm_counter=pmm_alloc_counter("kfs2_resize_buffer");
	_kfs2_root_block_buffer_pmm_counter=pmm_alloc_counter("kfs2_root_block_buffer");
	_kfs2_vfs_node_allocator=omm_init("kfs2_node",sizeof(kfs2_vfs_node_t),8,4,pmm_alloc_counter("omm_kfs2_node"));
	spinlock_init(&(_kfs2_vfs_node_allocator->lock));
	_kfs2_filesystem_allocator=omm_init("kfs2_filesystem",sizeof(kfs2_filesystem_t),8,1,pmm_alloc_counter("omm_kfs2_filesystem"));
	spinlock_init(&(_kfs2_filesystem_allocator->lock));
}



MODULE_POSTINIT(){
	_kfs2_filesystem_descriptor=fs_register_descriptor(&_kfs2_filesystem_descriptor_config);
}
