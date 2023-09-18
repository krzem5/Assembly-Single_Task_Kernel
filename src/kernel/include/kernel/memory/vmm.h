#ifndef _KERNEL_MEMORY_VMM_H_
#define _KERNEL_MEMORY_VMM_H_ 1
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define VMM_PAGE_FLAG_PRESENT 0x01
#define VMM_PAGE_FLAG_READWRITE 0x02
#define VMM_PAGE_FLAG_USER 0x04
#define VMM_PAGE_FLAG_LARGE 0x80
#define VMM_PAGE_FLAG_EXTRA_LARGE 0x0004000000000000ull
#define VMM_PAGE_FLAG_NOEXECUTE 0x8000000000000000ull

#define VMM_PAGE_COUNT_MASK 0x7ff0000000000000ull
#define VMM_PAGE_COUNT_SHIFT 52

#define VMM_MAP_WITH_COUNT 0x0008000000000000ull

#define VMM_PAGE_ADDRESS_MASK 0x0000fffffffff000ull

#define VMM_SHADOW_PAGE_ADDRESS 0x1000

#define VMM_HIGHER_HALF_ADDRESS_OFFSET 0xffff800000000000ull



typedef struct _VMM_PAGEMAP_TABLE{
	u64 entries[512];
} vmm_pagemap_table_t;



typedef struct _VMM_PAGEMAP{
	u64 toplevel;
	u16 ownership_limit;
	lock_t lock;
} vmm_pagemap_t;



extern vmm_pagemap_t vmm_kernel_pagemap;



void vmm_init(void);



void vmm_pagemap_init(vmm_pagemap_t* pagemap);



void vmm_pagemap_deinit(vmm_pagemap_t* pagemap);



void vmm_map_page(vmm_pagemap_t* pagemap,u64 physical_address,u64 virtual_address,u64 flags);



void vmm_map_pages(vmm_pagemap_t* pagemap,u64 physical_address,u64 virtual_address,u64 flags,u64 count);



u64 vmm_unmap_page(vmm_pagemap_t* pagemap,u64 virtual_address);



u64 vmm_identity_map(u64 physical_address,u64 size);



u64 vmm_virtual_to_physical(vmm_pagemap_t* pagemap,u64 virtual_address);



void vmm_reserve_pages(vmm_pagemap_t* pagemap,u64 virtual_address,u64 flags,u64 count);



void vmm_commit_pages(vmm_pagemap_t* pagemap,u64 virtual_address,u64 flags,u64 count);



void vmm_release_pages(vmm_pagemap_t* pagemap,u64 virtual_address,u64 count);



void vmm_update_address_and_set_present(vmm_pagemap_t* pagemap,u64 physical_address,u64 virtual_address);



u64 vmm_get_fault_address(void);



void vmm_invalidate_tlb_entry(u64 address);



void vmm_switch_to_pagemap(const vmm_pagemap_t* pagemap);



#endif
