#include <kernel/kernel.h>
#include <kernel/lock/lock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "pmm"



PMM_DECLARE_COUNTER(KERNEL_IMAGE);
PMM_DECLARE_COUNTER(PMM);
PMM_DECLARE_COUNTER(TOTAL);



static pmm_allocator_t KERNEL_CORE_BSS _pmm_low_allocator;
static pmm_allocator_t KERNEL_CORE_BSS _pmm_high_allocator;
static lock_t KERNEL_CORE_DATA _pmm_counter_lock=LOCK_INIT_STRUCT;
static pmm_counters_t* KERNEL_CORE_BSS _pmm_counters;
static u64 KERNEL_CORE_DATA _pmm_block_address_offset=0;

u64 KERNEL_CORE_BSS pmm_adjusted_kernel_end;



static inline pmm_allocator_page_header_t* _get_block_header(u64 address){
	return (pmm_allocator_page_header_t*)(address+_pmm_block_address_offset);
}



static inline u64 _get_bitmap_size(const pmm_allocator_t* allocator){
	return pmm_align_up_address(((((allocator->last_address-allocator->first_address)>>PAGE_SIZE_SHIFT)+64)>>6)*sizeof(u64));
	 // 64 instead of 63 to add one more bit for the end of the last memory address
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
		(_pmm_counters->data+PMM_COUNTER_TOTAL)->count+=size>>PAGE_SIZE_SHIFT;
		pmm_allocator_page_header_t* header=_get_block_header(address);
		header->prev=0;
		header->next=allocator->blocks[idx];
		header->idx=idx;
		if (allocator->blocks[idx]){
			_get_block_header(allocator->blocks[idx])->prev=address;
		}
		allocator->blocks[idx]=address;
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
	}
	u64 low_bitmap_size=_get_bitmap_size(&_pmm_low_allocator);
	INFO_CORE("Low bitmap size: %v",low_bitmap_size);
	_pmm_low_allocator.bitmap=(void*)pmm_align_up_address(kernel_get_bss_end());
	memset(_pmm_low_allocator.bitmap,0,low_bitmap_size);
	u64 high_bitmap_size=_get_bitmap_size(&_pmm_high_allocator);
	INFO_CORE("High bitmap size: %v",high_bitmap_size);
	_pmm_high_allocator.bitmap=(void*)(pmm_align_up_address(kernel_get_bss_end())+low_bitmap_size);
	memset(_pmm_high_allocator.bitmap,0,high_bitmap_size);
	LOG_CORE("Registering counters...");
	_pmm_counters=(void*)(pmm_align_up_address(kernel_get_bss_end())+low_bitmap_size+high_bitmap_size);
	_pmm_counters->length=0;
	for (const pmm_counter_descriptor_t*const* descriptor=(void*)(kernel_get_pmm_counter_start()+kernel_get_offset());(u64)descriptor<(kernel_get_pmm_counter_end()+kernel_get_offset());descriptor++){
		if (*descriptor){
			*((*descriptor)->var)=_pmm_counters->length;
			memcpy((_pmm_counters->data+_pmm_counters->length)->name,(*descriptor)->name,PMM_COUNTER_NAME_LENGTH);
			(_pmm_counters->data+_pmm_counters->length)->count=0;
			_pmm_counters->length++;
		}
	}
	u32 pmm_counters_size=pmm_align_up_address(sizeof(pmm_counters_t)+_pmm_counters->length*sizeof(pmm_counter_t));
	INFO_CORE("Counter array size: %v",pmm_counters_size);
	(_pmm_counters->data+PMM_COUNTER_PMM)->count=pmm_align_up_address(low_bitmap_size+high_bitmap_size+pmm_counters_size)>>PAGE_SIZE_SHIFT;
	(_pmm_counters->data+PMM_COUNTER_KERNEL_IMAGE)->count=pmm_align_up_address(kernel_get_bss_end()-kernel_get_start())>>PAGE_SIZE_SHIFT;
	pmm_adjusted_kernel_end=pmm_align_up_address(kernel_get_bss_end())+low_bitmap_size+high_bitmap_size+pmm_counters_size;
	LOG_CORE("Registering low memory...");
	for (u16 i=0;i<KERNEL_DATA->mmap_size;i++){
		if ((KERNEL_DATA->mmap+i)->type!=1){
			continue;
		}
		u64 address=pmm_align_up_address((KERNEL_DATA->mmap+i)->base);
		if (address<pmm_adjusted_kernel_end){
			address=pmm_adjusted_kernel_end;
		}
		u64 end=pmm_align_down_address((KERNEL_DATA->mmap+i)->base+(KERNEL_DATA->mmap+i)->length);
		_add_memory_range(&_pmm_low_allocator,address,(end>PMM_LOW_ALLOCATOR_LIMIT?PMM_LOW_ALLOCATOR_LIMIT:end));
	}
}



