#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "vmm"



static u64 _vmm_address_offset=0;

vmm_pagemap_t KERNEL_CORE_BSS vmm_kernel_pagemap;
vmm_pagemap_t KERNEL_CORE_BSS vmm_shared_pagemap;



static inline vmm_pagemap_table_t* KERNEL_CORE_CODE _get_table(u64* entry){
	return (vmm_pagemap_table_t*)(((*entry)&VMM_PAGE_ADDRESS_MASK)+_vmm_address_offset);
}



static inline _Bool KERNEL_CORE_CODE _decrease_length(u64* table){
	if ((*table)&VMM_PAGE_FLAG_PRESENT){
		*table-=1ull<<VMM_PAGE_COUNT_SHIFT;
		if (!((*table)&VMM_PAGE_COUNT_MASK)){
			pmm_dealloc((*table)&VMM_PAGE_ADDRESS_MASK,1,PMM_COUNTER_VMM);
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



static void _delete_pagemap_recursive(u64* table,u8 level,u16 limit){
	u64 entry=*table;
	if (!(entry&VMM_PAGE_FLAG_PRESENT)){
		return;
	}
	if (entry&VMM_PAGE_FLAG_LARGE){
		u64 count=(entry&VMM_PAGE_COUNT_MASK)>>VMM_PAGE_COUNT_SHIFT;
		if (count){
			pmm_dealloc(entry&VMM_PAGE_ADDRESS_MASK,count<<(level==3?EXTRA_LARGE_PAGE_SIZE_SHIFT-PAGE_SIZE_SHIFT:LARGE_PAGE_SIZE_SHIFT-PAGE_SIZE_SHIFT),PMM_COUNTER_USER);
		}
		return;
	}
	u64* entries=_get_table(table)->entries;
	if (level>1){
		for (u16 i=0;i<limit;i++){
			_delete_pagemap_recursive(entries+i,level-1,512);
		}
	}
	else{
		for (u16 i=0;i<limit;i++){
			if (!(entries[i]&VMM_PAGE_FLAG_PRESENT)){
				continue;
			}
			u64 count=(entries[i]&VMM_PAGE_COUNT_MASK)>>VMM_PAGE_COUNT_SHIFT;
			if (!count){
				continue;
			}
			pmm_dealloc(entries[i]&VMM_PAGE_ADDRESS_MASK,count,PMM_COUNTER_USER);
		}
	}
	pmm_dealloc(entry&VMM_PAGE_ADDRESS_MASK,1,PMM_COUNTER_VMM);
}



static u64* KERNEL_CORE_CODE _get_child_table(u64* table,u64 index,_Bool allocate_if_not_present){
	u64* entry=_get_table(table)->entries+index;
	if (*entry){
		return entry;
	}
	if (!allocate_if_not_present){
		return 0;
	}
	_increase_length(table);
	u64 out=pmm_alloc_zero(1,PMM_COUNTER_VMM,0);
	*entry=((u64)out)|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT;
	return entry;
}



static u64* _lookup_virtual_address(vmm_pagemap_t* pagemap,u64 virtual_address){
	u64 i=(virtual_address>>39)&0x1ff;
	u64 j=(virtual_address>>30)&0x1ff;
	u64 k=(virtual_address>>21)&0x1ff;
	u64 l=(virtual_address>>12)&0x1ff;
	u64* pml4=&(pagemap->toplevel);
	u64* pml3=_get_child_table(pml4,i,0);
	if (!pml3){
		return NULL;
	}
	if ((_get_table(pml3)->entries[j])&VMM_PAGE_FLAG_LARGE){
		return _get_table(pml3)->entries+j;
	}
	u64* pml2=_get_child_table(pml3,j,0);
	if (!pml2){
		return NULL;
	}
	if ((_get_table(pml2)->entries[k])&VMM_PAGE_FLAG_LARGE){
		return _get_table(pml2)->entries+k;
	}
	u64* pml1=_get_child_table(pml2,k,0);
	if (!pml1){
		return NULL;
	}
	return _get_table(pml1)->entries+l;
}



void KERNEL_CORE_CODE vmm_init(void){
	LOG_CORE("Initializing virtual memory manager...");
	vmm_kernel_pagemap.toplevel=pmm_alloc_zero(1,PMM_COUNTER_VMM,PMM_MEMORY_HINT_LOW_MEMORY);
	vmm_kernel_pagemap.ownership_limit=512;
	lock_init(&(vmm_kernel_pagemap.lock));
	INFO_CORE("Kernel top-level page map allocated at %p",vmm_kernel_pagemap.toplevel);
	vmm_shared_pagemap.toplevel=pmm_alloc_zero(1,PMM_COUNTER_VMM,PMM_MEMORY_HINT_LOW_MEMORY);
	vmm_shared_pagemap.ownership_limit=512;
	lock_init(&(vmm_shared_pagemap.lock));
	INFO_CORE("Shared top-level page map allocated at %p",vmm_shared_pagemap.toplevel);
	for (u32 i=256;i<512;i++){
		_get_table(&(vmm_kernel_pagemap.toplevel))->entries[i]=pmm_alloc_zero(1,PMM_COUNTER_VMM,0)|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_PRESENT;
		_get_table(&(vmm_shared_pagemap.toplevel))->entries[i]=pmm_alloc_zero(1,PMM_COUNTER_VMM,0)|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_PRESENT;
	}
	u64 kernel_length=pmm_align_up_address(kernel_get_bss_end());
	INFO_CORE("Mapping %v from %p to %p",kernel_length,NULL,kernel_get_offset());
	for (u64 i=0;i<kernel_length;i+=PAGE_SIZE){
		vmm_map_page(&vmm_kernel_pagemap,i,i+kernel_get_offset(),VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	}
	u64 highest_address=0;
	for (u16 i=0;i<KERNEL_DATA->mmap_size;i++){
		u64 end=pmm_align_up_address_extra_large((KERNEL_DATA->mmap+i)->base+(KERNEL_DATA->mmap+i)->length);
		if (end>highest_address){
			highest_address=end;
		}
	}
	INFO_CORE("Identity mapping first %v...",highest_address);
	for (u64 i=0;i<highest_address;i+=EXTRA_LARGE_PAGE_SIZE){
		vmm_map_page(&vmm_kernel_pagemap,i,i+VMM_HIGHER_HALF_ADDRESS_OFFSET,VMM_PAGE_FLAG_EXTRA_LARGE|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	}
	vmm_switch_to_pagemap(&vmm_kernel_pagemap);
	_vmm_address_offset=VMM_HIGHER_HALF_ADDRESS_OFFSET;
}



void vmm_pagemap_init(vmm_pagemap_t* pagemap,vmm_pagemap_t* user_pagemap){
	pagemap->toplevel=pmm_alloc_zero(1,PMM_COUNTER_VMM,0);
	pagemap->ownership_limit=(user_pagemap?0:256);
	lock_init(&(pagemap->lock));
	if (user_pagemap){
		for (u16 i=0;i<256;i++){
			_get_table(&(pagemap->toplevel))->entries[i]=_get_table(&(user_pagemap->toplevel))->entries[i];
		}
	}
	else{
		for (u16 i=0;i<256;i++){
			_get_table(&(pagemap->toplevel))->entries[i]=pmm_alloc_zero(1,PMM_COUNTER_VMM,0)|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT;
		}
	}
	u64* higher_half_pagemap=(user_pagemap?&(vmm_kernel_pagemap.toplevel):&(vmm_kernel_pagemap.toplevel));
	for (u16 i=256;i<512;i++){
		_get_table(&(pagemap->toplevel))->entries[i]=_get_table(higher_half_pagemap)->entries[i];
	}
}



void vmm_pagemap_deinit(vmm_pagemap_t* pagemap){
	_delete_pagemap_recursive(&(pagemap->toplevel),4,pagemap->ownership_limit);
}



void KERNEL_CORE_CODE vmm_map_page(vmm_pagemap_t* pagemap,u64 physical_address,u64 virtual_address,u64 flags){
	u64 i=(virtual_address>>39)&0x1ff;
	u64 j=(virtual_address>>30)&0x1ff;
	u64 k=(virtual_address>>21)&0x1ff;
	u64 l=(virtual_address>>12)&0x1ff;
	lock_acquire_exclusive(&(pagemap->lock));
	u64* pml4=&(pagemap->toplevel);
	u64* pml3=_get_child_table(pml4,i,1);
	if (flags&VMM_PAGE_FLAG_EXTRA_LARGE){
		if (pmm_align_down_address_extra_large(physical_address)!=physical_address||pmm_align_down_address_extra_large(virtual_address)!=virtual_address){
			panic("Invalid vmm_map_page arguments",0);
		}
		if (_get_table(pml3)->entries[j]&VMM_PAGE_FLAG_PRESENT){
			panic("Memory mapping already present",0);
		}
		_increase_length_if_entry_empty(pml3,j);
		_get_table(pml3)->entries[j]=(physical_address&VMM_PAGE_ADDRESS_MASK)|(flags&(~VMM_PAGE_FLAG_EXTRA_LARGE))|VMM_PAGE_FLAG_LARGE;
		goto _cleanup;
	}
	u64* pml2=_get_child_table(pml3,j,1);
	if (flags&VMM_PAGE_FLAG_LARGE){
		if (pmm_align_down_address_large(physical_address)!=physical_address||pmm_align_down_address_large(virtual_address)!=virtual_address){
			panic("Invalid vmm_map_page arguments",0);
		}
		if (_get_table(pml2)->entries[k]&VMM_PAGE_FLAG_PRESENT){
			panic("Memory mapping already present",0);
		}
		_increase_length_if_entry_empty(pml2,k);
		_get_table(pml2)->entries[k]=(physical_address&VMM_PAGE_ADDRESS_MASK)|(flags&(~VMM_PAGE_FLAG_EXTRA_LARGE))|VMM_PAGE_FLAG_LARGE;
		goto _cleanup;
	}
	if (pmm_align_down_address(physical_address)!=physical_address||pmm_align_down_address(virtual_address)!=virtual_address){
		panic("Invalid vmm_map_page arguments",0);
	}
	u64* pml1=_get_child_table(pml2,k,1);
	if (_get_table(pml1)->entries[l]&VMM_PAGE_FLAG_PRESENT){
		panic("Memory mapping already present",0);
	}
	_increase_length_if_entry_empty(pml1,l);
	_get_table(pml1)->entries[l]=(physical_address&VMM_PAGE_ADDRESS_MASK)|(flags&(~VMM_PAGE_FLAG_EXTRA_LARGE));
_cleanup:
	lock_release_exclusive(&(pagemap->lock));
}



void vmm_map_pages(vmm_pagemap_t* pagemap,u64 physical_address,u64 virtual_address,u64 flags,u64 count){
	if (!count){
		return;
	}
	u64 stride_shift=PAGE_SIZE_SHIFT;
	if (flags&VMM_PAGE_FLAG_LARGE){
		stride_shift=LARGE_PAGE_SIZE_SHIFT;
	}
	if (flags&VMM_PAGE_FLAG_EXTRA_LARGE){
		stride_shift=EXTRA_LARGE_PAGE_SIZE_SHIFT;
	}
	count>>=stride_shift-PAGE_SIZE_SHIFT;
	_Bool has_count=!!(flags&VMM_MAP_WITH_COUNT);
	flags&=~(VMM_PAGE_COUNT_MASK|VMM_MAP_WITH_COUNT);
	u64 index=0;
	while (index<count){
		if (has_count&&!(index&0x7ff)){
			u64 diff=count-index;
			flags|=(diff>2047?2047:diff)<<VMM_PAGE_COUNT_SHIFT;
		}
		vmm_map_page(pagemap,physical_address+(index<<stride_shift),virtual_address+(index<<stride_shift),flags);
		flags&=~VMM_PAGE_COUNT_MASK;
		index++;
	}
}



u64 vmm_unmap_page(vmm_pagemap_t* pagemap,u64 virtual_address){
	if (virtual_address&(PAGE_SIZE-1)){
		return 0;
	}
	u64 out=0;
	lock_acquire_exclusive(&(pagemap->lock));
	u64 i=(virtual_address>>39)&0x1ff;
	u64 j=(virtual_address>>30)&0x1ff;
	u64 k=(virtual_address>>21)&0x1ff;
	u64 l=(virtual_address>>12)&0x1ff;
	u64* pml4=&(pagemap->toplevel);
	u64* pml3=_get_child_table(pml4,i,0);
	if (!pml3){
		goto _cleanup;
	}
	u64 entry=_get_table(pml3)->entries[j];
	if (entry&VMM_PAGE_FLAG_LARGE){
		if (!entry||(virtual_address&(EXTRA_LARGE_PAGE_SIZE-1))){
			goto _cleanup;
		}
		u64 count=(entry&VMM_PAGE_COUNT_MASK)>>VMM_PAGE_COUNT_SHIFT;
		if (count){
			pmm_dealloc(entry&VMM_PAGE_ADDRESS_MASK,count<<(EXTRA_LARGE_PAGE_SIZE_SHIFT-PAGE_SIZE_SHIFT),PMM_COUNTER_USER);
		}
		_get_table(pml3)->entries[j]=0;
		_decrease_length(pml3);
		out=EXTRA_LARGE_PAGE_SIZE;
		goto _cleanup;
	}
	u64* pml2=_get_child_table(pml3,j,0);
	if (!pml2){
		goto _cleanup;
	}
	entry=_get_table(pml2)->entries[k];
	if (entry&VMM_PAGE_FLAG_LARGE){
		if (!entry||(virtual_address&(LARGE_PAGE_SIZE-1))){
			goto _cleanup;
		}
		u64 count=(entry&VMM_PAGE_COUNT_MASK)>>VMM_PAGE_COUNT_SHIFT;
		if (count){
			pmm_dealloc(entry&VMM_PAGE_ADDRESS_MASK,count<<(LARGE_PAGE_SIZE_SHIFT-PAGE_SIZE_SHIFT),PMM_COUNTER_USER);
		}
		_get_table(pml2)->entries[k]=0;
		if (_decrease_length(pml2)){
			_decrease_length(pml3);
		}
		out=LARGE_PAGE_SIZE;
		goto _cleanup;
	}
	u64* pml1=_get_child_table(pml2,k,0);
	if (!pml1){
		goto _cleanup;
	}
	entry=_get_table(pml1)->entries[l];
	if (!entry){
		goto _cleanup;
	}
	u64 count=(entry&VMM_PAGE_COUNT_MASK)>>VMM_PAGE_COUNT_SHIFT;
	if (count){
		pmm_dealloc(entry&VMM_PAGE_ADDRESS_MASK,count,PMM_COUNTER_USER);
	}
	_get_table(pml1)->entries[l]=0;
	if (_decrease_length(pml1)){
		if (_decrease_length(pml2)){
			_decrease_length(pml3);
		}
	}
	out=PAGE_SIZE;
_cleanup:
	lock_release_exclusive(&(pagemap->lock));
	return out;
}



u64 KERNEL_CORE_CODE vmm_identity_map(u64 physical_address,u64 size){
	if (!size){
		return physical_address+VMM_HIGHER_HALF_ADDRESS_OFFSET;
	}
	size=pmm_align_up_address(size+physical_address-pmm_align_down_address(physical_address));
	u64 address_aligned=pmm_align_down_address(physical_address);
	while (size){
		size-=PAGE_SIZE;
		if (!vmm_virtual_to_physical(&vmm_kernel_pagemap,address_aligned+size+VMM_HIGHER_HALF_ADDRESS_OFFSET)){
			vmm_map_page(&vmm_kernel_pagemap,address_aligned+size,address_aligned+size+VMM_HIGHER_HALF_ADDRESS_OFFSET,VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
		}
	}
	return physical_address+VMM_HIGHER_HALF_ADDRESS_OFFSET;
}



u64 KERNEL_CORE_CODE vmm_virtual_to_physical(vmm_pagemap_t* pagemap,u64 virtual_address){
	u64 out=0;
	lock_acquire_shared(&(pagemap->lock));
	u64 i=(virtual_address>>39)&0x1ff;
	u64 j=(virtual_address>>30)&0x1ff;
	u64 k=(virtual_address>>21)&0x1ff;
	u64 l=(virtual_address>>12)&0x1ff;
	u64* pml4=&(pagemap->toplevel);
	u64* pml3=_get_child_table(pml4,i,0);
	if (!pml3){
		goto _cleanup;
	}
	u64 entry=_get_table(pml3)->entries[j];
	if (entry&VMM_PAGE_FLAG_LARGE){
		out=(entry?(entry&VMM_PAGE_ADDRESS_MASK)|(virtual_address&(EXTRA_LARGE_PAGE_SIZE-1)):0);
		goto _cleanup;
	}
	u64* pml2=_get_child_table(pml3,j,0);
	if (!pml2){
		goto _cleanup;
	}
	entry=_get_table(pml2)->entries[k];
	if (entry&VMM_PAGE_FLAG_LARGE){
		out=(entry?(entry&VMM_PAGE_ADDRESS_MASK)|(virtual_address&(LARGE_PAGE_SIZE-1)):0);
		goto _cleanup;
	}
	u64* pml1=_get_child_table(pml2,k,0);
	if (!pml1){
		goto _cleanup;
	}
	entry=_get_table(pml1)->entries[l];
	out=(entry?(entry&VMM_PAGE_ADDRESS_MASK)|(virtual_address&(PAGE_SIZE-1)):0);
_cleanup:
	lock_release_shared(&(pagemap->lock));
	return out;
}



void vmm_reserve_pages(vmm_pagemap_t* pagemap,u64 virtual_address,u64 flags,u64 count){
	flags&=~VMM_PAGE_FLAG_PRESENT;
	for (;count;count--){
		vmm_map_page(pagemap,VMM_SHADOW_PAGE_ADDRESS,virtual_address,flags);
		virtual_address+=PAGE_SIZE;
	}
}



void vmm_release_pages(vmm_pagemap_t* pagemap,u64 virtual_address,u64 count){
	for (;count;count--){
		lock_acquire_shared(&(pagemap->lock));
		u64 physical_address=(*_lookup_virtual_address(pagemap,virtual_address))&VMM_PAGE_ADDRESS_MASK;
		if (physical_address!=VMM_SHADOW_PAGE_ADDRESS){
			lock_shared_to_exclusive(&(pagemap->lock));
			pmm_dealloc(physical_address,1,PMM_COUNTER_USER);
			lock_release_exclusive(&(pagemap->lock));
		}
		else{
			lock_release_shared(&(pagemap->lock));
		}
		vmm_unmap_page(pagemap,virtual_address);
		virtual_address+=PAGE_SIZE;
	}
}



void vmm_update_address_and_set_present(vmm_pagemap_t* pagemap,u64 physical_address,u64 virtual_address){
	lock_acquire_shared(&(pagemap->lock));
	u64* address=_lookup_virtual_address(pagemap,virtual_address);
	if (address){
		lock_shared_to_exclusive(&(pagemap->lock));
		*address=((*address)&(~VMM_PAGE_ADDRESS_MASK))|(physical_address&VMM_PAGE_ADDRESS_MASK)|VMM_PAGE_FLAG_PRESENT;
		lock_release_exclusive(&(pagemap->lock));
	}
	else{
		lock_release_shared(&(pagemap->lock));
	}
}
