#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "pmm"



static pmm_counter_descriptor_t _pmm_pmm_counter=PMM_COUNTER_INIT_STRUCT("pmm");
static pmm_counter_descriptor_t _pmm_kernel_image_pmm_counter=PMM_COUNTER_INIT_STRUCT("kernel_image");
static pmm_counter_descriptor_t _pmm_total_pmm_counter=PMM_COUNTER_INIT_STRUCT("total");



HANDLE_DECLARE_TYPE(PMM_COUNTER,{});



static pmm_allocator_t* KERNEL_INIT_WRITE _pmm_allocators;
static u32 KERNEL_INIT_WRITE _pmm_allocator_count;
static u64* KERNEL_INIT_WRITE _pmm_bitmap;
static pmm_load_balancer_t _pmm_load_balancer;
static _Bool KERNEL_INIT_WRITE _pmm_initialized=0;

pmm_load_balancer_stats_t* KERNEL_INIT_WRITE pmm_load_balancer_stats;



static KERNEL_INLINE u64 _get_bitmap_size(u64 max_address){
	return pmm_align_up_address((((max_address>>PAGE_SIZE_SHIFT)+64)>>6)*sizeof(u64));
	 // 64 instead of 63 to add one more bit for the end of the last memory address
}



static KERNEL_INLINE _Bool _get_address_bit(const pmm_allocator_t* allocator,u64 address){
	address>>=PAGE_SIZE_SHIFT;
	return !!(_pmm_bitmap[address>>6]&(1ull<<(address&63)));
}



static KERNEL_INLINE void _toggle_address_bit(const pmm_allocator_t* allocator,u64 address){
	address>>=PAGE_SIZE_SHIFT;
	_pmm_bitmap[address>>6]^=1ull<<(address&63);
}



static KERNEL_INLINE u64 _get_block_size(u8 index){
	return 1ull<<(PAGE_SIZE_SHIFT+index);
}



static KERNEL_INLINE pmm_allocator_t* _get_allocator_from_address(u64 address){
	return _pmm_allocators+(address>>PMM_ALLOCATOR_MAX_REGION_SIZE_SHIFT);
}



static void _add_memory_range(u64 address,u64 end){
	if (address>=end){
		return;
	}
	INFO("Registering memory range %p - %p",address,end);
	do{
		pmm_allocator_t* allocator=_get_allocator_from_address(address);
		u8 idx=__builtin_ctzll(address)-PAGE_SIZE_SHIFT;
		if (idx>=PMM_ALLOCATOR_BLOCK_GROUP_COUNT){
			idx=PMM_ALLOCATOR_BLOCK_GROUP_COUNT-1;
		}
		u64 size=_get_block_size(idx);
		u64 length=end-address;
		if (size>length){
			idx=63-__builtin_clzll(address)-PAGE_SIZE_SHIFT;
			if (idx>=PMM_ALLOCATOR_BLOCK_GROUP_COUNT){
				idx=PMM_ALLOCATOR_BLOCK_GROUP_COUNT-1;
			}
			size=_get_block_size(idx);
		}
		_pmm_total_pmm_counter.count+=size>>PAGE_SIZE_SHIFT;
		pmm_allocator_page_header_t* header=(void*)(address+VMM_HIGHER_HALF_ADDRESS_OFFSET);
		header->prev=(allocator->block_groups+idx)->tail;
		header->next=NULL;
		header->idx=idx;
		if (header->prev){
			header->prev->next=header;
		}
		else{
			(allocator->block_groups+idx)->head=header;
		}
		(allocator->block_groups+idx)->tail=header;
		allocator->block_group_bitmap|=1<<idx;
		address+=size;
	} while (address<end);
}



