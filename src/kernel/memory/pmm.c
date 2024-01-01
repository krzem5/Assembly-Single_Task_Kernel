#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/process.h>
#include <kernel/mp/thread.h>
#include <kernel/scheduler/load_balancer.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "pmm"



#define PMM_FLAG_ALLOCATOR_INITIALIZED 1
#define PMM_FLAG_HANDLE_INITIALIZED 2



static u32 KERNEL_INIT_WRITE _pmm_initialization_flags=0;
static u32 KERNEL_INIT_WRITE _pmm_allocator_count;
static pmm_allocator_t* KERNEL_INIT_WRITE _pmm_allocators;
static u64* KERNEL_INIT_WRITE _pmm_bitmap;
static pmm_block_descriptor_t* KERNEL_INIT_WRITE _pmm_block_descriptors;
static pmm_load_balancer_t _pmm_load_balancer;
static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _pmm_counter_omm_pmm_counter=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _pmm_counter_allocator=NULL;

KERNEL_PUBLIC handle_type_t pmm_counter_handle_type=0;
KERNEL_PUBLIC const pmm_load_balancer_stats_t* KERNEL_INIT_WRITE pmm_load_balancer_stats;



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



static KERNEL_INLINE u64 _get_block_size(u32 index){
	return 1ull<<(PAGE_SIZE_SHIFT+index);
}



static KERNEL_INLINE pmm_allocator_t* _get_allocator_from_address(u64 address){
	return _pmm_allocators+(address>>PMM_ALLOCATOR_MAX_REGION_SIZE_SHIFT);
}



static KERNEL_INLINE pmm_block_descriptor_t* _get_block_descriptor(u64 address){
	return _pmm_block_descriptors+(address>>(PAGE_SIZE_SHIFT/*+1*/));
}



static KERNEL_INLINE void _block_descriptor_init(u64 address,u64 prev,u64 next,u32 idx){
	pmm_block_descriptor_t* descriptor=_get_block_descriptor(address);
	descriptor->data[0]=prev|idx;
	descriptor->data[1]=next|((address>>PAGE_SIZE_SHIFT)&1);
}



static KERNEL_INLINE u64 _block_descriptor_get_idx(u64 address){
	return _get_block_descriptor(address)->data[0]&0xff;
}



static KERNEL_INLINE u64 _block_descriptor_get_prev(u64 address){
	return _get_block_descriptor(address)->data[0]&0xffffffffffffff00ull;
}



static KERNEL_INLINE u64 _block_descriptor_get_next(u64 address){
	return _get_block_descriptor(address)->data[1]&0xfffffffffffffffeull;
}



static KERNEL_INLINE void _block_descriptor_set_prev(u64 address,u64 prev){
	pmm_block_descriptor_t* descriptor=_get_block_descriptor(address);
	descriptor->data[0]=(descriptor->data[0]&0xff)|prev;
}



static KERNEL_INLINE void _block_descriptor_set_next(u64 address,u64 next){
	pmm_block_descriptor_t* descriptor=_get_block_descriptor(address);
	descriptor->data[1]=(descriptor->data[1]&1)|next;
}



static void KERNEL_EARLY_EXEC _add_memory_range(u64 address,u64 end){
	if (address>=end){
		return;
	}
	INFO("Registering memory range %p - %p",address,end);
	do{
		pmm_allocator_t* allocator=_get_allocator_from_address(address);
		u32 idx=__builtin_ctzll(address)-PAGE_SIZE_SHIFT;
		if (idx>=PMM_ALLOCATOR_BLOCK_GROUP_COUNT){
			idx=PMM_ALLOCATOR_BLOCK_GROUP_COUNT-1;
		}
		u64 size=_get_block_size(idx);
		u64 length=end-address;
		if (size>length){
			idx=63-__builtin_clzll(length)-PAGE_SIZE_SHIFT;
			if (idx>=PMM_ALLOCATOR_BLOCK_GROUP_COUNT){
				idx=PMM_ALLOCATOR_BLOCK_GROUP_COUNT-1;
			}
			size=_get_block_size(idx);
		}
		_block_descriptor_init(address,(allocator->block_groups+idx)->tail,0,idx);
		if ((allocator->block_groups+idx)->tail){
			_block_descriptor_set_next((allocator->block_groups+idx)->tail,address);
		}
		else{
			(allocator->block_groups+idx)->head=address;
		}
		(allocator->block_groups+idx)->tail=address;
		allocator->block_group_bitmap|=1<<idx;
		address+=size;
	} while (address<end);
}



