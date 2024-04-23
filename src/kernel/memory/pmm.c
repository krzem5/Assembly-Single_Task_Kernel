#include <kernel/format/format.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/bitlock.h>
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
#include <kernel/util/memory.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "pmm"



#define PMM_DEBUG_VALUE 0xde

#define PMM_FLAG_PARTIALLY_INITIALIZED 1
#define PMM_FLAG_FULLY_INITIALIZED 2



static u32 KERNEL_INIT_WRITE _pmm_initialization_flags=0;
static u32 KERNEL_INIT_WRITE _pmm_allocator_count;
static pmm_allocator_t* KERNEL_INIT_WRITE _pmm_allocators;
static pmm_block_descriptor_t* KERNEL_INIT_WRITE _pmm_block_descriptors;
static pmm_load_balancer_t _pmm_load_balancer;
static omm_allocator_t* KERNEL_INIT_WRITE _pmm_counter_allocator=NULL;
static u64 KERNEL_EARLY_WRITE _pmm_self_counter_value;

KERNEL_PUBLIC handle_type_t pmm_counter_handle_type=0;
KERNEL_PUBLIC const pmm_load_balancer_stats_t* KERNEL_INIT_WRITE pmm_load_balancer_stats;
KERNEL_EARLY_POINTER(pmm_load_balancer_stats);



_Static_assert(PMM_ALLOCATOR_BUCKET_COUNT_MASK_SHIFT*2+2/*two flags/locks*/<=PAGE_SIZE_SHIFT);



static KERNEL_INLINE u64 _get_block_size(u32 index){
	return 1ull<<(PAGE_SIZE_SHIFT+index);
}



static KERNEL_INLINE pmm_allocator_t* _get_allocator_from_address(u64 address){
	return _pmm_allocators+(address>>PMM_ALLOCATOR_MAX_REGION_SIZE_SHIFT);
}



static KERNEL_INLINE pmm_block_descriptor_t* _get_block_descriptor(u64 address){
	return _pmm_block_descriptors+(address>>PAGE_SIZE_SHIFT);
}



static void _block_descriptor_deinit(u64 address){
	_get_block_descriptor(address)->data|=PMM_ALLOCATOR_BLOCK_DESCRIPTOR_INDEX_USED;
}



static KERNEL_INLINE void _block_descriptor_init(u64 address,u64 prev,u32 idx){
	pmm_block_descriptor_t* descriptor=_get_block_descriptor(address);
	descriptor->data=prev|(descriptor->data&(PAGE_SIZE-1)&(~PMM_ALLOCATOR_BUCKET_COUNT_MASK))|idx;
	descriptor->next=0;
}



static KERNEL_INLINE u64 _block_descriptor_get_idx(u64 address){
	return _get_block_descriptor(address)->data&PMM_ALLOCATOR_BUCKET_COUNT_MASK;
}



static KERNEL_INLINE u64 _block_descriptor_get_prev_idx(u64 address){
	return (_get_block_descriptor(address)->data>>PMM_ALLOCATOR_BUCKET_COUNT_MASK_SHIFT)&PMM_ALLOCATOR_BUCKET_COUNT_MASK;
}



static KERNEL_INLINE u64 _block_descriptor_get_prev(u64 address){
	return _get_block_descriptor(address)->data&(-PAGE_SIZE);
}



static KERNEL_INLINE void _block_descriptor_set_prev_idx(u64 address,u32 prev_idx){
	pmm_block_descriptor_t* descriptor=_get_block_descriptor(address);
	descriptor->data=(descriptor->data&(-(PMM_ALLOCATOR_BUCKET_COUNT_MASK<<PMM_ALLOCATOR_BUCKET_COUNT_MASK_SHIFT)-1))|(prev_idx<<PMM_ALLOCATOR_BUCKET_COUNT_MASK_SHIFT);
}



static KERNEL_INLINE void _block_descriptor_set_prev(u64 address,u64 prev){
	pmm_block_descriptor_t* descriptor=_get_block_descriptor(address);
	descriptor->data=(descriptor->data&(PAGE_SIZE-1))|prev;
}



