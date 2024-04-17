#ifndef _KFS2_BITMAP_H_
#define _KFS2_BITMAP_H_ 1
#include <kernel/types.h>
#include <kfs2/api.h>
#include <kfs2/structures.h>



void kfs2_bitmap_init(kfs2_filesystem_t* fs,kfs2_bitmap_t* allocator,const u64* bitmap_offsets,u32 highest_level_length);



u64 kfs2_bitmap_alloc(kfs2_filesystem_t* fs,kfs2_bitmap_t* allocator);



#endif
