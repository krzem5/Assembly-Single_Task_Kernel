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



HANDLE_DECLARE_TYPE(PMM_COUNTER,{
	panic("Unable to delete	HANDLE_TYPE_PMM_COUNTER");
});



static pmm_allocator_t _pmm_low_allocator;
static pmm_allocator_t _pmm_high_allocator;



static KERNEL_INLINE pmm_allocator_page_header_t* _get_block_header(u64 address){
	return (pmm_allocator_page_header_t*)(address+VMM_HIGHER_HALF_ADDRESS_OFFSET);
}



static KERNEL_INLINE u64 _get_bitmap_size(const pmm_allocator_t* allocator){
	return pmm_align_up_address(((((allocator->last_address-allocator->first_address)>>PAGE_SIZE_SHIFT)+64)>>6)*sizeof(u64));
	 // 64 instead of 63 to add one more bit for the end of the last memory address
}



static KERNEL_INLINE _Bool _get_address_bit(const pmm_allocator_t* allocator,u64 address){
	address=(address-allocator->first_address)>>PAGE_SIZE_SHIFT;
	return !!(allocator->bitmap[address>>6]&(1ull<<(address&63)));
}



static KERNEL_INLINE void _toggle_address_bit(const pmm_allocator_t* allocator,u64 address){
	address=(address-allocator->first_address)>>PAGE_SIZE_SHIFT;
	allocator->bitmap[address>>6]^=1ull<<(address&63);
}



static KERNEL_INLINE u64 _get_block_size(u8 index){
	return 1ull<<(PAGE_SIZE_SHIFT+index);
}



static void _add_memory_range(pmm_allocator_t* allocator,u64 address,u64 end){
	if (address>=end){
		return;
	}
	INFO("Registering memory range %p - %p",address,end);
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
		_pmm_total_pmm_counter.count+=size>>PAGE_SIZE_SHIFT;
		pmm_allocator_page_header_t* header=_get_block_header(address);
		header->prev=0;
		header->next=allocator->blocks[idx];
		header->cleared_pages=0;
		header->lock=0;
		header->idx=idx;
		if (allocator->blocks[idx]){
			_get_block_header(allocator->blocks[idx])->prev=address;
		}
		allocator->blocks[idx]=address;
		allocator->block_bitmap|=1<<idx;
		address+=size;
	} while (address<end);
}



static void _memory_clear_thread(void){
	WARN("Clearing memory...");
	while (1){
		scheduler_pause();
		// u32 expected=0;
		// if (!__atomic_compare_exchange_n(&(header->lock),&expected,PMM_LOCK_FLAG_CLEAR,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)){continue;}
		// __atomic_fetch_xor(&(header->lock),PMM_LOCK_FLAG_CLEAR,__ATOMIC_SEQ_CST);
		scheduler_resume();
		scheduler_yield();
	}
}



