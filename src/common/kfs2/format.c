#include <common/kfs2/api.h>
#include <common/kfs2/crc.h>
#include <common/kfs2/io.h>
#include <common/kfs2/structures.h>
#include <common/kfs2/util.h>

#include <stdio.h>

typedef struct _BITMAP_DATA{
	u64 lengths[KFS2_BITMAP_LEVEL_COUNT];
	u64 offsets[KFS2_BITMAP_LEVEL_COUNT];
} bitmap_data_t;



static void _compute_bitmap_lengths(u64 count,bitmap_data_t* out){
	for (u32 i=0;i<KFS2_BITMAP_LEVEL_COUNT;i++){
		count=(count+63)>>6;
		out->lengths[i]=count;
	}
}



static u64 _compute_bitmap_offsets(bitmap_data_t* bitmap,u64 offset){
	for (u32 i=0;i<KFS2_BITMAP_LEVEL_COUNT;i++){
		offset-=(bitmap->lengths[i]*sizeof(u64)+KFS2_BLOCK_SIZE-1)/KFS2_BLOCK_SIZE;
		bitmap->offsets[i]=offset;
	}
	return offset;
}



static void _init_bitmap(const kfs2_filesystem_config_t* config,const bitmap_data_t* bitmap,u64 count){
	u64* buffer=config->alloc_callback(1);
	for (u32 i=0;i<KFS2_BITMAP_LEVEL_COUNT;i++){
		for (u64 j=0;j<((bitmap->lengths[i]+KFS2_BLOCK_SIZE/sizeof(u64)-1)&(-KFS2_BLOCK_SIZE/sizeof(u64)));j++){
			u64 mask=0;
			if ((j<<6)+63<=count){
				mask=0xffffffffffffffffull;
			}
			else if ((j<<6)<=count){
				mask=(1ull<<(count+64-(j<<6)))-1;
			}
			buffer[j&(KFS2_BLOCK_SIZE/sizeof(u64)-1)]=mask;
			if ((j&(KFS2_BLOCK_SIZE/sizeof(u64)-1))==KFS2_BLOCK_SIZE/sizeof(u64)-1){
				config->write_callback(config->ctx,config->start_lba+((bitmap->offsets[i]+j*sizeof(u64)/KFS2_BLOCK_SIZE)*KFS2_BLOCK_SIZE)/config->block_size,buffer,KFS2_BLOCK_SIZE/config->block_size);
			}
		}
		count=(count+63)>>6;
	}
	config->dealloc_callback(buffer,1);
}



_Bool kfs2_filesystem_format(const kfs2_filesystem_config_t* config,kfs2_filesystem_t* out){
	if (config->block_size>4096){
		return 0;
	}
	u64 block_count=(config->end_lba-config->start_lba)*config->block_size/KFS2_BLOCK_SIZE;
	u64 inode_block_count=block_count/KFS2_BLOCKS_PER_INODE_BLOCK;
	u64 inode_count=KFS2_BLOCK_SIZE/sizeof(kfs2_node_t)*inode_block_count;
	if (inode_count>KFS2_MAX_INODES){
		inode_count=KFS2_MAX_INODES;
	}
	u64 first_inode_block=1;
	u64 first_data_block=first_inode_block+inode_block_count;
	bitmap_data_t inode_bitmap;
	bitmap_data_t data_block_bitmap;
	_compute_bitmap_lengths(inode_count,&inode_bitmap);
	_compute_bitmap_lengths(block_count-first_data_block,&data_block_bitmap);
	u64 first_bitmap_block=_compute_bitmap_offsets(&data_block_bitmap,_compute_bitmap_offsets(&inode_bitmap,block_count));
	u64 data_block_count=first_bitmap_block-first_data_block;
	kfs2_root_block_t* root_block=config->alloc_callback(1);
	root_block->signature=KFS2_ROOT_BLOCK_SIGNATURE;
	root_block->block_count=block_count;
	root_block->inode_count=inode_count;
	root_block->data_block_count=data_block_count;
	root_block->first_inode_block=first_inode_block;
	root_block->first_data_block=first_data_block;
	mem_copy(root_block->inode_allocation_bitmap_offsets,inode_bitmap.offsets,KFS2_BITMAP_LEVEL_COUNT*sizeof(u64));
	mem_copy(root_block->data_block_allocation_bitmap_offsets,data_block_bitmap.offsets,KFS2_BITMAP_LEVEL_COUNT*sizeof(u64));
	root_block->inode_allocation_bitmap_highest_level_length=inode_bitmap.lengths[KFS2_BITMAP_LEVEL_COUNT-1];
	root_block->data_block_allocation_bitmap_highest_level_length=data_block_bitmap.lengths[KFS2_BITMAP_LEVEL_COUNT-1];
	kfs2_insert_crc(root_block,sizeof(kfs2_root_block_t));
	_Bool ret=config->write_callback(config->ctx,config->start_lba,root_block,1);
	config->dealloc_callback(root_block,1);
	if (ret!=1){
		return 0;
	}
	u64 drive_blocks_per_block=KFS2_BLOCK_SIZE/config->block_size;
	void* zero_page=config->alloc_callback(1);
	for (u64 i=0;i<inode_block_count;i++){
		if (config->write_callback(config->ctx,config->start_lba+(first_inode_block+i)*drive_blocks_per_block,zero_page,drive_blocks_per_block)!=drive_blocks_per_block){
			ret=0;
			break;
		}
	}
	config->dealloc_callback(zero_page,1);
	if (!ret){
		return 0;
	}
	_init_bitmap(config,&inode_bitmap,inode_count);
	_init_bitmap(config,&data_block_bitmap,data_block_count);
	if (!kfs2_filesystem_init(config,out)){
		return 0;
	}
	u64 root_inode=kfs2_bitmap_alloc(out,&(out->inode_allocator));
	if (root_inode){
		return 0;
	}
	kfs2_node_t root_node;
	mem_fill(&root_node,sizeof(kfs2_node_t),0);
	root_node.size=48;
	root_node.flags=KFS2_INODE_TYPE_DIRECTORY|KFS2_INODE_STORAGE_TYPE_INLINE|(0755<<KFS2_INODE_PERMISSION_SHIFT);
	kfs2_directory_entry_t* entry=(void*)(root_node.data.inline_);
	entry->size=48;
	kfs2_io_inode_write(out,&root_node);//
	return 1;
}
