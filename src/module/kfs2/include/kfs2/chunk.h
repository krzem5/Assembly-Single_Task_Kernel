#ifndef _KFS2_CHUNK_H_
#define _KFS2_CHUNK_H_ 1
#include <kernel/types.h>
#include <kfs2/structures.h>



void kfs2_chunk_init(kfs2_data_chunk_t* out);



void kfs2_chunk_deinit(kfs2_data_chunk_t* chunk);



void kfs2_chunk_read(kfs2_vfs_node_t* node,u64 offset,_Bool fetch_data,kfs2_data_chunk_t* out);



void kfs2_chunk_write(kfs2_vfs_node_t* node,kfs2_data_chunk_t* chunk);



#endif
