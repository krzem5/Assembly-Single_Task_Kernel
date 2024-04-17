#ifndef _COMMON_KFS2_CHUNK_H_
#define _COMMON_KFS2_CHUNK_H_ 1
#include <common/kfs2/api.h>
#include <common/kfs2/structures.h>
#include <common/kfs2/util.h>



typedef struct _KFS2_DATA_CHUNK{
	u64 offset;
	u64* quadruple_cache;
	u64* triple_cache;
	u64* double_cache;
	void* data;
	u16 length;
	u64 data_offset;
} kfs2_data_chunk_t;



void kfs2_chunk_init(kfs2_data_chunk_t* out);



void kfs2_chunk_deinit(kfs2_filesystem_t* fs,kfs2_data_chunk_t* chunk);



void kfs2_chunk_read(kfs2_filesystem_t* fs,kfs2_node_t* node,u64 offset,_Bool fetch_data,kfs2_data_chunk_t* out);



void kfs2_chunk_write(kfs2_filesystem_t* fs,kfs2_node_t* node,kfs2_data_chunk_t* chunk);



#endif