void pmm_init(void){
	LOG("Initializing physical memory manager...");
	LOG("Scanning memory...");
	u64 max_address=0;
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		u64 address=pmm_align_down_address((kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length);
		if (address>max_address){
			max_address=address;
		}
	}
	LOG("Allocating memory...");
	_pmm_allocator_count=(max_address+PMM_ALLOCATOR_MAX_REGION_SIZE-1)>>PMM_ALLOCATOR_MAX_REGION_SIZE_SHIFT;
	INFO("Allocator count: %u (%v)",_pmm_allocator_count,pmm_align_up_address(_pmm_allocator_count*sizeof(pmm_allocator_t)));
	_pmm_allocators=(void*)(pmm_align_up_address(kernel_data.first_free_address)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	kernel_data.first_free_address+=pmm_align_up_address(_pmm_allocator_count*sizeof(pmm_allocator_t));
	memset(_pmm_allocators,0,_pmm_allocator_count*sizeof(pmm_allocator_t));
	for (u32 i=0;i<_pmm_allocator_count;i++){
		spinlock_init(&((_pmm_allocators+i)->lock));
	}
	u64 bitmap_size=_get_bitmap_size(max_address);
	INFO("Bitmap size: %v",bitmap_size);
	_pmm_bitmap=(void*)(pmm_align_up_address(kernel_data.first_free_address)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	kernel_data.first_free_address+=bitmap_size;
	memset(_pmm_bitmap,0,bitmap_size);
	spinlock_init(&(_pmm_load_balancer.lock));
	_pmm_load_balancer.index=0;
	_pmm_load_balancer.stats.hit_count=0;
	_pmm_load_balancer.stats.miss_count=0;
	_pmm_load_balancer.stats.miss_locked_count=0;
	pmm_load_balancer_stats=&(_pmm_load_balancer.stats);
	LOG("Registering counters...");
	handle_new(&_pmm_pmm_counter,HANDLE_TYPE_PMM_COUNTER,&(_pmm_pmm_counter.handle));
	handle_new(&_pmm_kernel_image_pmm_counter,HANDLE_TYPE_PMM_COUNTER,&(_pmm_kernel_image_pmm_counter.handle));
	handle_new(&_pmm_total_pmm_counter,HANDLE_TYPE_PMM_COUNTER,&(_pmm_total_pmm_counter.handle));
	handle_finish_setup(&(_pmm_pmm_counter.handle));
	handle_finish_setup(&(_pmm_kernel_image_pmm_counter.handle));
	handle_finish_setup(&(_pmm_total_pmm_counter.handle));
	_pmm_pmm_counter.count+=pmm_align_up_address(_pmm_allocator_count*sizeof(pmm_allocator_t)+bitmap_size)>>PAGE_SIZE_SHIFT;
	_pmm_kernel_image_pmm_counter.count+=pmm_align_up_address(kernel_section_kernel_end()-kernel_section_kernel_start())>>PAGE_SIZE_SHIFT;
	LOG("Registering low memory...");
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		u64 address=pmm_align_up_address((kernel_data.mmap+i)->base);
		if (address<kernel_data.first_free_address){
			address=kernel_data.first_free_address;
		}
		u64 end=pmm_align_down_address((kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length);
		_add_memory_range(address,(end>PMM_LOW_ALLOCATOR_LIMIT?PMM_LOW_ALLOCATOR_LIMIT:end));
	}
	_pmm_initialized=1;
}



void pmm_init_high_mem(void){
	LOG("Registering high memory...");
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		u64 address=pmm_align_up_address((kernel_data.mmap+i)->base);
		u64 end=pmm_align_down_address((kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length);
		_add_memory_range((address<PMM_LOW_ALLOCATOR_LIMIT?PMM_LOW_ALLOCATOR_LIMIT:address),end);
	}
}



u64 pmm_alloc(u64 count,pmm_counter_descriptor_t* counter,_Bool memory_hint){
	if (!_pmm_initialized){
		return 0;
	}
	if (!count){
		panic("pmm_alloc: trying to allocate zero physical pages");
	}
	u8 i=63-__builtin_clzll(count)+(!!(count&(count-1)));
	if (i>=PMM_ALLOCATOR_BLOCK_GROUP_COUNT){
		panic("pmm_alloc: trying to allocate too many pages at once");
	}
	scheduler_pause();
	spinlock_acquire_exclusive(&(_pmm_load_balancer.lock));
	u32 index;
	_pmm_load_balancer.stats.miss_count--;
	do{
		_pmm_load_balancer.stats.miss_count++;
		index=_pmm_load_balancer.index;
		_pmm_load_balancer.index++;
		if (_pmm_load_balancer.index>=_pmm_allocator_count){
			_pmm_load_balancer.index-=_pmm_allocator_count;
		}
	} while (!((_pmm_allocators+index)->block_group_bitmap>>i));
	spinlock_release_exclusive(&(_pmm_load_balancer.lock));
	if (memory_hint==PMM_MEMORY_HINT_LOW_MEMORY){
		index&=PMM_LOW_ALLOCATOR_LIMIT/PMM_ALLOCATOR_MAX_REGION_SIZE-1;
	}
	u32 base_index=index;
_retry_allocator:
	pmm_allocator_t* allocator=_pmm_allocators+index;
	spinlock_acquire_exclusive(&(allocator->lock));
	if (!(allocator->block_group_bitmap>>i)){
		spinlock_release_exclusive(&(allocator->lock));
		index++;
		_pmm_load_balancer.stats.miss_locked_count++;
		if (index==_pmm_allocator_count){
			index=0;
		}
		if (index!=base_index){
			goto _retry_allocator;
		}
		panic("pmm_alloc: out of memory");
	}
	_pmm_load_balancer.stats.hit_count++;
	u8 j=__builtin_ffs(allocator->block_group_bitmap>>i)+i-1;
	pmm_allocator_page_header_t* header=(allocator->block_groups+j)->head;
	(allocator->block_groups+j)->head=header->next;
	if (header->next){
		header->next->prev=0;
	}
	else{
		(allocator->block_groups+j)->tail=NULL;
		allocator->block_group_bitmap&=~(1<<j);
	}
	u64 out=((u64)header)-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	while (j>i){
		j--;
		header=(void*)(out+_get_block_size(j)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
		header->prev=0;
		header->next=0;
		header->idx=j;
		(allocator->block_groups+j)->head=header;
		(allocator->block_groups+j)->tail=header;
		allocator->block_group_bitmap|=1<<j;
	}
	_toggle_address_bit(allocator,out);
	_toggle_address_bit(allocator,out+_get_block_size(i));
	spinlock_release_exclusive(&(allocator->lock));
	scheduler_resume();
	if (!counter->handle.rb_node.key){
		handle_new(counter,HANDLE_TYPE_PMM_COUNTER,&(counter->handle));
		handle_finish_setup(&(counter->handle));
	}
	counter->count+=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	for (u64* ptr=(u64*)(out+VMM_HIGHER_HALF_ADDRESS_OFFSET);ptr<(u64*)(out+_get_block_size(i)+VMM_HIGHER_HALF_ADDRESS_OFFSET);ptr++){
		*ptr=0;
	}
	return out;
}



void pmm_dealloc(u64 address,u64 count,pmm_counter_descriptor_t* counter){
	scheduler_pause();
	if (!count){
		panic("pmm_dealloc: trying to deallocate zero physical pages");
	}
	u8 i=63-__builtin_clzll(count)+(!!(count&(count-1)));
	if (i>=PMM_ALLOCATOR_BLOCK_GROUP_COUNT){
		panic("pmm_dealloc: trying to deallocate too many pages at once");
	}
	counter->count-=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	pmm_allocator_t* allocator=_get_allocator_from_address(address);
	spinlock_acquire_exclusive(&(allocator->lock));
	_toggle_address_bit(allocator,address);
	_toggle_address_bit(allocator,address+_get_block_size(i));
	u64 first_address=address&(-PMM_ALLOCATOR_MAX_REGION_SIZE);
	u64 last_address=address+PMM_ALLOCATOR_MAX_REGION_SIZE;
	while (i<PMM_ALLOCATOR_BLOCK_GROUP_COUNT){
		pmm_allocator_page_header_t* buddy=(void*)((address^_get_block_size(i))+VMM_HIGHER_HALF_ADDRESS_OFFSET);
		if (((u64)buddy)<first_address||((u64)buddy)>=last_address||_get_address_bit(allocator,address|_get_block_size(i))||buddy->idx!=i){
			break;
		}
		address&=~_get_block_size(i);
		if (buddy->prev){
			buddy->prev->next=buddy->next;
		}
		else{
			(allocator->block_groups+i)->head=buddy->next;
			if (!buddy->next){
				allocator->block_group_bitmap&=~(1<<i);
			}
		}
		if (buddy->next){
			buddy->next->prev=buddy->prev;
		}
		else{
			(allocator->block_groups+i)->tail=buddy->prev;
		}
		i++;
	}
	pmm_allocator_page_header_t* header=(void*)(address+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	header->prev=(allocator->block_groups+i)->tail;
	header->next=0;
	header->idx=i;
	if (header->prev){
		header->prev->next=header;
	}
	else{
		(allocator->block_groups+i)->head=header;
	}
	(allocator->block_groups+i)->tail=header;
	allocator->block_group_bitmap|=1<<i;
	spinlock_release_exclusive(&(allocator->lock));
	scheduler_resume();
}
