#ifndef _KFS2_BITMAP_H_
#define _KFS2_BITMAP_H_ 1
#include <kernel/fs/fs.h>
#include <kernel/types.h>
#include <kfs2/api.h>
#include <kfs2/structures.h>



void kfs2_bitmap_init(filesystem_t* fs,kfs2_bitmap_t* allocator,const u64* bitmap_offsets,u32 highest_level_length);



void kfs2_bitmap_init_NEW(kfs2_filesystem_t* fs,kfs2_bitmap_t* allocator,const u64* bitmap_offsets,u32 highest_level_length);



u64 kfs2_bitmap_alloc(filesystem_t* fs,kfs2_bitmap_t* allocator);



u64 kfs2_bitmap_alloc_NEW(kfs2_filesystem_t* fs,kfs2_bitmap_t* allocator);



#endif
