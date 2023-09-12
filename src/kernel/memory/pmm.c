#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "pmm"



#define PMM_LOW_ALLOCATOR_LIMIT 0x40000000ull



static pmm_allocator_t KERNEL_CORE_BSS _pmm_low_allocator;
static pmm_allocator_t KERNEL_CORE_BSS _pmm_high_allocator;
static lock_t KERNEL_CORE_DATA _pmm_counter_lock=LOCK_INIT_STRUCT;
static pmm_counters_t KERNEL_CORE_BSS _pmm_counters;



static inline u64 _get_bitmap_size(const pmm_allocator_t* allocator){
	 // 64 instead of 63 to add one more bit for the end of the last memory address
	return pmm_align_up_address(((((allocator->last_address-allocator->first_address)>>PAGE_SIZE_SHIFT)+64)>>6)*sizeof(u64));
}



static inline _Bool _get_address_bit(const pmm_allocator_t* allocator,u64 address){
	address=(address-allocator->first_address)>>PAGE_SIZE_SHIFT;
	return !!(allocator->bitmap[address>>6]&(1ull<<(address&63)));
}



static inline void _toggle_address_bit(const pmm_allocator_t* allocator,u64 address){
	address=(address-allocator->first_address)>>PAGE_SIZE_SHIFT;
	allocator->bitmap[address>>6]^=1ull<<(address&63);
}



static inline u64 KERNEL_CORE_CODE _get_block_size(u8 index){
	return 1ull<<(PAGE_SIZE_SHIFT+index);
}



static void KERNEL_CORE_CODE _add_memory_range(pmm_allocator_t* allocator,u64 address,u64 end){
	if (address>=end){
		return;
	}
	INFO_CORE("Registering memory range %p - %p",address,end);
	do{
		u8 idx=__builtin_ctzll(address)-PAGE_SIZE_SHIFT;
		if (idx>=PMM_ALLOCATOR_SIZE_COUNT){
			idx=PMM_ALLOCATOR_SIZE_COUNT-1;
		}
		u64 size=_get_block_size(idx);
		u64 length=end-address;
		if (size>length){
			idx=63-__builtin_clzll(address)-PAGE_SIZE_SHIFT;
			if (idx>=PMM_ALLOCATOR_SIZE_COUNT){
				idx=PMM_ALLOCATOR_SIZE_COUNT-1;
			}
			size=_get_block_size(idx);
		}
		_pmm_counters.data[PMM_COUNTER_TOTAL]+=size>>PAGE_SIZE_SHIFT;
		pmm_allocator_page_header_t* header=(void*)address;
		header->prev=0;
		header->next=allocator->blocks[idx];
		header->idx=idx;
		if (allocator->blocks[idx]){
			allocator->blocks[idx]->prev=header;
		}
		allocator->blocks[idx]=header;
		allocator->block_bitmap|=1<<idx;
		address+=size;
	} while (address<end);
}



void KERNEL_CORE_CODE pmm_init(void){
	LOG_CORE("Initializing physical memory manager...");
	LOG_CORE("Scanning memory...");
	_pmm_low_allocator.first_address=0;
	_pmm_low_allocator.last_address=0;
	lock_init(&(_pmm_low_allocator.lock));
	_pmm_high_allocator.first_address=PMM_LOW_ALLOCATOR_LIMIT;
	_pmm_high_allocator.last_address=PMM_LOW_ALLOCATOR_LIMIT;
	lock_init(&(_pmm_high_allocator.lock));
	for (u16 i=0;i<KERNEL_DATA->mmap_size;i++){
		u64 end=pmm_align_down_address((KERNEL_DATA->mmap+i)->base+(KERNEL_DATA->mmap+i)->length);
		if ((KERNEL_DATA->mmap+i)->type==1){
			if (end>_pmm_low_allocator.last_address){
				_pmm_low_allocator.last_address=(end>PMM_LOW_ALLOCATOR_LIMIT?PMM_LOW_ALLOCATOR_LIMIT:end);
			}
			if (end>_pmm_high_allocator.last_address){
				_pmm_high_allocator.last_address=end;
			}
		}
	};
	u64 low_bitmap_size=_get_bitmap_size(&_pmm_low_allocator);
	u64 high_bitmap_size=_get_bitmap_size(&_pmm_high_allocator);
	INFO_CORE("Low bitmap size: %v",low_bitmap_size);
	INFO_CORE("High bitmap size: %v",high_bitmap_size);
	_pmm_low_allocator.bitmap=(void*)pmm_align_up_address(kernel_get_bss_end());
	_pmm_high_allocator.bitmap=(void*)(pmm_align_up_address(kernel_get_bss_end())+low_bitmap_size);
	u64 kernel_end=pmm_align_up_address(kernel_get_bss_end())+low_bitmap_size+high_bitmap_size;
	LOG_CORE("Registering low memory...");
	for (u16 i=0;i<KERNEL_DATA->mmap_size;i++){
		if ((KERNEL_DATA->mmap+i)->type!=1){
			continue;
		}
		u64 address=pmm_align_up_address((KERNEL_DATA->mmap+i)->base);
		if (address<kernel_end){
			address=kernel_end;
		}
		u64 end=pmm_align_down_address((KERNEL_DATA->mmap+i)->base+(KERNEL_DATA->mmap+i)->length);
		_add_memory_range(&_pmm_low_allocator,address,(end>PMM_LOW_ALLOCATOR_LIMIT?PMM_LOW_ALLOCATOR_LIMIT:end));
	}
}



