#ifndef _KERNEL_MEMORY_PMM_H_
#define _KERNEL_MEMORY_PMM_H_ 1
#include <kernel/types.h>



#define PAGE_SIZE 4096
#define PAGE_SIZE_SHIFT 12
#define LARGE_PAGE_SIZE 2097152
#define LARGE_PAGE_SIZE_SHIFT 21
#define EXTRA_LARGE_PAGE_SIZE 1073741824
#define EXTRA_LARGE_PAGE_SIZE_SHIFT 30

#define PMM_ALLOCATOR_SIZE_COUNT 16

#define PMM_COUNTER_TOTAL 0
#define PMM_COUNTER_DRIVER_AHCI 1
#define PMM_COUNTER_DRIVER_I82540 2
#define PMM_COUNTER_KERNEL_STACK 3
#define PMM_COUNTER_KFS 4
#define PMM_COUNTER_KMM 5
#define PMM_COUNTER_NETWORK 6
#define PMM_COUNTER_UMM 7
#define PMM_COUNTER_USER 8
#define PMM_COUNTER_USER_STACK 9
#define PMM_COUNTER_VMM 10
#define PMM_COUNTER_MAX PMM_COUNTER_VMM



typedef struct _PMM_ALLOCATOR_PAGE_HEADER{
	u64 prev;
	u64 next;
	u8 idx;
} pmm_allocator_page_header_t;



typedef struct _PMM_COUNTERS{
	u64 data[PMM_COUNTER_MAX+1];
} pmm_counters_t;



typedef struct _PMM_ALLOCATOR{
	u64 last_memory_address;
	u64* bitmap;
	u64 blocks[PMM_ALLOCATOR_SIZE_COUNT];
	u16 block_bitmap;
	pmm_counters_t counters;
} pmm_allocator_t;



static inline u64 pmm_align_up_address(u64 base){
	return (base+PAGE_SIZE-1)&(-PAGE_SIZE);
}



static inline u64 pmm_align_up_address_large(u64 base){
	return (base+LARGE_PAGE_SIZE-1)&(-LARGE_PAGE_SIZE);
}



static inline u64 pmm_align_up_address_extra_large(u64 base){
	return (base+EXTRA_LARGE_PAGE_SIZE-1)&(-EXTRA_LARGE_PAGE_SIZE);
}



static inline u64 pmm_align_down_address(u64 base){
	return base&(-PAGE_SIZE);
}



static inline u64 pmm_align_down_address_large(u64 base){
	return base&(-LARGE_PAGE_SIZE);
}



static inline u64 pmm_align_down_address_extra_large(u64 base){
	return base&(-EXTRA_LARGE_PAGE_SIZE);
}



void pmm_init(void);



void pmm_init_high_mem(void);



u64 pmm_alloc(u64 count,u8 counter);



u64 pmm_alloc_zero(u64 count,u8 counter);



void pmm_dealloc(u64 address,u64 count,u8 counter);



const pmm_counters_t* pmm_get_counters(void);



#endif