void KERNEL_CORE_CODE pmm_init_high_mem(void){
	_pmm_block_address_offset=VMM_HIGHER_HALF_ADDRESS_OFFSET;
	_pmm_low_allocator.bitmap=(void*)(((u64)(_pmm_low_allocator.bitmap))+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	_pmm_high_allocator.bitmap=(void*)(((u64)(_pmm_high_allocator.bitmap))+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	_pmm_counters=(void*)(((u64)_pmm_counters)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
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



u64 KERNEL_CORE_CODE pmm_alloc(u64 count,u8 counter,_Bool memory_hint){
	scheduler_pause();
	if (!count){
		panic("pmm_alloc: trying to allocate zero physical pages");
	}
	if (counter>=_pmm_counters->length){
		panic("pmm_alloc: invalid counter");
	}
	u8 i=63-__builtin_clzll(count)+(!!(count&(count-1)));
	if (i>=PMM_ALLOCATOR_SIZE_COUNT){
		panic("pmm_alloc: trying to allocate too many pages at once");
	}
	pmm_allocator_t* allocator=(memory_hint==PMM_MEMORY_HINT_LOW_MEMORY||!_pmm_high_allocator.block_bitmap||__builtin_ffs(_pmm_high_allocator.block_bitmap>>i)>__builtin_ffs(_pmm_low_allocator.block_bitmap>>i)?&_pmm_low_allocator:&_pmm_high_allocator);
	lock_acquire_exclusive(&(allocator->lock));
	if (!(allocator->block_bitmap>>i)){
		lock_release_exclusive(&(allocator->lock));
		panic("pmm_alloc: out of memory");
	}
	u8 j=__builtin_ffs(allocator->block_bitmap>>i)+i-1;
	u64 out=(u64)(allocator->blocks[j]);
	pmm_allocator_page_header_t* header=_get_block_header(out);
	allocator->blocks[j]=header->next;
	if (header->next){
		_get_block_header(header->next)->prev=0;
	}
	else{
		allocator->block_bitmap&=~(1<<j);
	}
	while (j>i){
		j--;
		u64 child_block=out+_get_block_size(j);
		header=_get_block_header(child_block);
		header->prev=0;
		header->next=allocator->blocks[j];
		header->idx=j;
		allocator->blocks[j]=child_block;
		allocator->block_bitmap|=1<<j;
	}
	_toggle_address_bit(allocator,out);
	_toggle_address_bit(allocator,out+_get_block_size(i));
	lock_release_exclusive(&(allocator->lock));
	lock_acquire_exclusive(&_pmm_counter_lock);
	(_pmm_counters->data+counter)->count+=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	lock_release_exclusive(&_pmm_counter_lock);
	scheduler_resume();
	return out;
}



u64 KERNEL_CORE_CODE pmm_alloc_zero(u64 count,u8 counter,_Bool memory_hint){
	u64 out=pmm_alloc(count,counter,memory_hint);
	if (!out){
		return 0;
	}
	memset((void*)(out+_pmm_block_address_offset),0,count<<PAGE_SIZE_SHIFT);
	return out;
}



void KERNEL_CORE_CODE pmm_dealloc(u64 address,u64 count,u8 counter){
	scheduler_pause();
	if (!count){
		panic("pmm_dealloc: trying to deallocate zero physical pages");
	}
	if (counter>=_pmm_counters->length){
		panic("pmm_dealloc: invalid counter");
	}
	u8 i=63-__builtin_clzll(count)+(!!(count&(count-1)));
	if (i>=PMM_ALLOCATOR_SIZE_COUNT){
		panic("pmm_dealloc: trying to deallocate too many pages at once");
	}
	lock_acquire_exclusive(&_pmm_counter_lock);
	(_pmm_counters->data+counter)->count-=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	lock_release_exclusive(&_pmm_counter_lock);
	pmm_allocator_t* allocator=(address<PMM_LOW_ALLOCATOR_LIMIT?&_pmm_low_allocator:&_pmm_high_allocator);
	lock_acquire_exclusive(&(allocator->lock));
	_toggle_address_bit(allocator,address);
	_toggle_address_bit(allocator,address+_get_block_size(i));
	while (i<PMM_ALLOCATOR_SIZE_COUNT){
		const pmm_allocator_page_header_t* buddy=_get_block_header(address^_get_block_size(i));
		if (((u64)buddy)>=allocator->last_address||_get_address_bit(allocator,address|_get_block_size(i))||buddy->idx!=i){
			break;
		}
		address&=~_get_block_size(i);
		if (buddy->prev){
			_get_block_header(buddy->prev)->next=buddy->next;
		}
		else{
			allocator->blocks[i]=buddy->next;
			if (!buddy->next){
				allocator->block_bitmap&=~(1<<i);
			}
		}
		if (buddy->next){
			_get_block_header(buddy->next)->prev=buddy->prev;
		}
		i++;
	}
	pmm_allocator_page_header_t* header=_get_block_header(address);
	header->prev=0;
	header->next=allocator->blocks[i];
	header->idx=i;
	if (allocator->blocks[i]){
		_get_block_header(allocator->blocks[i])->prev=address;
	}
	allocator->blocks[i]=address;
	allocator->block_bitmap|=1<<i;
	lock_release_exclusive(&(allocator->lock));
	scheduler_resume();
}



u8 pmm_get_counter_count(void){
	return _pmm_counters->length;
}



_Bool pmm_get_counter(u8 counter,pmm_counter_t* out){
	if (counter>=_pmm_counters->length){
		return 0;
	}
	memcpy(out->name,(_pmm_counters->data+counter)->name,PMM_COUNTER_NAME_LENGTH);
	out->count=(_pmm_counters->data+counter)->count;
	return 1;
}
