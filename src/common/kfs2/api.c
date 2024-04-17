#include <kfs2/api.h>
#include <kfs2/bitmap.h>
#include <kfs2/chunk.h>
#include <kfs2/crc.h>
#include <kfs2/io.h>
#include <kfs2/structures.h>
#include <kfs2/util.h>



#define FNV_OFFSET_BASIS 0x811c9dc5
#define FNV_PRIME 0x01000193



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



_Bool kfs2_filesystem_init(const kfs2_filesystem_config_t* config,kfs2_filesystem_t* out){
	if (config->block_size>4096){
		return 0;
	}
	out->config=*config;
	out->block_size_shift=63-__builtin_clzll(KFS2_BLOCK_SIZE/out->config.block_size);
	void* buffer=out->config.alloc_callback(1);
	kfs2_root_block_t* root_block=(kfs2_root_block_t*)buffer;
	if (out->config.read_callback(out->config.ctx,out->config.start_lba,buffer,1)!=1||root_block->signature!=KFS2_ROOT_BLOCK_SIGNATURE||!kfs2_verify_crc(root_block,sizeof(kfs2_root_block_t))){
		out->config.dealloc_callback(buffer,1);
		return 0;
	}
	out->root_block=*root_block;
	out->config.dealloc_callback(buffer,1);
	kfs2_bitmap_init(out,&(out->data_block_allocator),out->root_block.data_block_allocation_bitmap_offsets,out->root_block.data_block_allocation_bitmap_highest_level_length);
	kfs2_bitmap_init(out,&(out->inode_allocator),out->root_block.inode_allocation_bitmap_offsets,out->root_block.inode_allocation_bitmap_highest_level_length);
	return 1;
}



void kfs2_filesystem_deinit(kfs2_filesystem_t* fs);



_Bool kfs2_filesystem_get_root(kfs2_filesystem_t* fs,kfs2_node_t* out){
	kfs2_io_inode_read(fs,0,out);
	return 1;
}



void kfs2_filesystem_flush_root_block(kfs2_filesystem_t* fs){
	kfs2_insert_crc(&(fs->root_block),sizeof(kfs2_root_block_t));
	kfs2_root_block_t* buffer=fs->config.alloc_callback(1);
	*buffer=fs->root_block;
	fs->config.write_callback(fs->config.ctx,fs->config.start_lba,buffer,1);
	fs->config.dealloc_callback(buffer,1);
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



u64 kfs2_node_iterate(kfs2_filesystem_t* fs,kfs2_node_t* parent,u64 pointer,char* buffer,u32* buffer_length){
	if (parent->_inode==0xffffffff||pointer>=parent->size||(parent->flags&KFS2_INODE_TYPE_MASK)!=KFS2_INODE_TYPE_DIRECTORY){
		return 0;
	}
	kfs2_data_chunk_t chunk;
	kfs2_chunk_init(&chunk);
	while (pointer<parent->size){
		if (pointer-chunk.offset>=chunk.length){
			kfs2_chunk_read(fs,parent,pointer,1,&chunk);
		}
		kfs2_directory_entry_t* entry=(kfs2_directory_entry_t*)(chunk.data+pointer-chunk.offset);
		pointer+=entry->size;
		if (!entry->name_length){
			continue;
		}
		if (entry->name_length<*buffer_length){
			*buffer_length=entry->name_length;
		}
		mem_copy(buffer,entry->name,*buffer_length);
		kfs2_chunk_deinit(fs,&chunk);
		return pointer;
	}
	kfs2_chunk_deinit(fs,&chunk);
	return 0;
}



_Bool kfs2_node_link(kfs2_filesystem_t* fs,kfs2_node_t* parent,kfs2_node_t* child);



_Bool kfs2_node_unlink(kfs2_filesystem_t* fs,kfs2_node_t* parent,kfs2_node_t* child);



u64 kfs2_node_read(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 offset,void* buffer,u64 size){
	if (node->_inode==0xffffffff||offset>=node->size){
		return 0;
	}
	if (offset+size>=node->size){
		size=node->size-offset;
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
			kfs2_chunk_read(fs,node,offset,1,&chunk);
		}
		u64 padding=offset-chunk.offset;
		u64 read_size=(chunk.length-padding>size-offset?size-offset:chunk.length-padding);
		mem_copy(buffer,chunk.data+padding,read_size);
		buffer+=read_size;
		offset+=chunk.length-padding;
	}
	kfs2_chunk_deinit(fs,&chunk);
	return out;
}



u64 kfs2_node_write(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 offset,const void* buffer,u64 size){
	if (node->_inode==0xffffffff||offset>=node->size){
		return 0;
	}
	if (offset+size>=node->size){
		size=node->size-offset;
	}
	if (!size){
		return 0;
	}
	u64 out=size;
	kfs2_data_chunk_t chunk;
	kfs2_chunk_init(&chunk);
	size+=offset;
	while (offset<size){
		kfs2_chunk_read(fs,node,offset,(offset&(KFS2_BLOCK_SIZE-1))||(size-offset<KFS2_BLOCK_SIZE),&chunk);
		u64 padding=offset-chunk.offset;
		u64 write_size=(chunk.length-padding>size-offset?size-offset:chunk.length-padding);
		mem_copy(chunk.data+padding,buffer,write_size);
		buffer+=write_size;
		offset+=write_size;
		kfs2_chunk_write(fs,node,&chunk);
	}
	kfs2_chunk_deinit(fs,&chunk);
	return out;
}



u64 kfs2_node_resize(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 size){
	_node_resize(fs,node,size);
	return node->size;
}



_Bool kfs2_node_flush(kfs2_filesystem_t* fs,kfs2_node_t* node){
	kfs2_io_inode_write(fs,node);
	return 1;
}
