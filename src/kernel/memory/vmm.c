#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "vmm"



#define VMM_HIGHER_HALF_ADDRESS_OFFSET 0xffff800000000000ull



u64 KERNEL_CORE_DATA vmm_address_offset=KERNEL_OFFSET;
vmm_pagemap_t KERNEL_CORE_DATA vmm_kernel_pagemap;
vmm_pagemap_t KERNEL_CORE_DATA vmm_user_pagemap;



static inline vmm_pagemap_table_t* KERNEL_CORE_CODE _get_table(u64* entry){
	return (vmm_pagemap_table_t*)VMM_TRANSLATE_ADDRESS((*entry)&VMM_PAGE_ADDRESS_MASK);
}



static inline _Bool KERNEL_CORE_CODE _decrease_length(u64* table){
	if ((*table)&VMM_PAGE_FLAG_PRESENT){
		*table-=1ull<<VMM_PAGE_COUNT_SHIFT;
		if (!((*table)&VMM_PAGE_COUNT_MASK)){
			pmm_dealloc((*table)&VMM_PAGE_ADDRESS_MASK,1);
			*table=0;
			return 1;
		}
	}
	return 0;
}



static inline void KERNEL_CORE_CODE _increase_length(u64* table){
	if ((*table)&VMM_PAGE_FLAG_PRESENT){
		*table+=1ull<<VMM_PAGE_COUNT_SHIFT;
	}
}



static inline void KERNEL_CORE_CODE _increase_length_if_entry_empty(u64* table,u64 entry){
	if (!_get_table(table)->entries[entry]){
		_increase_length(table);
	}
}



static void _delete_pagemap_recursive(u64 table,u8 level){
	if (!table){
		return;
	}
	u64* entry=VMM_TRANSLATE_ADDRESS(table);
	if (level>1){
		for (u16 i=0;i<512;i++){
			_delete_pagemap_recursive(entry[i]&VMM_PAGE_ADDRESS_MASK,level-1);
		}
	}
	else{
		for (u16 i=0;i<512;i++){
			if (!(entry[i]&VMM_PAGE_FLAG_PRESENT)){
				continue;
			}
			u64 count=(entry[i]&VMM_PAGE_COUNT_MASK)>>VMM_PAGE_COUNT_SHIFT;
			if (!count){
				continue;
			}
			pmm_dealloc(entry[i]&VMM_PAGE_ADDRESS_MASK,count);
		}
	}
	pmm_dealloc(table,1);
}



static u64* KERNEL_CORE_CODE _get_child_table(u64* table,u64 index,_Bool allocate_if_not_present){
	u64* entry=_get_table(table)->entries+index;
	if ((*entry)&VMM_PAGE_FLAG_PRESENT){
		return entry;
	}
	if (!allocate_if_not_present){
		return 0;
	}
	_increase_length(table);
	u64 out=pmm_alloc(1);
	*entry=((u64)out)|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT;
	return entry;
}



