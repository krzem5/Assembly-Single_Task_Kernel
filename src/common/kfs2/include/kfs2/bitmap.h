#ifndef _KFS2_BITMAP_H_
#define _KFS2_BITMAP_H_ 1
#include <kfs2/structures.h>
#include <kfs2/util.h>



struct _KFS2_FILESYSTEM;



typedef struct _KFS2_BITMAP_CACHE_ENTRY{
	u64 offset;
	u64 block_index;
	u64* data;
} kfs2_bitmap_cache_entry_t;



typedef struct _KFS2_BITMAP{
	u64 bitmap_offset;
	u32 highest_level_length;
	u32 highest_level_offset;
	kfs2_bitmap_cache_entry_t cache[KFS2_BITMAP_LEVEL_COUNT];
} kfs2_bitmap_t;



void kfs2_bitmap_init(struct _KFS2_FILESYSTEM* fs,kfs2_bitmap_t* allocator,const u64* bitmap_offsets,u32 highest_level_length);



u64 kfs2_bitmap_alloc(struct _KFS2_FILESYSTEM* fs,kfs2_bitmap_t* allocator);



#endif
