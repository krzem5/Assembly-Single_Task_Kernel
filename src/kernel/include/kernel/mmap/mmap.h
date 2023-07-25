#ifndef _KERNEL_MMAP_MMAP_H_
#define _KERNEL_MMAP_MMAP_H_ 1
#include <kernel/types.h>



void mmap_init(void);



void mmap_set_range(u64 from,u64 to);



u64 mmap_alloc(u64 length);



_Bool mmap_dealloc(u64 address,u64 length);



#endif
