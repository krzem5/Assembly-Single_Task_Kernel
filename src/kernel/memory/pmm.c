#include <kernel/kernel.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "pmm"



static pmm_allocator_t KERNEL_CORE_BSS _pmm_allocator;



static inline u8 KERNEL_CORE_CODE _get_block_index(u64 address){
	u8 out=__builtin_ctzll(address)-PAGE_SIZE_SHIFT;
	return (out>=PMM_ALLOCATOR_SIZE_COUNT?PMM_ALLOCATOR_SIZE_COUNT-1:out);
}



static inline u8 KERNEL_CORE_CODE _get_largest_block_index(u64 address){
	u8 out=63-__builtin_clzll(address)-PAGE_SIZE_SHIFT;
	return (out>=PMM_ALLOCATOR_SIZE_COUNT?PMM_ALLOCATOR_SIZE_COUNT-1:out);
}



static inline u64 KERNEL_CORE_CODE _get_block_size(u8 index){
	return 1ull<<(PAGE_SIZE_SHIFT+index);
}



static void KERNEL_CORE_CODE _add_memory_range(u64 address,u64 end){
	if (address>=end){
		return;
	}
	INFO_CORE("Registering memory range %p - %p",address,end);
	do{
		u8 idx=_get_block_index(address);
		u64 size=_get_block_size(idx);
		u64 length=end-address;
		if (size>length){
			idx=_get_largest_block_index(length);
			size=_get_block_size(idx);
		}
		_pmm_allocator.counters.data[PMM_COUNTER_TOTAL]+=size>>PAGE_SIZE_SHIFT;
		pmm_allocator_page_header_t* header=(void*)address;
		header->prev=0;
		header->next=_pmm_allocator.blocks[idx];
		header->idx=idx;
		if (_pmm_allocator.blocks[idx]){
			((pmm_allocator_page_header_t*)(_pmm_allocator.blocks[idx]))->prev=address;
		}
		_pmm_allocator.blocks[idx]=address;
		address+=size;
	} while (address<end);
}



void KERNEL_CORE_CODE pmm_init(void){
	LOG_CORE("Initializing physical memory manager...");
	LOG_CORE("Registering low memory...");
	u64 last_memory_address=0;
	for (u16 i=0;i<KERNEL_DATA->mmap_size;i++){
		if ((KERNEL_DATA->mmap+i)->type!=1){
			continue;
		}
		u64 address=pmm_align_up_address((KERNEL_DATA->mmap+i)->base);
		if (!address){
			address+=PAGE_SIZE;
		}
		if (address<pmm_align_up_address(kernel_get_bss_end())){
			address=pmm_align_up_address(kernel_get_bss_end());
		}
		u64 end=pmm_align_down_address((KERNEL_DATA->mmap+i)->base+(KERNEL_DATA->mmap+i)->length);
		if (end>last_memory_address){
			last_memory_address=end;
		}
		if (end>=0x3fffffffull){
			end=0x3fffffffull;
		}
		_add_memory_range(address,end);
	}
	_pmm_allocator.last_memory_address=last_memory_address;
	LOG_CORE("Allocating allocator bitmap...");
	u64 bitmap_size=pmm_align_up_address((((last_memory_address>>PAGE_SIZE_SHIFT)+64)>>6)<<3); // 64 instead of 63 to add one more bit for the end of the last memory page
	INFO_CORE("Bitmap size: %v",bitmap_size);
	_pmm_allocator.bitmap=(void*)pmm_alloc_zero(bitmap_size>>PAGE_SIZE_SHIFT,PMM_COUNTER_PMM);
}



void KERNEL_CORE_CODE pmm_init_high_mem(void){
	LOG_CORE("Registering high memory...");
	for (u16 i=0;i<KERNEL_DATA->mmap_size;i++){
		if ((KERNEL_DATA->mmap+i)->type!=1){
			continue;
		}
		u64 address=pmm_align_up_address((KERNEL_DATA->mmap+i)->base);
		if (address<0x3fffffffull){
			address=0x3fffffffull;
		}
		u64 end=pmm_align_down_address((KERNEL_DATA->mmap+i)->base+(KERNEL_DATA->mmap+i)->length);
		_add_memory_range(address,end);
	}
}



