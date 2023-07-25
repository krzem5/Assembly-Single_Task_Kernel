#ifndef _KERNEL_MEMORY_VMM_H_
#define _KERNEL_MEMORY_VMM_H_ 1
#include <kernel/lock/lock.h>
#include <kernel/types.h>



#define VMM_TRANSLATE_ADDRESS(address) ((void*)(((u64)(address))+vmm_address_offset))
#define VMM_TRANSLATE_ADDRESS_REVERSE(address) (((u64)(address))-vmm_address_offset)

#define VMM_PAGE_FLAG_PRESENT 0x1
#define VMM_PAGE_FLAG_READWRITE 0x2
#define VMM_PAGE_FLAG_USER 0x4
#define VMM_PAGE_FLAG_NOEXECUTE 0x8000000000000000ull

#define VMM_PAGE_COUNT_MASK 0x7ff0000000000000ull
#define VMM_PAGE_COUNT_SHIFT 52

#define VMM_MAP_WITH_COUNT 0x0008000000000000ull

#define VMM_PAGE_ADDRESS_MASK 0x0000fffffffff000ull



typedef struct _VMM_PAGEMAP_TABLE{
	u64 entries[512];
} vmm_pagemap_table_t;



typedef struct _VMM_PAGEMAP{
	vmm_pagemap_table_t* toplevel;
	lock_t lock;
} vmm_pagemap_t;



extern u64 vmm_address_offset;
extern vmm_pagemap_t vmm_kernel_pagemap;
extern vmm_pagemap_t vmm_user_pagemap;



void vmm_init(const kernel_data_t* kernel_data);



void vmm_pagemap_init(vmm_pagemap_t* pagemap);



void vmm_pagemap_deinit(vmm_pagemap_t* pagemap);



void vmm_map_page(vmm_pagemap_t* pagemap,u64 physical_address,u64 virtual_address,u64 flags);



void vmm_map_pages(vmm_pagemap_t* pagemap,u64 physical_address,u64 virtual_address,u64 flags,u64 count);



_Bool vmm_unmap_page(vmm_pagemap_t* pagemap,u64 virtual_address);



u64 vmm_virtual_to_physical(vmm_pagemap_t* pagemap,u64 virtual_address);



void vmm_switch_to_pagemap(const vmm_pagemap_t* pagemap);



void vmm_set_common_kernel_pagemap(void);



#endif