static void KERNEL_EARLY_EXEC _add_memory_range(u64 address,u64 end){
	if (address>=end){
		return;
	}
	INFO("Registering memory range %p - %p",address,end);
#ifndef KERNEL_RELEASE
	INFO("Resetting memory...");
	mem_fill((void*)(address+VMM_HIGHER_HALF_ADDRESS_OFFSET),end-address,PMM_DEBUG_VALUE);
#endif
	do{
		pmm_allocator_t* allocator=_get_allocator_from_address(address);
		u32 idx=__builtin_ctzll(address)-PAGE_SIZE_SHIFT;
		if (idx>=PMM_ALLOCATOR_BUCKET_COUNT){
			idx=PMM_ALLOCATOR_BUCKET_COUNT-1;
		}
		u64 size=_get_block_size(idx);
		u64 length=end-address;
		if (size>length){
			idx=63-__builtin_clzll(length)-PAGE_SIZE_SHIFT;
			if (idx>=PMM_ALLOCATOR_BUCKET_COUNT){
				idx=PMM_ALLOCATOR_BUCKET_COUNT-1;
			}
			size=_get_block_size(idx);
		}
		_block_descriptor_set_prev_idx(address+_get_block_size(idx),idx);
		_block_descriptor_init(address,(allocator->buckets+idx)->tail,idx);
		if ((allocator->buckets+idx)->tail){
			_get_block_descriptor((allocator->buckets+idx)->tail)->next=address;
		}
		else{
			(allocator->buckets+idx)->head=address;
		}
		(allocator->buckets+idx)->tail=address;
		allocator->bucket_bitmap|=1<<idx;
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
	u64 first_free_address=kernel_data.first_free_address;
	LOG("Allocating memory...");
	_pmm_allocator_count=(max_address+PMM_ALLOCATOR_MAX_REGION_SIZE-1)>>PMM_ALLOCATOR_MAX_REGION_SIZE_SHIFT;
	u64 allocator_array_size=pmm_align_up_address(_pmm_allocator_count*sizeof(pmm_allocator_t));
	INFO("Allocator count: %u (%v)",_pmm_allocator_count,allocator_array_size);
	_pmm_allocators=(void*)(first_free_address+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	first_free_address+=allocator_array_size;
	mem_fill(_pmm_allocators,allocator_array_size,0);
	for (u32 i=0;i<_pmm_allocator_count;i++){
		spinlock_init(&((_pmm_allocators+i)->lock));
	}
	u64 block_descriptor_array_size=pmm_align_up_address(((max_address>>PAGE_SIZE_SHIFT)+1)*sizeof(pmm_block_descriptor_t));
	INFO("Block descriptor array size: %v",block_descriptor_array_size);
	_pmm_block_descriptors=(void*)(first_free_address+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	first_free_address+=block_descriptor_array_size;
	for (u64 i=0;i<((max_address+PAGE_SIZE)>>PAGE_SIZE_SHIFT);i++){
		(_pmm_block_descriptors+i)->data=PMM_ALLOCATOR_BLOCK_DESCRIPTOR_INDEX_USED;
		(_pmm_block_descriptors+i)->next=0;
		(_pmm_block_descriptors+i)->cookie=0;
		bitlock_init((u32*)(&((_pmm_block_descriptors+i)->data)),PMM_ALLOCATOR_BLOCK_DESCRIPTOR_LOCK_BIT);
	}
	spinlock_init(&(_pmm_load_balancer.lock));
	_pmm_load_balancer.index=0;
	_pmm_load_balancer.stats.hit_count=0;
	_pmm_load_balancer.stats.miss_count=0;
	_pmm_load_balancer.stats.miss_locked_count=0;
	pmm_load_balancer_stats=&(_pmm_load_balancer.stats);
	LOG("Registering low memory...");
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		u64 address=pmm_align_up_address((kernel_data.mmap+i)->base);
		if (address<first_free_address){
			address=first_free_address;
		}
		u64 end=pmm_align_down_address((kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length);
		_add_memory_range(address,(end>PMM_LOW_ALLOCATOR_LIMIT?PMM_LOW_ALLOCATOR_LIMIT:end));
	}
	_pmm_initialization_flags|=PMM_FLAG_PARTIALLY_INITIALIZED;
	LOG("Allocating structures...");
	pmm_counter_descriptor_t tmp_counter={
		.count=0
	};
	void* pmm_data=(void*)(pmm_alloc(pmm_align_up_address(allocator_array_size+block_descriptor_array_size)>>PAGE_SIZE_SHIFT,&tmp_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	mem_copy(pmm_data,_pmm_allocators,allocator_array_size);
	mem_copy(pmm_data+allocator_array_size,_pmm_block_descriptors,block_descriptor_array_size);
	_pmm_allocators=pmm_data;
	_pmm_block_descriptors=pmm_data+allocator_array_size;
	_pmm_self_counter_value=tmp_counter.count;
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
	_pmm_counter_allocator=omm_init("pmm_counter",sizeof(pmm_counter_descriptor_t),8,1);
	spinlock_init(&(_pmm_counter_allocator->lock));
	pmm_alloc_counter("pmm")->count=_pmm_self_counter_value;
	pmm_alloc_counter("kernel_image")->count=pmm_align_up_address(kernel_section_kernel_end()-kernel_section_kernel_start())>>PAGE_SIZE_SHIFT;
	pmm_alloc_counter("total")->count=total_memory;
	_pmm_initialization_flags|=PMM_FLAG_FULLY_INITIALIZED;
}



KERNEL_PUBLIC pmm_counter_descriptor_t* pmm_alloc_counter(const char* name){
	pmm_counter_descriptor_t* out=omm_alloc(_pmm_counter_allocator);
	out->name=name;
	out->count=0;
	handle_new(out,pmm_counter_handle_type,&(out->handle));
	handle_finish_setup(&(out->handle));
	return out;
}



KERNEL_PUBLIC void pmm_dealloc_counter(pmm_counter_descriptor_t* counter){
	handle_destroy(&(counter->handle));
	omm_dealloc(_pmm_counter_allocator,counter);
}



KERNEL_PUBLIC u64 pmm_alloc(u64 count,pmm_counter_descriptor_t* counter,bool memory_hint){
	if (!(_pmm_initialization_flags&PMM_FLAG_PARTIALLY_INITIALIZED)){
		return 0;
	}
	if (!count){
		panic("pmm_alloc: trying to allocate zero physical pages");
	}
	u32 i=63-__builtin_clzll(count)+(!!(count&(count-1)));
	if (i>=PMM_ALLOCATOR_BUCKET_COUNT){
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
	} while (!((_pmm_allocators+index)->bucket_bitmap>>i));
	spinlock_release_exclusive(&(_pmm_load_balancer.lock));
	u32 wrap=(memory_hint==PMM_MEMORY_HINT_LOW_MEMORY?PMM_LOW_ALLOCATOR_LIMIT/PMM_ALLOCATOR_MAX_REGION_SIZE-1:0xffffffff);
	index&=wrap;
	u32 base_index=index;
_retry_allocator:
	pmm_allocator_t* allocator=_pmm_allocators+index;
	spinlock_acquire_exclusive(&(allocator->lock));
	if (!(allocator->bucket_bitmap>>i)){
		spinlock_release_exclusive(&(allocator->lock));
		_pmm_load_balancer.stats.miss_locked_count++;
		index=(index+1)&wrap;
		if (index==_pmm_allocator_count){
			index=0;
		}
		if (index!=base_index){
			goto _retry_allocator;
		}
		panic("pmm_alloc: out of memory");
	}
	_pmm_load_balancer.stats.hit_count++;
	u32 j=__builtin_ffs(allocator->bucket_bitmap>>i)+i-1;
	u64 out=(allocator->buckets+j)->head;
#ifndef KERNEL_RELEASE
	if (_block_descriptor_get_idx(out)!=j){
		ERROR("List head corrupted [%u]: %p, %u",j,out,_block_descriptor_get_idx(out));
		panic("List head corrupted");
	}
#endif
	(allocator->buckets+j)->head=_get_block_descriptor(out)->next;
	if (!(allocator->buckets+j)->head){
		(allocator->buckets+j)->tail=0;
		allocator->bucket_bitmap&=~(1<<j);
	}
	else{
		_block_descriptor_set_prev((allocator->buckets+j)->head,0);
	}
	_block_descriptor_deinit(out);
	_block_descriptor_set_prev_idx(out+_get_block_size(i),i);
	while (j>i){
		j--;
		u64 split_block=out+_get_block_size(j);
		_block_descriptor_set_prev_idx(split_block+_get_block_size(j),j);
		_block_descriptor_init(split_block,0,j);
		(allocator->buckets+j)->head=split_block;
		(allocator->buckets+j)->tail=split_block;
		allocator->bucket_bitmap|=1<<j;
	}
	spinlock_release_exclusive(&(allocator->lock));
#ifndef KERNEL_RELEASE
	const u64* ptr=(const u64*)(out+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	bool error=0;
	for (u64 k=0;k<_get_block_size(i)/sizeof(u64);k++){
		if (_get_block_descriptor(out+k)->data&PMM_ALLOCATOR_BLOCK_DESCRIPTOR_FLAG_IS_CACHE){
			k+=PAGE_SIZE/sizeof(u64);
		}
		if (ptr[k]==PMM_DEBUG_VALUE*0x0101010101010101ull){
			continue;
		}
		ERROR("pmm_alloc: use after free at %p+%u/%u: %p",out,k<<3,_get_block_size(i),ptr[k]);
		error=1;
		break;
	}
	if (error){
		panic("pmm_alloc: use after free");
	}
#endif
	pmm_block_descriptor_t* block_descriptor=_get_block_descriptor(out);
	for (u64 offset=0;offset<_get_block_size(i);offset+=PAGE_SIZE){
		bitlock_acquire_exclusive((u32*)(&(block_descriptor->data)),PMM_ALLOCATOR_BLOCK_DESCRIPTOR_LOCK_BIT);
		if (block_descriptor->data&PMM_ALLOCATOR_BLOCK_DESCRIPTOR_FLAG_IS_CACHE){
			block_descriptor->data&=~PMM_ALLOCATOR_BLOCK_DESCRIPTOR_FLAG_IS_CACHE;
			WARN("Flush cache @ %p [cookie: %p]",out+offset,block_descriptor->cookie);
			block_descriptor->cookie=0;
		}
		u64* ptr=(u64*)(out+offset+VMM_HIGHER_HALF_ADDRESS_OFFSET);
		for (u64 k=0;k<PAGE_SIZE/sizeof(u64);k++){
			ptr[k]=0;
		}
		bitlock_release_exclusive((u32*)(&(block_descriptor->data)),PMM_ALLOCATOR_BLOCK_DESCRIPTOR_LOCK_BIT);
		block_descriptor++;
	}
	scheduler_resume();
	if ((_pmm_initialization_flags&PMM_FLAG_FULLY_INITIALIZED)&&!counter->handle.rb_node.key){
		handle_new(counter,pmm_counter_handle_type,&(counter->handle));
		handle_finish_setup(&(counter->handle));
	}
	counter->count+=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	return out;
}



KERNEL_PUBLIC void pmm_dealloc(u64 address,u64 count,pmm_counter_descriptor_t* counter){
#ifndef KERNEL_RELEASE
	mem_fill((void*)(address+VMM_HIGHER_HALF_ADDRESS_OFFSET),count<<PAGE_SIZE_SHIFT,PMM_DEBUG_VALUE);
#endif
	if (!count){
		panic("pmm_dealloc: trying to deallocate zero physical pages");
	}
	u32 i=63-__builtin_clzll(count)+(!!(count&(count-1)));
	if (i>=PMM_ALLOCATOR_BUCKET_COUNT){
		panic("pmm_dealloc: trying to deallocate too many pages at once");
	}
	scheduler_pause();
	counter->count-=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	pmm_allocator_t* allocator=_get_allocator_from_address(address);
	spinlock_acquire_exclusive(&(allocator->lock));
	while (i<PMM_ALLOCATOR_BUCKET_COUNT-1){
		if ((address&_get_block_size(i))&&_block_descriptor_get_prev_idx(address)!=i){
			break;
		}
		u64 buddy=address^_get_block_size(i);
		if (_block_descriptor_get_idx(buddy)!=i){
			break;
		}
		address&=~_get_block_size(i);
		u64 buddy_prev=_block_descriptor_get_prev(buddy);
		u64 buddy_next=_get_block_descriptor(buddy)->next;
		_block_descriptor_deinit(buddy);
		if (buddy_prev){
			_get_block_descriptor(buddy_prev)->next=buddy_next;
		}
		else{
			(allocator->buckets+i)->head=buddy_next;
			if (!buddy_next){
				allocator->bucket_bitmap&=~(1<<i);
			}
		}
		if (buddy_next){
			_block_descriptor_set_prev(buddy_next,buddy_prev);
		}
		else{
			(allocator->buckets+i)->tail=buddy_prev;
		}
		i++;
	}
	_block_descriptor_set_prev_idx(address+_get_block_size(i),i);
	_block_descriptor_init(address,(allocator->buckets+i)->tail,i);
	if ((allocator->buckets+i)->tail){
		_get_block_descriptor((allocator->buckets+i)->tail)->next=address;
	}
	else{
		(allocator->buckets+i)->head=address;
	}
	(allocator->buckets+i)->tail=address;
	allocator->bucket_bitmap|=1<<i;
	spinlock_release_exclusive(&(allocator->lock));
	scheduler_resume();
}
