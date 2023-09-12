#ifndef _KERNEL_MEMORY_PMM_H_
#define _KERNEL_MEMORY_PMM_H_ 1
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define PAGE_SIZE 4096
#define PAGE_SIZE_SHIFT 12
#define LARGE_PAGE_SIZE 2097152
#define LARGE_PAGE_SIZE_SHIFT 21
#define EXTRA_LARGE_PAGE_SIZE 1073741824
#define EXTRA_LARGE_PAGE_SIZE_SHIFT 30

#define PMM_ALLOCATOR_SIZE_COUNT 16

#define PMM_COUNTER_TOTAL 0
#define PMM_COUNTER_FREE 1
#define PMM_COUNTER_DRIVER_AHCI 2
#define PMM_COUNTER_DRIVER_I82540 3
#define PMM_COUNTER_KERNEL_STACK 4
#define PMM_COUNTER_KFS 5
#define PMM_COUNTER_KMM 6
#define PMM_COUNTER_NETWORK 7
#define PMM_COUNTER_UMM 8
#define PMM_COUNTER_USER 9
#define PMM_COUNTER_USER_STACK 10
#define PMM_COUNTER_VMM 11
#define PMM_COUNTER_MAX PMM_COUNTER_VMM



typedef struct _PMM_ALLOCATOR_PAGE_HEADER{
	struct _PMM_ALLOCATOR_PAGE_HEADER* prev;
	struct _PMM_ALLOCATOR_PAGE_HEADER* next;
	u8 idx;
} pmm_allocator_page_header_t;



typedef struct _PMM_ALLOCATOR{
	u64 first_address;
	u64 last_address;
	u64* bitmap;
	pmm_allocator_page_header_t* blocks[PMM_ALLOCATOR_SIZE_COUNT];
	lock_t lock;
	u16 block_bitmap;
} pmm_allocator_t;



typedef struct _PMM_COUNTERS{
	u64 data[PMM_COUNTER_MAX+1];
} pmm_counters_t;



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



void pmm_get_counters(pmm_counters_t* out);



#endif