void KERNEL_CORE_CODE pmm_init_high_mem(void){
	LOG_CORE("Registering high memory...");
	for (u16 i=0;i<KERNEL_DATA->mmap_size;i++){
		if ((KERNEL_DATA->mmap+i)->type!=1){
			continue;
		}
		u64 address=pmm_align_up_address((KERNEL_DATA->mmap+i)->base);
		u64 end=pmm_align_down_address((KERNEL_DATA->mmap+i)->base+(KERNEL_DATA->mmap+i)->length);
		_add_memory_range(&_pmm_high_allocator,(address<PMM_LOW_ALLOCATOR_LIMIT?PMM_LOW_ALLOCATOR_LIMIT:address),end);
	}
}



u64 KERNEL_CORE_CODE pmm_alloc(u64 count,u8 counter){
	if (!count){
		panic("Trying to allocate zero physical pages",1);
		return 0;
	}
	u8 i=63-__builtin_clzll(count)+(!!(count&(count-1)));
	if (i>=PMM_ALLOCATOR_SIZE_COUNT){
		panic("Trying to allocate too many pages at once",1);
		return 0;
	}
	pmm_allocator_t* allocator=(!_pmm_high_allocator.block_bitmap||__builtin_ffs(_pmm_high_allocator.block_bitmap>>i)>__builtin_ffs(_pmm_low_allocator.block_bitmap>>i)?&_pmm_low_allocator:&_pmm_high_allocator);
	lock_acquire_exclusive(&(allocator->lock));
	if (!(allocator->block_bitmap>>i)){
		lock_release_exclusive(&(allocator->lock));
		panic("Out of memory",1);
		return 0;
	}
	u8 j=__builtin_ffs(allocator->block_bitmap>>i)+i-1;
	u64 out=(u64)(allocator->blocks[j]);
	pmm_allocator_page_header_t* header=(void*)out;
	allocator->blocks[j]=header->next;
	if (header->next){
		header->next->prev=0;
	}
	else{
		allocator->block_bitmap&=~(1<<j);
	}
	while (j>i){
		j--;
		u64 child_block=out+_get_block_size(j);
		header=(void*)child_block;
		header->prev=0;
		header->next=allocator->blocks[j];
		header->idx=j;
		allocator->blocks[j]=header;
		allocator->block_bitmap|=1<<j;
	}
	_toggle_address_bit(allocator,out);
	_toggle_address_bit(allocator,out+_get_block_size(i));
	lock_release_exclusive(&(allocator->lock));
	lock_acquire_exclusive(&_pmm_counter_lock);
	_pmm_counters.data[counter]+=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	lock_release_exclusive(&_pmm_counter_lock);
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
		panic("Trying to deallocate zero physical pages",1);
		return;
	}
	u8 i=63-__builtin_clzll(count)+(!!(count&(count-1)));
	if (i>=PMM_ALLOCATOR_SIZE_COUNT){
		panic("Trying to deallocate too many pages at once",1);
		return;
	}
	lock_acquire_exclusive(&_pmm_counter_lock);
	_pmm_counters.data[counter]-=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	lock_release_exclusive(&_pmm_counter_lock);
	pmm_allocator_t* allocator=(address<PMM_LOW_ALLOCATOR_LIMIT?&_pmm_low_allocator:&_pmm_high_allocator);
	lock_acquire_exclusive(&(allocator->lock));
	_toggle_address_bit(allocator,address);
	_toggle_address_bit(allocator,address+_get_block_size(i));
	while (i<PMM_ALLOCATOR_SIZE_COUNT){
		const pmm_allocator_page_header_t* buddy=(void*)(address^_get_block_size(i));
		if (((u64)buddy)>=allocator->last_address||_get_address_bit(allocator,address|_get_block_size(i))||buddy->idx!=i){
			break;
		}
		address&=~_get_block_size(i);
		if (buddy->prev){
			buddy->prev->next=buddy->next;
		}
		else{
			allocator->blocks[i]=buddy->next;
			if (!buddy->next){
				allocator->block_bitmap&=~(1<<i);
			}
		}
		if (buddy->next){
			buddy->next->prev=buddy->prev;
		}
		i++;
	}
	pmm_allocator_page_header_t* header=(void*)address;
	header->prev=0;
	header->next=allocator->blocks[i];
	header->idx=i;
	if (allocator->blocks[i]){
		allocator->blocks[i]->prev=header;
	}
	allocator->blocks[i]=header;
	allocator->block_bitmap|=1<<i;
	lock_release_exclusive(&(allocator->lock));
}



void pmm_get_counters(pmm_counters_t* out){
	lock_acquire_exclusive(&_pmm_counter_lock);
	*out=_pmm_counters;
	lock_release_exclusive(&_pmm_counter_lock);
	out->data[PMM_COUNTER_FREE]=out->data[PMM_COUNTER_TOTAL];
	for (u8 i=PMM_COUNTER_FREE+1;i<=PMM_COUNTER_MAX;i++){
		out->data[PMM_COUNTER_FREE]-=out->data[i];
	}
}
