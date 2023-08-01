#ifndef _KERNEL_MEMORY_PMM_H_
#define _KERNEL_MEMORY_PMM_H_ 1
#include <kernel/types.h>



#define PAGE_SIZE 4096
#define PAGE_SIZE_SHIFT 12

#define PMM_ALLOCATOR_SIZE_COUNT 16



typedef struct _PMM_ALLOCATOR_PAGE_HEADER{
	u64 next;
} pmm_allocator_page_header_t;



typedef struct _PMM_ALLOCATOR{
	u64 bitmap;
	u64 blocks[PMM_ALLOCATOR_SIZE_COUNT];
} pmm_allocator_t;



static inline u64 pmm_align_up_address(u64 base){
	return (base+PAGE_SIZE-1)&(~(PAGE_SIZE-1));
}



static inline u64 pmm_align_down_address(u64 base){
	return base&(~(PAGE_SIZE-1));
}



void pmm_init(const kernel_data_t* kernel_data);



void pmm_init_high_mem(const kernel_data_t* kernel_data);



u64 pmm_alloc(u64 count);



u64 pmm_alloc_zero(u64 count);



void pmm_dealloc(u64 address,u64 count);



#endif
