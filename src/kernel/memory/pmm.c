#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>



static pmm_allocator_t KERNEL_CORE_DATA _pmm_allocator;



static inline u8 KERNEL_CORE_CODE _get_block_index(u64 address){
	u8 out=__builtin_ctzll(address)-PAGE_SIZE_SHIFT;
	return (out>PMM_ALLOCATOR_SIZE_COUNT?PMM_ALLOCATOR_SIZE_COUNT:out);
}



static inline u8 KERNEL_CORE_CODE _get_largest_block_index(u64 address){
	u8 out=63-__builtin_clzll(address)-PAGE_SIZE_SHIFT;
	return (out>PMM_ALLOCATOR_SIZE_COUNT?PMM_ALLOCATOR_SIZE_COUNT:out);
}



static inline u64 KERNEL_CORE_CODE _get_block_size(u8 index){
	return 1ull<<(PAGE_SIZE_SHIFT+index);
}



static void KERNEL_CORE_CODE _add_memory_range(u64 address,u64 end){
	while (address<end){
		u8 idx=_get_block_index(address);
		u64 size=_get_block_size(idx);
		u64 length=end-address;
		if (size>length){
			idx=_get_largest_block_index(length);
			size=_get_block_size(idx);
		}
		pmm_allocator_page_header_t* header=VMM_TRANSLATE_ADDRESS(address);
		header->next=_pmm_allocator.blocks[idx];
		_pmm_allocator.blocks[idx]=address;
		address+=size;
	}
}



void KERNEL_CORE_CODE pmm_init(const kernel_data_t* kernel_data){
	LOG("Initializing physical memory manager...");
	INFO("Initializing allocator...");
	_pmm_allocator.bitmap=0;
	for (u8 i=0;i<PMM_ALLOCATOR_SIZE_COUNT;i++){
		_pmm_allocator.blocks[i]=0;
	}
	LOG("Registering low memory...");
	u64 last_memory_address=0;
	for (u16 i=0;i<kernel_data->mmap_size;i++){
		if ((kernel_data->mmap+i)->type!=1){
			continue;
		}
		u64 address=pmm_align_up_address((kernel_data->mmap+i)->base);
		if (!address){
			address+=PAGE_SIZE;
		}
		if (address<pmm_align_up_address(kernel_get_end())){
			address=pmm_align_up_address(kernel_get_end());
		}
		u64 end=pmm_align_down_address((kernel_data->mmap+i)->base+(kernel_data->mmap+i)->length);
		if (end>last_memory_address){
			last_memory_address=end;
		}
		if (end>=0x3fffffffull){
			end=0x3fffffffull;
		}
		_add_memory_range(address,end);
	}
	INFO("Allocating allocator bitmap...");
	u64 bitmap_size=pmm_align_up_address((((last_memory_address>>PAGE_SIZE_SHIFT)+64)>>6)<<3); // 64 instead of 63 to add one more bit for the end of the last memory page
	INFO("Bitmap size: %v",bitmap_size);
	_pmm_allocator.bitmap=pmm_alloc(bitmap_size>>PAGE_SIZE_SHIFT);
}



void KERNEL_CORE_CODE pmm_init_high_mem(const kernel_data_t* kernel_data){
	LOG("Registering high memory...");
	for (u16 i=0;i<kernel_data->mmap_size;i++){
		if ((kernel_data->mmap+i)->type!=1){
			continue;
		}
		u64 address=pmm_align_up_address((kernel_data->mmap+i)->base);
		if (address<0x3fffffffull){
			address=0x3fffffffull;
		}
		u64 end=pmm_align_down_address((kernel_data->mmap+i)->base+(kernel_data->mmap+i)->length);
		_add_memory_range(address,end);
	}
}



u64 KERNEL_CORE_CODE pmm_alloc_raw(u64 count){
	if (!count){
		ERROR("Trying to allocate zero physical pages!");
		for (;;);
		return 0;
	}
	u8 i=63-__builtin_clzll(count);
	if ((_get_block_size(i)>>PAGE_SIZE_SHIFT)!=count){
		i++;
		if (i>PMM_ALLOCATOR_SIZE_COUNT){
			ERROR("Trying to allocate too many pages at once!");
			for (;;);
			return 0;
		}
	}
	u64 out=0;
	if (_pmm_allocator.blocks[i]){
		out=_pmm_allocator.blocks[i];
		const pmm_allocator_page_header_t* header=VMM_TRANSLATE_ADDRESS(out);
		_pmm_allocator.blocks[i]=header->next;
		goto _toggle_bitmap;
	}
	u8 j=i;
	do{
		j++;
		if (j==PMM_ALLOCATOR_SIZE_COUNT){
			ERROR("Out of memory!");
			for (;;);
			return 0;
		}
	} while (!_pmm_allocator.blocks[j]);
	out=_pmm_allocator.blocks[j];
	pmm_allocator_page_header_t* header=VMM_TRANSLATE_ADDRESS(out);
	_pmm_allocator.blocks[j]=header->next;
	do{
		j--;
		u64 child_block=out+_get_block_size(j);
		header=VMM_TRANSLATE_ADDRESS(child_block);
		header->next=_pmm_allocator.blocks[j];
		_pmm_allocator.blocks[j]=child_block;
	} while (j>i);
_toggle_bitmap:
	if (_pmm_allocator.bitmap){
		u64* bitmap=VMM_TRANSLATE_ADDRESS(_pmm_allocator.bitmap);
		u64 k=out>>PAGE_SIZE_SHIFT;
		bitmap[k>>6]^=1ull<<(k&63);
	}
	return out;
}



u64 KERNEL_CORE_CODE pmm_alloc(u64 count){
	u64 out=pmm_alloc_raw(count);
	if (!out){
		return 0;
	}
	u64* data=VMM_TRANSLATE_ADDRESS(out);
	count<<=PAGE_SIZE_SHIFT-3;
	do{
		*data=0;
		count--;
		data++;
	} while (count);
	return out;
}



void KERNEL_CORE_CODE pmm_dealloc(u64 address,u64 count){
	if (!count){
		ERROR("Trying to deallocate zero physical pages!");
		return;
	}
	u8 i=63-__builtin_clzll(count);
	if ((_get_block_size(i)>>PAGE_SIZE_SHIFT)!=count){
		i++;
		if (i>PMM_ALLOCATOR_SIZE_COUNT){
			ERROR("Trying to deallocate too many pages at once!");
			return;
		}
	}
	u64* bitmap=VMM_TRANSLATE_ADDRESS(_pmm_allocator.bitmap);
	u64 j=address>>PAGE_SIZE_SHIFT;
	u64 mask=1ull<<(j&63);
	bitmap[j>>6]^=mask;
	while (i<PMM_ALLOCATOR_SIZE_COUNT-1){
		// implement block coalescing
		break;
	}
	pmm_allocator_page_header_t* header=VMM_TRANSLATE_ADDRESS(address);
	header->next=_pmm_allocator.blocks[i];
	_pmm_allocator.blocks[i]=address;
	return;
}