void KERNEL_CORE_CODE vmm_init(const kernel_data_t* kernel_data){
	LOG_CORE("Initializing virtual memory manager...");
	vmm_kernel_pagemap.toplevel=pmm_alloc(1);
	lock_init(&(vmm_kernel_pagemap.lock));
	vmm_user_pagemap.toplevel=0;
	lock_init(&(vmm_user_pagemap.lock));
	INFO_CORE("Kernel top-level page map alloated at %p",vmm_kernel_pagemap.toplevel);
	for (u32 i=256;i<512;i++){
		_get_table(&(vmm_kernel_pagemap.toplevel))->entries[i]=pmm_alloc(1)|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT;
	}
	u64 kernel_length=pmm_align_up_address(kernel_get_end());
	INFO_CORE("Mapping %v from %p to %p",kernel_length,NULL,kernel_get_offset());
	for (u64 i=0;i<kernel_length;i+=PAGE_SIZE){
		vmm_map_page(&vmm_kernel_pagemap,i,i+kernel_get_offset(),VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	}
	INFO_CORE("Mapping %v from %p to %p",0x100000000-PAGE_SIZE,PAGE_SIZE,PAGE_SIZE+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	for (u64 i=PAGE_SIZE;i<0x100000000;i+=PAGE_SIZE){
		vmm_map_page(&vmm_kernel_pagemap,i,i+VMM_HIGHER_HALF_ADDRESS_OFFSET,VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	}
	for (u16 i=0;i<kernel_data->mmap_size;i++){
		if ((kernel_data->mmap+i)->type!=1){
			continue;
		}
		u64 end=pmm_align_down_address((kernel_data->mmap+i)->base+(kernel_data->mmap+i)->length);
		if (end<=0x100000000){
			continue;
		}
		u64 address=pmm_align_up_address((kernel_data->mmap+i)->base);
		if (address<0x100000000){
			address=0x100000000;
		}
		INFO_CORE("Mapping %v from %p to %p",end-address,address,address+VMM_HIGHER_HALF_ADDRESS_OFFSET);
		for (;address<end;address+=PAGE_SIZE){
			vmm_map_page(&vmm_kernel_pagemap,address,address+VMM_HIGHER_HALF_ADDRESS_OFFSET,VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
		}
	}
	vmm_switch_to_pagemap(&vmm_kernel_pagemap);
	vmm_address_offset=VMM_HIGHER_HALF_ADDRESS_OFFSET;
}



void vmm_pagemap_init(vmm_pagemap_t* pagemap){
	LOG("Creating new pagemap...");
	pagemap->toplevel=pmm_alloc(1);
	lock_init(&(pagemap->lock));
	LOG("Mapping common section...");
	INFO("Mapping %v from %p to %p",kernel_get_end()-kernel_get_common_start(),kernel_get_common_start(),kernel_get_common_start()+kernel_get_offset());
	vmm_map_pages(pagemap,kernel_get_common_start(),kernel_get_common_start()+kernel_get_offset(),VMM_PAGE_FLAG_PRESENT,pmm_align_up_address(kernel_get_end()-kernel_get_common_start())>>PAGE_SIZE_SHIFT);
}



void vmm_pagemap_deinit(vmm_pagemap_t* pagemap){
	_delete_pagemap_recursive((u64)(pagemap->toplevel),4);
}



void KERNEL_CORE_CODE vmm_map_page(vmm_pagemap_t* pagemap,u64 physical_address,u64 virtual_address,u64 flags){
	if (pmm_align_down_address(physical_address)!=physical_address||pmm_align_down_address(virtual_address)!=virtual_address){
		ERROR_CORE("Invalid vmm_map_page arguments");
		for (;;);
	}
	lock_acquire(&(pagemap->lock));
	u64 i=(virtual_address>>39)&0x1ff;
	u64 j=(virtual_address>>30)&0x1ff;
	u64 k=(virtual_address>>21)&0x1ff;
	u64 l=(virtual_address>>12)&0x1ff;
	u64* pml4=&(pagemap->toplevel);
	u64* pml3=_get_child_table(pml4,i,1);
	u64* pml2=_get_child_table(pml3,j,1);
	u64* pml1=_get_child_table(pml2,k,1);
	_increase_length_if_entry_empty(pml1,l);
	_get_table(pml1)->entries[l]=(physical_address&VMM_PAGE_ADDRESS_MASK)|flags;
	lock_release(&(pagemap->lock));
}



void vmm_map_pages(vmm_pagemap_t* pagemap,u64 physical_address,u64 virtual_address,u64 flags,u64 size){
	if (!size){
		return;
	}
	_Bool has_count=!!(flags&VMM_MAP_WITH_COUNT);
	flags&=~(VMM_PAGE_COUNT_MASK|VMM_MAP_WITH_COUNT);
	u64 index=0;
	while (index<size){
		if (has_count&&!(index&0x7ff)){
			u64 diff=size-index;
			flags|=(diff>2047?2047:diff)<<VMM_PAGE_COUNT_SHIFT;
		}
		vmm_map_page(pagemap,physical_address+(index<<PAGE_SIZE_SHIFT),virtual_address+(index<<PAGE_SIZE_SHIFT),flags);
		flags&=~VMM_PAGE_COUNT_MASK;
		index++;
	}
}



_Bool vmm_unmap_page(vmm_pagemap_t* pagemap,u64 virtual_address){
	lock_acquire(&(pagemap->lock));
	u64 i=(virtual_address>>39)&0x1ff;
	u64 j=(virtual_address>>30)&0x1ff;
	u64 k=(virtual_address>>21)&0x1ff;
	u64 l=(virtual_address>>12)&0x1ff;
	u64* pml4=&(pagemap->toplevel);
	u64* pml3=_get_child_table(pml4,i,0);
	if (!pml3){
		lock_release(&(pagemap->lock));
		return 0;
	}
	u64* pml2=_get_child_table(pml3,j,0);
	if (!pml2){
		lock_release(&(pagemap->lock));
		return 0;
	}
	u64* pml1=_get_child_table(pml2,k,0);
	if (!pml1){
		lock_release(&(pagemap->lock));
		return 0;
	}
	_get_table(pml1)->entries[l]=0;
	if (_decrease_length(pml1)){
		if (_decrease_length(pml2)){
			_decrease_length(pml3);
		}
	}
	lock_release(&(pagemap->lock));
	return 1;
}



u64 vmm_virtual_to_physical(vmm_pagemap_t* pagemap,u64 virtual_address){
	lock_acquire(&(pagemap->lock));
	u64 i=(virtual_address>>39)&0x1ff;
	u64 j=(virtual_address>>30)&0x1ff;
	u64 k=(virtual_address>>21)&0x1ff;
	u64 l=(virtual_address>>12)&0x1ff;
	u64* pml4=&(pagemap->toplevel);
	u64* pml3=_get_child_table(pml4,i,0);
	if (!pml3){
		return 0;
	}
	u64* pml2=_get_child_table(pml3,j,0);
	if (!pml2){
		return 0;
	}
	u64* pml1=_get_child_table(pml2,k,0);
	if (!pml1){
		return 0;
	}
	u64 entry=_get_table(pml1)->entries[l];
	lock_release(&(pagemap->lock));
	return ((entry&VMM_PAGE_FLAG_PRESENT)?(entry&VMM_PAGE_ADDRESS_MASK)+(virtual_address&0xfff):0);
}
