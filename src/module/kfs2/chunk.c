#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/module/module.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#include <kfs2/io.h>
#include <kfs2/structures.h>



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _kfs2_chunk_pmm_counter=NULL;



MODULE_INIT(){
	_kfs2_chunk_pmm_counter=pmm_alloc_counter("kfs2_chunk");
}



void kfs2_chunk_init(kfs2_data_chunk_t* out){
	out->offset=0;
	out->quadruple_cache=NULL;
	out->triple_cache=NULL;
	out->double_cache=NULL;
	out->data=NULL;
	out->length=0;
	out->data_offset=0;
}



void kfs2_chunk_deinit(kfs2_data_chunk_t* chunk){
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



void kfs2_chunk_read(kfs2_vfs_node_t* node,u64 offset,_Bool fetch_data,kfs2_data_chunk_t* out){
	switch (node->kfs2_node.flags&KFS2_INODE_STORAGE_MASK){
		case KFS2_INODE_STORAGE_TYPE_INLINE:
			if (offset>=48){
				panic("kfs2_chunk_read: invalid offset");
			}
			out->offset=0;
			out->data=node->kfs2_node.data.inline_;
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
					out->data=(void*)(pmm_alloc(1,_kfs2_chunk_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
				}
				out->offset=offset&(-KFS2_BLOCK_SIZE);
				out->data_offset=node->kfs2_node.data.single[index];
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
					out->double_cache=(void*)(pmm_alloc(1,_kfs2_chunk_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
					kfs2_io_data_block_read(node->node.fs,node->kfs2_node.data.double_,out->double_cache);
				}
				if (!out->data){
					out->data=(void*)(pmm_alloc(1,_kfs2_chunk_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
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
	if (fetch_data){
		kfs2_io_data_block_read(node->node.fs,out->data_offset,out->data);
	}
}



void kfs2_chunk_write(kfs2_vfs_node_t* node,kfs2_data_chunk_t* chunk){
	if ((node->kfs2_node.flags&KFS2_INODE_STORAGE_MASK)==KFS2_INODE_STORAGE_TYPE_INLINE){
		kfs2_io_inode_write(node);
	}
	else{
		kfs2_io_data_block_write(node->node.fs,chunk->data_offset,chunk->data);
	}
}
