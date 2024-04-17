#include <common/kfs2/chunk.h>
#include <common/kfs2/io.h>
#include <common/kfs2/structures.h>
#include <common/kfs2/util.h>



void kfs2_chunk_init(kfs2_data_chunk_t* out){
	out->offset=0;
	out->quadruple_cache=NULL;
	out->triple_cache=NULL;
	out->double_cache=NULL;
	out->data=NULL;
	out->length=0;
	out->data_offset=0;
}



void kfs2_chunk_deinit(kfs2_filesystem_t* fs,kfs2_data_chunk_t* chunk){
	chunk->offset=0;
	if (chunk->quadruple_cache){
		fs->config.dealloc_callback(chunk->quadruple_cache,1);
		chunk->quadruple_cache=NULL;
	}
	if (chunk->triple_cache){
		fs->config.dealloc_callback(chunk->triple_cache,1);
		chunk->triple_cache=NULL;
	}
	if (chunk->double_cache){
		fs->config.dealloc_callback(chunk->double_cache,1);
		chunk->double_cache=NULL;
	}
	if (chunk->data&&chunk->length==KFS2_BLOCK_SIZE){
		fs->config.dealloc_callback(chunk->data,1);
	}
	chunk->data=NULL;
	chunk->length=0;
}



void kfs2_chunk_read(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 offset,_Bool fetch_data,kfs2_data_chunk_t* out){
	switch (node->flags&KFS2_INODE_STORAGE_MASK){
		case KFS2_INODE_STORAGE_TYPE_INLINE:
			if (offset>=48){
				panic("kfs2_chunk_read: invalid offset");
			}
			out->offset=0;
			out->data=node->data.inline_;
			out->length=48;
			out->data_offset=0;
			return;
		case KFS2_INODE_STORAGE_TYPE_SINGLE:
			{
				u64 index=offset/KFS2_BLOCK_SIZE;
				if (index>=6){
					panic("kfs2_chunk_read: invalid offset");
				}
				if (!out->data){
					out->data=fs->config.alloc_callback(1);
				}
				out->offset=offset&(-KFS2_BLOCK_SIZE);
				out->data_offset=node->data.single[index];
				out->length=KFS2_BLOCK_SIZE;
				break;
			}
		case KFS2_INODE_STORAGE_TYPE_DOUBLE:
			{
				u64 index=offset/KFS2_BLOCK_SIZE;
				if (index>=KFS2_BLOCK_SIZE/sizeof(u64)){
					panic("kfs2_chunk_read: invalid offset");
				}
				if (!out->double_cache){
					out->double_cache=fs->config.alloc_callback(1);
					kfs2_io_data_block_read(fs,node->data.double_,out->double_cache);
				}
				if (!out->data){
					out->data=fs->config.alloc_callback(1);
				}
				out->offset=offset&(-KFS2_BLOCK_SIZE);
				out->data_offset=out->double_cache[index];
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
	if (!out->data_offset){
		panic("I/O out of bounds");
	}
	if (fetch_data){
		kfs2_io_data_block_read(fs,out->data_offset,out->data);
	}
}



void kfs2_chunk_write(kfs2_filesystem_t* fs,kfs2_node_t* node,kfs2_data_chunk_t* chunk){
	if ((node->flags&KFS2_INODE_STORAGE_MASK)==KFS2_INODE_STORAGE_TYPE_INLINE){
		kfs2_io_inode_write(fs,node);
	}
	else{
		kfs2_io_data_block_write(fs,chunk->data_offset,chunk->data);
	}
}