void KERNEL_EARLY_EXEC pmm_init(void){
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
	u64 block_descriptor_array_size=pmm_align_up_address(((max_address+PAGE_SIZE/**2-1*/)>>(PAGE_SIZE_SHIFT/*+1*/))*sizeof(pmm_block_descriptor_t));
	INFO("Block descriptor array size: %v",block_descriptor_array_size);
	_pmm_block_descriptors=(void*)(pmm_align_up_address(kernel_data.first_free_address)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	kernel_data.first_free_address+=block_descriptor_array_size;
	memset(_pmm_block_descriptors,0,block_descriptor_array_size);
	spinlock_init(&(_pmm_load_balancer.lock));
	_pmm_load_balancer.index=0;
	_pmm_load_balancer.stats.hit_count=0;
	_pmm_load_balancer.stats.miss_count=0;
	_pmm_load_balancer.stats.miss_locked_count=0;
	pmm_load_balancer_stats=&(_pmm_load_balancer.stats);
	LOG("Registering low memory...");
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		u64 address=pmm_align_up_address((kernel_data.mmap+i)->base);
		if (address<kernel_data.first_free_address){
			address=kernel_data.first_free_address;
		}
		u64 end=pmm_align_down_address((kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length);
		_add_memory_range(address,(end>PMM_LOW_ALLOCATOR_LIMIT?PMM_LOW_ALLOCATOR_LIMIT:end));
	}
	_pmm_initialization_flags|=PMM_FLAG_ALLOCATOR_INITIALIZED;
}



void KERNEL_EARLY_EXEC pmm_init_high_mem(void){
	LOG("Registering high memory...");
	u64 total_memory=0;
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		u64 address=pmm_align_up_address((kernel_data.mmap+i)->base);
		u64 end=pmm_align_down_address((kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length);
		total_memory+=end-address;
		_add_memory_range((address<PMM_LOW_ALLOCATOR_LIMIT?PMM_LOW_ALLOCATOR_LIMIT:address),end);
	}
	INFO("Registering counters...");
	pmm_counter_handle_type=handle_alloc("pmm_counter",NULL);
	pmm_counter_descriptor_t tmp={
		.count=0
	};
	_pmm_counter_allocator=omm_init("pmm_counter",sizeof(pmm_counter_descriptor_t),8,1,&tmp);
	spinlock_init(&(_pmm_counter_allocator->lock));
	_pmm_counter_omm_pmm_counter=omm_alloc(_pmm_counter_allocator);
	_pmm_counter_omm_pmm_counter->name="pmm_counter";
	_pmm_counter_omm_pmm_counter->count=tmp.count;
	handle_new(_pmm_counter_omm_pmm_counter,pmm_counter_handle_type,&(_pmm_counter_omm_pmm_counter->handle));
	handle_finish_setup(&(_pmm_counter_omm_pmm_counter->handle));
	_pmm_counter_allocator->pmm_counter=_pmm_counter_omm_pmm_counter;
	pmm_alloc_counter("pmm")->count=pmm_align_up_address(kernel_data.first_free_address-kernel_section_kernel_end())>>PAGE_SIZE_SHIFT;
	pmm_alloc_counter("kernel_image")->count=pmm_align_up_address(kernel_section_kernel_end()-kernel_section_kernel_start())>>PAGE_SIZE_SHIFT;
	pmm_alloc_counter("total")->count=total_memory;
	_pmm_initialization_flags|=PMM_FLAG_HANDLE_INITIALIZED;
}



KERNEL_PUBLIC pmm_counter_descriptor_t* pmm_alloc_counter(const char* name){
	pmm_counter_descriptor_t* out=omm_alloc(_pmm_counter_allocator);
	out->name=name;
	out->count=0;
	handle_new(out,pmm_counter_handle_type,&(out->handle));
	handle_finish_setup(&(out->handle));
	return out;
}



KERNEL_PUBLIC u64 pmm_alloc(u64 count,pmm_counter_descriptor_t* counter,_Bool memory_hint){
	if (!(_pmm_initialization_flags&PMM_FLAG_ALLOCATOR_INITIALIZED)){
		return 0;
	}
	if (!count){
		panic("pmm_alloc: trying to allocate zero physical pages");
	}
	u32 i=63-__builtin_clzll(count)+(!!(count&(count-1)));
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
	u32 j=__builtin_ffs(allocator->block_group_bitmap>>i)+i-1;
	u64 out=(allocator->block_groups+j)->head;
	(allocator->block_groups+j)->head=_block_descriptor_get_next(out);
	if (!(allocator->block_groups+j)->head){
		(allocator->block_groups+j)->tail=0;
		allocator->block_group_bitmap&=~(1<<j);
	}
	while (j>i){
		j--;
		u64 split_block=out+_get_block_size(j);
		_block_descriptor_init(split_block,0,0,j);
		(allocator->block_groups+j)->head=split_block;
		(allocator->block_groups+j)->tail=split_block;
		allocator->block_group_bitmap|=1<<j;
	}
	spinlock_release_exclusive(&(allocator->lock));
	_toggle_address_bit(allocator,out);
	_toggle_address_bit(allocator,out+_get_block_size(i));
	scheduler_resume();
	if ((_pmm_initialization_flags&PMM_FLAG_HANDLE_INITIALIZED)&&!counter->handle.rb_node.key){
		handle_new(counter,pmm_counter_handle_type,&(counter->handle));
		handle_finish_setup(&(counter->handle));
	}
	counter->count+=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	for (u64* ptr=(u64*)(out+VMM_HIGHER_HALF_ADDRESS_OFFSET);ptr<(u64*)(out+_get_block_size(i)+VMM_HIGHER_HALF_ADDRESS_OFFSET);ptr++){
		*ptr=0;
	}
	return out;
}



KERNEL_PUBLIC void pmm_dealloc(u64 address,u64 count,pmm_counter_descriptor_t* counter){
	scheduler_pause();
	if (!count){
		panic("pmm_dealloc: trying to deallocate zero physical pages");
	}
	u32 i=63-__builtin_clzll(count)+(!!(count&(count-1)));
	if (i>=PMM_ALLOCATOR_BLOCK_GROUP_COUNT){
		panic("pmm_dealloc: trying to deallocate too many pages at once");
	}
	counter->count-=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	pmm_allocator_t* allocator=_get_allocator_from_address(address);
	_toggle_address_bit(allocator,address);
	_toggle_address_bit(allocator,address+_get_block_size(i));
	spinlock_acquire_exclusive(&(allocator->lock));
	u64 first_address=address&(-PMM_ALLOCATOR_MAX_REGION_SIZE);
	u64 last_address=first_address+PMM_ALLOCATOR_MAX_REGION_SIZE;
	while (i<PMM_ALLOCATOR_BLOCK_GROUP_COUNT){
		u64 buddy=address^_get_block_size(i);
		if (buddy<first_address||buddy>=last_address||_get_address_bit(allocator,address|_get_block_size(i))||_block_descriptor_get_idx(buddy)!=i){
			break;
		}
		address&=~_get_block_size(i);
		u64 buddy_prev=_block_descriptor_get_prev(buddy);
		u64 buddy_next=_block_descriptor_get_next(buddy);
		if (buddy_prev){
			_block_descriptor_set_next(buddy_prev,buddy_next);
		}
		else{
			(allocator->block_groups+i)->head=buddy_next;
			if (!buddy_next){
				allocator->block_group_bitmap&=~(1<<i);
			}
		}
		if (buddy_next){
			_block_descriptor_set_prev(buddy_next,buddy_prev);
		}
		else{
			(allocator->block_groups+i)->tail=buddy_prev;
		}
		i++;
	}
	_block_descriptor_init(address,(allocator->block_groups+i)->tail,0,i);
	if ((allocator->block_groups+i)->tail){
		_block_descriptor_set_next((allocator->block_groups+i)->tail,address);
	}
	else{
		(allocator->block_groups+i)->head=address;
	}
	(allocator->block_groups+i)->tail=address;
	allocator->block_group_bitmap|=1<<i;
	spinlock_release_exclusive(&(allocator->lock));
	scheduler_resume();
}



#ifndef KERNEL_DISABLE_ASSERT
// Usage: /*DEBUG*/extern void __debug_pmm_verify_all(void);__debug_pmm_verify_all();/*DEBUG*/
KERNEL_PUBLIC void __debug_pmm_verify_all(void){
	for (u32 i=0;i<_pmm_allocator_count;i++){
		pmm_allocator_t* allocator=_pmm_allocators+i;
		spinlock_acquire_exclusive(&(allocator->lock));
		for (u32 j=0;j<PMM_ALLOCATOR_BLOCK_GROUP_COUNT;j++){
			if (!(allocator->block_group_bitmap&(1<<j))){
				continue;
			}
			// pmm_allocator_page_header_t* tail=(allocator->block_groups+j)->tail;
			// for (pmm_allocator_page_header_t* block=(allocator->block_groups+j)->head;block!=tail;block=block->next){
			// 	if (((u64)block)&((1<<(j+12))-1)){
			// 		ERROR("Wrong block @ %p",block);
			// 		*((char*)0)=0;
			// 	}
			// 	if (!block->next&&block!=tail){
			// 		ERROR("Broken chain @ %p",block);
			// 		*((char*)0)=0;
			// 	}
			// 	else if (!(((u64)(block->next))>>63)){
			// 		ERROR("Memory overwrite @ %p",block);
			// 		*((char*)0)=0;
			// 	}
			// }
		}
		spinlock_release_exclusive(&(allocator->lock));
	}
}
#endif
