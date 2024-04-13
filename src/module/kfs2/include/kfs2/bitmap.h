#ifndef _KFS2_BITMAP_H_
#define _KFS2_BITMAP_H_ 1
#include <kernel/fs/fs.h>
#include <kernel/types.h>
#include <kfs2/structures.h>



void kfs2_bitmap_init(filesystem_t* fs,kfs2_bitmap_t* allocator,const u64* bitmap_offsets,u32 highest_level_length);



u64 kfs2_bitmap_alloc(filesystem_t* fs,kfs2_bitmap_t* allocator);



#endif
