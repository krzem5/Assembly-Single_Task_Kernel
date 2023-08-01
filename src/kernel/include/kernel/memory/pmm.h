#ifndef _KERNEL_MEMORY_PMM_H_
#define _KERNEL_MEMORY_PMM_H_ 1
#include <kernel/types.h>



#define PAGE_SIZE 4096
#define PAGE_SIZE_SHIFT 12

#define PMM_ALLOCATOR_SIZE_COUNT 16

#define PMM_COUNTER_TOTAL 0
#define PMM_COUNTER_DRIVE_LIST 1
#define PMM_COUNTER_DRIVER_AHCI 2
#define PMM_COUNTER_DRIVER_I82540 3
#define PMM_COUNTER_ELF 4
#define PMM_COUNTER_FD 5
#define PMM_COUNTER_FS 6
#define PMM_COUNTER_KERNEL_STACK 7
#define PMM_COUNTER_KFS 8
#define PMM_COUNTER_MMAP 9
#define PMM_COUNTER_NETWORK 10
#define PMM_COUNTER_NODE_ALLOCATOR 11
#define PMM_COUNTER_PMM 12
#define PMM_COUNTER_USER_STACK 13
#define PMM_COUNTER_VMM 14
#define PMM_COUNTER_MAX PMM_COUNTER_VMM



typedef struct _PMM_ALLOCATOR_PAGE_HEADER{
	u64 next;
} pmm_allocator_page_header_t;



typedef struct _PMM_ALLOCATOR{
	u64 bitmap;
	u64 blocks[PMM_ALLOCATOR_SIZE_COUNT];
	u64 counters[PMM_COUNTER_MAX+1];
} pmm_allocator_t;



static inline u64 pmm_align_up_address(u64 base){
	return (base+PAGE_SIZE-1)&(~(PAGE_SIZE-1));
}



static inline u64 pmm_align_down_address(u64 base){
	return base&(~(PAGE_SIZE-1));
}



void pmm_init(const kernel_data_t* kernel_data);



void pmm_init_high_mem(const kernel_data_t* kernel_data);



u64 pmm_alloc(u64 count,u8 counter);



u64 pmm_alloc_zero(u64 count,u8 counter);



void pmm_dealloc(u64 address,u64 count);



#endif
