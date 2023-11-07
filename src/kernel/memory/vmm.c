#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "vmm"



static pmm_counter_descriptor_t _vmm_pmm_counter=PMM_COUNTER_INIT_STRUCT("vmm");



vmm_pagemap_t vmm_kernel_pagemap;



static KERNEL_INLINE vmm_pagemap_table_t* _get_table(u64* entry){
	return (vmm_pagemap_table_t*)(((*entry)&VMM_PAGE_ADDRESS_MASK)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
}



static KERNEL_INLINE _Bool _decrease_length(u64* table){
	if (!((*table)&VMM_PAGE_FLAG_PRESENT)){
		return 0;
	}
	*table-=1ull<<VMM_PAGE_COUNT_SHIFT;
	if ((*table)&VMM_PAGE_COUNT_MASK){
		return 0;
	}
	pmm_dealloc((*table)&VMM_PAGE_ADDRESS_MASK,1,&_vmm_pmm_counter);
	*table=0;
	return 1;
}



static KERNEL_INLINE void _increase_length(u64* table){
	if ((*table)&VMM_PAGE_FLAG_PRESENT){
		*table+=1ull<<VMM_PAGE_COUNT_SHIFT;
	}
}



static KERNEL_INLINE void _increase_length_if_entry_empty(u64* table,u64 entry){
	if (!_get_table(table)->entries[entry]){
		_increase_length(table);
	}
}



static void _delete_pagemap_recursive(u64* table,u8 level,u16 limit){
	u64 entry=*table;
	if (!(entry&VMM_PAGE_FLAG_PRESENT)||(entry&VMM_PAGE_FLAG_LARGE)){
		return;
	}
	u64* entries=_get_table(table)->entries;
	if (level>1){
		for (u16 i=0;i<limit;i++){
			_delete_pagemap_recursive(entries+i,level-1,512);
		}
	}
	pmm_dealloc(entry&VMM_PAGE_ADDRESS_MASK,1,&_vmm_pmm_counter);
}



static u64* _get_child_table(u64* table,u64 index,_Bool allocate_if_not_present){
	u64* entry=_get_table(table)->entries+index;
	if (*entry){
		return entry;
	}
	if (!allocate_if_not_present){
		return NULL;
	}
	_increase_length(table);
	u64 out=pmm_alloc(1,&_vmm_pmm_counter,0);
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



static u64 _unmap_page(vmm_pagemap_t* pagemap,u64 virtual_address){
	u64 i=(virtual_address>>39)&0x1ff;
	u64 j=(virtual_address>>30)&0x1ff;
	u64 k=(virtual_address>>21)&0x1ff;
	u64 l=(virtual_address>>12)&0x1ff;
	u64* pml4=&(pagemap->toplevel);
	u64* pml3=_get_child_table(pml4,i,0);
	if (!pml3){
		return 0;
	}
	u64 entry=_get_table(pml3)->entries[j];
	if (entry&VMM_PAGE_FLAG_LARGE){
		if (virtual_address&(EXTRA_LARGE_PAGE_SIZE-1)){
			return 0;
		}
		_get_table(pml3)->entries[j]=0;
		_decrease_length(pml3);
		return entry;
	}
	u64* pml2=_get_child_table(pml3,j,0);
	if (!pml2){
		return 0;
	}
	entry=_get_table(pml2)->entries[k];
	if (entry&VMM_PAGE_FLAG_LARGE){
		if (virtual_address&(LARGE_PAGE_SIZE-1)){
			return 0;
		}
		_get_table(pml2)->entries[k]=0;
		if (_decrease_length(pml2)){
			_decrease_length(pml3);
		}
		return entry;
	}
	u64* pml1=_get_child_table(pml2,k,0);
	if (!pml1){
		return 0;
	}
	entry=_get_table(pml1)->entries[l];
	if (!entry){
		return 0;
	}
	_get_table(pml1)->entries[l]=0;
	if (_decrease_length(pml1)){
		if (_decrease_length(pml2)){
			_decrease_length(pml3);
		}
	}
	return entry;
}



void vmm_init(void){
	LOG("Initializing virtual memory manager...");
	vmm_kernel_pagemap.toplevel=pmm_alloc(1,&_vmm_pmm_counter,PMM_MEMORY_HINT_LOW_MEMORY);
	spinlock_init(&(vmm_kernel_pagemap.lock));
	INFO("Kernel top-level page map allocated at %p",vmm_kernel_pagemap.toplevel);
	for (u32 i=256;i<512;i++){
		_get_table(&(vmm_kernel_pagemap.toplevel))->entries[i]=pmm_alloc(1,&_vmm_pmm_counter,0)|VMM_PAGE_FLAG_USER|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT;
	}
	u64 kernel_length=pmm_align_up_address(kernel_data.first_free_address);
	INFO("Mapping %v from %p to %p",kernel_length,NULL,kernel_get_offset());
	for (u64 i=0;i<kernel_length;i+=PAGE_SIZE){
		vmm_map_page(&vmm_kernel_pagemap,i,i+kernel_get_offset(),VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	}
	u64 highest_address=0;
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		u64 end=pmm_align_up_address_extra_large((kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length);
		if (end>highest_address){
			highest_address=end;
		}
	}
	INFO("Identity mapping first %v...",highest_address);
	for (u64 i=0;i<highest_address;i+=EXTRA_LARGE_PAGE_SIZE){
		vmm_map_page(&vmm_kernel_pagemap,i,i+VMM_HIGHER_HALF_ADDRESS_OFFSET,VMM_PAGE_FLAG_NOEXECUTE|VMM_PAGE_FLAG_EXTRA_LARGE|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_PRESENT);
	}
	LOG("Switching to kernel pagemap...");
	vmm_switch_to_pagemap(&vmm_kernel_pagemap);
}



void vmm_pagemap_init(vmm_pagemap_t* pagemap){
	pagemap->toplevel=pmm_alloc(1,&_vmm_pmm_counter,0);
	spinlock_init(&(pagemap->lock));
	for (u16 i=256;i<512;i++){
		_get_table(&(pagemap->toplevel))->entries[i]=_get_table(&(vmm_kernel_pagemap.toplevel))->entries[i];
	}
}



void vmm_pagemap_deinit(vmm_pagemap_t* pagemap){
	_delete_pagemap_recursive(&(pagemap->toplevel),4,256);
}



void vmm_map_page(vmm_pagemap_t* pagemap,u64 physical_address,u64 virtual_address,u64 flags){
	scheduler_pause();
	u64 i=(virtual_address>>39)&0x1ff;
	u64 j=(virtual_address>>30)&0x1ff;
	u64 k=(virtual_address>>21)&0x1ff;
	u64 l=(virtual_address>>12)&0x1ff;
	spinlock_acquire_exclusive(&(pagemap->lock));
	u64* pml4=&(pagemap->toplevel);
	u64* pml3=_get_child_table(pml4,i,1);
	if (flags&VMM_PAGE_FLAG_EXTRA_LARGE){
		if (pmm_align_down_address_extra_large(physical_address)!=physical_address||pmm_align_down_address_extra_large(virtual_address)!=virtual_address){
			panic("vmm_map_page: invalid vmm_map_page arguments");
		}
		if (_get_table(pml3)->entries[j]&VMM_PAGE_FLAG_PRESENT){
			panic("vmm_map_page: memory mapping already present");
		}
		_increase_length_if_entry_empty(pml3,j);
		_get_table(pml3)->entries[j]=(physical_address&VMM_PAGE_ADDRESS_MASK)|(flags&(~VMM_PAGE_FLAG_EXTRA_LARGE))|VMM_PAGE_FLAG_LARGE;
		goto _cleanup;
	}
	u64* pml2=_get_child_table(pml3,j,1);
	if (flags&VMM_PAGE_FLAG_LARGE){
		if (pmm_align_down_address_large(physical_address)!=physical_address||pmm_align_down_address_large(virtual_address)!=virtual_address){
			panic("vmm_map_page: invalid vmm_map_page arguments");
		}
		if (_get_table(pml2)->entries[k]&VMM_PAGE_FLAG_PRESENT){
			panic("vmm_map_page: memory mapping already present");
		}
		_increase_length_if_entry_empty(pml2,k);
		_get_table(pml2)->entries[k]=(physical_address&VMM_PAGE_ADDRESS_MASK)|(flags&(~VMM_PAGE_FLAG_EXTRA_LARGE))|VMM_PAGE_FLAG_LARGE;
		goto _cleanup;
	}
	if (pmm_align_down_address(physical_address)!=physical_address||pmm_align_down_address(virtual_address)!=virtual_address){
		panic("vmm_map_page: invalid vmm_map_page arguments");
	}
	u64* pml1=_get_child_table(pml2,k,1);
	if (_get_table(pml1)->entries[l]&VMM_PAGE_FLAG_PRESENT){
		panic("vmm_map_page: memory mapping already present");
	}
	_increase_length_if_entry_empty(pml1,l);
	_get_table(pml1)->entries[l]=(physical_address&VMM_PAGE_ADDRESS_MASK)|(flags&(~VMM_PAGE_FLAG_EXTRA_LARGE));
_cleanup:
	spinlock_release_exclusive(&(pagemap->lock));
	scheduler_resume();
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
	u64 index=0;
	while (index<count){
		vmm_map_page(pagemap,physical_address+(index<<stride_shift),virtual_address+(index<<stride_shift),flags);
		index++;
	}
}



u64 vmm_unmap_page(vmm_pagemap_t* pagemap,u64 virtual_address){
	if (virtual_address&(PAGE_SIZE-1)){
		return 0;
	}
	scheduler_pause();
	spinlock_acquire_exclusive(&(pagemap->lock));
	u64 out=_unmap_page(pagemap,virtual_address);
	spinlock_release_exclusive(&(pagemap->lock));
	scheduler_resume();
	return out;
}



u64 vmm_identity_map(u64 physical_address,u64 size){
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



u64 vmm_virtual_to_physical(vmm_pagemap_t* pagemap,u64 virtual_address){
	u64 out=0;
	scheduler_pause();
	spinlock_acquire_shared(&(pagemap->lock));
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
	spinlock_release_shared(&(pagemap->lock));
	scheduler_resume();
	return out;
}



void vmm_adjust_flags(vmm_pagemap_t* pagemap,u64 virtual_address,u64 set_flags,u64 clear_flags,u64 count){
	scheduler_pause();
	spinlock_acquire_exclusive(&(pagemap->lock));
	for (;count;count--){
		u64* entry=_lookup_virtual_address(pagemap,virtual_address);
		if (entry){
			*entry=((*entry)&(~clear_flags))|set_flags;
		}
		virtual_address+=PAGE_SIZE;
	}
	spinlock_release_exclusive(&(pagemap->lock));
	scheduler_resume();
}



_Bool vmm_is_user_accessible(vmm_pagemap_t* pagemap,u64 virtual_address,u64 count){
	scheduler_pause();
	spinlock_acquire_shared(&(pagemap->lock));
	_Bool out=1;
	for (;count;count--){
		u64* entry=_lookup_virtual_address(pagemap,virtual_address);
		out&=(entry&&*entry&&((*entry)&VMM_PAGE_FLAG_USER));
		virtual_address+=PAGE_SIZE;
	}
	spinlock_release_shared(&(pagemap->lock));
	scheduler_resume();
	return out;
}