u64 KERNEL_CORE_CODE pmm_alloc(u64 count,u8 counter){
	if (!count){
		ERROR_CORE("Trying to allocate zero physical pages!");
		for (;;);
		return 0;
	}
	u8 i=63-__builtin_clzll(count)+(!!(count&(count-1)));
	if (i>=PMM_ALLOCATOR_SIZE_COUNT){
		ERROR_CORE("Trying to allocate too many pages at once!");
		for (;;);
		return 0;
	}
	u8 j=i;
	while (!_pmm_allocator.blocks[i]&&i<PMM_ALLOCATOR_SIZE_COUNT){ // this loop can be refactored to a bitmap_size
		i++;
	}
	if (i==PMM_ALLOCATOR_SIZE_COUNT){
		ERROR_CORE("Out of memory!");
		return 0;
	}
	u64 out=_pmm_allocator.blocks[i];
	pmm_allocator_page_header_t* header=(void*)out;
	_pmm_allocator.blocks[i]=header->next;
	if (header->next){
		((pmm_allocator_page_header_t*)(header->next))->prev=0;
	}
	while (i>j){
		i--;
		u64 child_block=out+_get_block_size(i);
		header=(void*)child_block;
		header->prev=0;
		header->next=_pmm_allocator.blocks[i];
		header->idx=i;
		if (_pmm_allocator.blocks[i]){
			((pmm_allocator_page_header_t*)(_pmm_allocator.blocks[i]))->prev=child_block;
		}
		_pmm_allocator.blocks[i]=child_block;
	}
	if (_pmm_allocator.bitmap){
		u64 k=out>>PAGE_SIZE_SHIFT;
		_pmm_allocator.bitmap[k>>6]^=1ull<<(k&63);
		k+=_get_block_size(j)>>PAGE_SIZE_SHIFT;
		_pmm_allocator.bitmap[k>>6]^=1ull<<(k&63);
	}
	_pmm_allocator.counters.data[counter]+=_get_block_size(j)>>PAGE_SIZE_SHIFT;
	return out;
}



u64 KERNEL_CORE_CODE pmm_alloc_zero(u64 count,u8 counter){
	u64 out=pmm_alloc(count,counter);
	if (!out){
		return 0;
	}
	memset((void*)out,0,count<<PAGE_SIZE_SHIFT);
	return out;
}



void KERNEL_CORE_CODE pmm_dealloc(u64 address,u64 count,u8 counter){
	if (!count){
		ERROR_CORE("Trying to deallocate zero physical pages!");
		return;
	}
	u8 i=63-__builtin_clzll(count)+(!!(count&(count-1)));
	if (i>=PMM_ALLOCATOR_SIZE_COUNT){
		ERROR_CORE("Trying to deallocate too many pages at once!");
		for (;;);
		return;
	}
	_pmm_allocator.counters.data[counter]-=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	u64 j=address>>PAGE_SIZE_SHIFT;
	_pmm_allocator.bitmap[j>>6]^=1ull<<(j&63);
	j+=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	_pmm_allocator.bitmap[j>>6]^=1ull<<(j&63);
	while (i<PMM_ALLOCATOR_SIZE_COUNT){
		u64 buddy=address^_get_block_size(i);
		j=(address|_get_block_size(i))>>PAGE_SIZE_SHIFT;
		if (buddy>=_pmm_allocator.last_memory_address||(_pmm_allocator.bitmap[j>>6]&(1ull<<(j&63)))||((pmm_allocator_page_header_t*)buddy)->idx!=i){
			break;
		}
		address&=~_get_block_size(i);
		const pmm_allocator_page_header_t* header=(void*)buddy;
		if (header->prev){
			((pmm_allocator_page_header_t*)(header->prev))->next=header->next;
		}
		else{
			_pmm_allocator.blocks[i]=header->next;
		}
		if (header->next){
			((pmm_allocator_page_header_t*)(header->next))->prev=header->prev;
		}
		i++;
	}
	pmm_allocator_page_header_t* header=(void*)address;
	header->prev=0;
	header->next=_pmm_allocator.blocks[i];
	header->idx=i;
	if (_pmm_allocator.blocks[i]){
		((pmm_allocator_page_header_t*)(_pmm_allocator.blocks[i]))->prev=address;
	}
	_pmm_allocator.blocks[i]=address;
}



const pmm_counters_t* pmm_get_counters(void){
	return &(_pmm_allocator.counters);
}