void pmm_init(void){
	LOG("Initializing physical memory manager...");
	LOG("Scanning memory...");
	_pmm_low_allocator.first_address=0;
	_pmm_low_allocator.last_address=0;
	spinlock_init(&(_pmm_low_allocator.lock));
	_pmm_high_allocator.first_address=PMM_LOW_ALLOCATOR_LIMIT;
	_pmm_high_allocator.last_address=PMM_LOW_ALLOCATOR_LIMIT;
	spinlock_init(&(_pmm_high_allocator.lock));
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		u64 end=pmm_align_down_address((kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length);
		if (end>_pmm_low_allocator.last_address){
			_pmm_low_allocator.last_address=(end>PMM_LOW_ALLOCATOR_LIMIT?PMM_LOW_ALLOCATOR_LIMIT:end);
		}
		if (end>_pmm_high_allocator.last_address){
			_pmm_high_allocator.last_address=end;
		}
	}
	u64 low_bitmap_size=_get_bitmap_size(&_pmm_low_allocator);
	INFO("Low bitmap size: %v",low_bitmap_size);
	_pmm_low_allocator.bitmap=(void*)(pmm_align_up_address(kernel_data.first_free_address)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	memset(_pmm_low_allocator.bitmap,0,low_bitmap_size);
	u64 high_bitmap_size=_get_bitmap_size(&_pmm_high_allocator);
	INFO("High bitmap size: %v",high_bitmap_size);
	_pmm_high_allocator.bitmap=(void*)(pmm_align_up_address(kernel_data.first_free_address)+low_bitmap_size+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	memset(_pmm_high_allocator.bitmap,0,high_bitmap_size);
	LOG("Registering counters...");
	handle_new(&_pmm_pmm_counter,HANDLE_TYPE_PMM_COUNTER,&(_pmm_pmm_counter.handle));
	handle_new(&_pmm_kernel_image_pmm_counter,HANDLE_TYPE_PMM_COUNTER,&(_pmm_kernel_image_pmm_counter.handle));
	handle_new(&_pmm_total_pmm_counter,HANDLE_TYPE_PMM_COUNTER,&(_pmm_total_pmm_counter.handle));
	handle_finish_setup(&(_pmm_pmm_counter.handle));
	handle_finish_setup(&(_pmm_kernel_image_pmm_counter.handle));
	handle_finish_setup(&(_pmm_total_pmm_counter.handle));
	_pmm_pmm_counter.count+=pmm_align_up_address(low_bitmap_size+high_bitmap_size)>>PAGE_SIZE_SHIFT;
	_pmm_kernel_image_pmm_counter.count+=pmm_align_up_address(kernel_section_kernel_end()-kernel_section_kernel_start())>>PAGE_SIZE_SHIFT;
	kernel_data.first_free_address+=low_bitmap_size+high_bitmap_size;
	LOG("Registering low memory...");
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		u64 address=pmm_align_up_address((kernel_data.mmap+i)->base);
		if (address<kernel_data.first_free_address){
			address=kernel_data.first_free_address;
		}
		u64 end=pmm_align_down_address((kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length);
		_add_memory_range(&_pmm_low_allocator,address,(end>PMM_LOW_ALLOCATOR_LIMIT?PMM_LOW_ALLOCATOR_LIMIT:end));
	}
}



void pmm_init_high_mem(void){
	LOG("Registering high memory...");
	for (u16 i=0;i<kernel_data.mmap_size;i++){
		u64 address=pmm_align_up_address((kernel_data.mmap+i)->base);
		u64 end=pmm_align_down_address((kernel_data.mmap+i)->base+(kernel_data.mmap+i)->length);
		_add_memory_range(&_pmm_high_allocator,(address<PMM_LOW_ALLOCATOR_LIMIT?PMM_LOW_ALLOCATOR_LIMIT:address),end);
	}
}



u64 pmm_alloc(u64 count,pmm_counter_descriptor_t* counter,_Bool memory_hint){
	scheduler_pause();
	if (!count){
		panic("pmm_alloc: trying to allocate zero physical pages");
	}
	u8 i=63-__builtin_clzll(count)+(!!(count&(count-1)));
	if (i>=PMM_ALLOCATOR_SIZE_COUNT){
		panic("pmm_alloc: trying to allocate too many pages at once");
	}
	pmm_allocator_t* allocator=(memory_hint==PMM_MEMORY_HINT_LOW_MEMORY||!_pmm_high_allocator.block_bitmap||__builtin_ffs(_pmm_high_allocator.block_bitmap>>i)>__builtin_ffs(_pmm_low_allocator.block_bitmap>>i)?&_pmm_low_allocator:&_pmm_high_allocator);
	spinlock_acquire_exclusive(&(allocator->lock));
	if (!(allocator->block_bitmap>>i)){
		spinlock_release_exclusive(&(allocator->lock));
		panic("pmm_alloc: out of memory");
	}
	u8 j=__builtin_ffs(allocator->block_bitmap>>i)+i-1;
	u64 out=(u64)(allocator->blocks[j]);
	pmm_allocator_page_header_t* header=_get_block_header(out);
	SPINLOOP(__atomic_fetch_or(&(header->lock),PMM_LOCK_FLAG_ALLOC,__ATOMIC_SEQ_CST)&PMM_LOCK_FLAG_CLEAR);
	u64 first_uncleared_address=out+(header->cleared_pages<<PAGE_SIZE_SHIFT);
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
		header->cleared_pages=(first_uncleared_address<=child_block?0:(first_uncleared_address-child_block)>>PAGE_SIZE_SHIFT);
		if (header->cleared_pages>(1ull<<j)){
			header->cleared_pages=1ull<<j;
		}
		header->lock=0;
		header->idx=j;
		allocator->blocks[j]=child_block;
		allocator->block_bitmap|=1<<j;
	}
	_toggle_address_bit(allocator,out);
	_toggle_address_bit(allocator,out+_get_block_size(i));
	spinlock_release_exclusive(&(allocator->lock));
	if (!counter->handle.rb_node.key){
		handle_new(counter,HANDLE_TYPE_PMM_COUNTER,&(counter->handle));
		handle_finish_setup(&(counter->handle));
	}
	counter->count+=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	scheduler_resume();
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
	if (i>=PMM_ALLOCATOR_SIZE_COUNT){
		panic("pmm_dealloc: trying to deallocate too many pages at once");
	}
	counter->count-=_get_block_size(i)>>PAGE_SIZE_SHIFT;
	pmm_allocator_t* allocator=(address<PMM_LOW_ALLOCATOR_LIMIT?&_pmm_low_allocator:&_pmm_high_allocator);
	spinlock_acquire_exclusive(&(allocator->lock));
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
	header->cleared_pages=0;
	header->lock=0;
	header->idx=i;
	if (allocator->blocks[i]){
		_get_block_header(allocator->blocks[i])->prev=address;
	}
	allocator->blocks[i]=address;
	allocator->block_bitmap|=1<<i;
	spinlock_release_exclusive(&(allocator->lock));
	scheduler_resume();
}



void pmm_register_memory_clear_thread(void){
	LOG("Registering memory clearer thread...");
	thread_t* thread=thread_new_kernel_thread(process_kernel,_memory_clear_thread,0x200000,0);
	thread->priority=SCHEDULER_PRIORITY_BACKGROUND;
	scheduler_enqueue_thread(thread);
}
