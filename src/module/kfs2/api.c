#include <kfs2/api.h>
#include <kfs2/bitmap.h>
#include <kfs2/chunk.h>
#include <kfs2/crc.h>
#include <kfs2/io.h>
#include <kfs2/structures.h>
#define KERNEL_LOG_NAME "kfs2"



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



_Bool kfs2_filesystem_get_root(kfs2_filesystem_t* fs,kfs2_node_t* out);



_Bool kfs2_node_create(kfs2_filesystem_t* fs,kfs2_node_t* parent,const char* name,u32 name_length,u32 flags,kfs2_node_t* out);



_Bool kfs2_node_delete(kfs2_filesystem_t* fs,kfs2_node_t* node);



_Bool kfs2_node_lookup(kfs2_filesystem_t* fs,kfs2_node_t* parent,const char* name,u32 name_length,kfs2_node_t* out);



u64 kfs2_node_iterate(kfs2_filesystem_t* fs,kfs2_node_t* parent,u64 pointer,char* buffer,u32* buffer_length);



_Bool kfs2_node_link(kfs2_filesystem_t* fs,kfs2_node_t* parent,kfs2_node_t* child);



_Bool kfs2_node_unlink(kfs2_filesystem_t* fs,kfs2_node_t* parent,kfs2_node_t* child);



u64 kfs2_node_read(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 offset,void* buffer,u64 size,u32 flags);



u64 kfs2_node_write(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 offset,const void* buffer,u64 size,u32 flags);



u64 kfs2_node_resize(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 size,_Bool relative);



_Bool kfs2_node_flush(kfs2_filesystem_t* fs,kfs2_node_t* node);
