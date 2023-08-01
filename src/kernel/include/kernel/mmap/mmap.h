#ifndef _KERNEL_MMAP_MMAP_H_
#define _KERNEL_MMAP_MMAP_H_ 1
#include <kernel/types.h>



#define MMAP_FLAG_LARGE 1
#define MMAP_FLAG_EXTRA_LARGE 2



void mmap_init(void);



void mmap_set_range(u64 from,u64 to);



u64 mmap_alloc(u64 length,u8 flags);



_Bool mmap_dealloc(u64 address,u64 length);



#endif
