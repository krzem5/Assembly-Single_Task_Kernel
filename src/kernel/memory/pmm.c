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



static pmm_allocator_t _pmm_low_allocator;
static pmm_allocator_t _pmm_high_allocator;



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
		pmm_allocator_page_header_t* header=(void*)(address+VMM_HIGHER_HALF_ADDRESS_OFFSET);
		header->prev=(allocator->blocks+idx)->tail;
		header->next=NULL;
		header->idx=idx;
		if (header->prev){
			header->prev->next=header;
		}
		else{
			(allocator->blocks+idx)->head=header;
		}
		(allocator->blocks+idx)->tail=header;
		allocator->block_bitmap|=1<<idx;
		address+=size;
	} while (address<end);
}



void pmm_init(void){
	LOG("Initializing physical memory manager...");
	LOG("Scanning memory...");
	memset(&_pmm_low_allocator,0,sizeof(pmm_allocator_t));
	_pmm_low_allocator.first_address=0;
	_pmm_low_allocator.last_address=0;
	spinlock_init(&(_pmm_low_allocator.lock));
	memset(&_pmm_high_allocator,0,sizeof(pmm_allocator_t));
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
	pmm_allocator_page_header_t* header=(allocator->blocks+j)->head;
	(allocator->blocks+j)->head=header->next;
	if (header->next){
		header->next->prev=0;
	}
	else{
		(allocator->blocks+j)->tail=NULL;
		allocator->block_bitmap&=~(1<<j);
	}
	u64 out=((u64)header)-VMM_HIGHER_HALF_ADDRESS_OFFSET;
	while (j>i){
		j--;
		header=(void*)(out+_get_block_size(j)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
		header->prev=0;
		header->next=0;
		header->idx=j;
		(allocator->blocks+j)->head=header;
		(allocator->blocks+j)->tail=header;
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
		pmm_allocator_page_header_t* buddy=(void*)((address^_get_block_size(i))+VMM_HIGHER_HALF_ADDRESS_OFFSET);
		if (((u64)buddy)>=allocator->last_address||_get_address_bit(allocator,address|_get_block_size(i))||buddy->idx!=i){
			break;
		}
		address&=~_get_block_size(i);
		if (buddy->prev){
			buddy->prev->next=buddy->next;
		}
		else{
			(allocator->blocks+i)->head=buddy->next;
			if (!buddy->next){
				allocator->block_bitmap&=~(1<<i);
			}
		}
		if (buddy->next){
			buddy->next->prev=buddy->prev;
		}
		else{
			(allocator->blocks+i)->tail=buddy->prev;
		}
		i++;
	}
	pmm_allocator_page_header_t* header=(void*)(address+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	header->prev=(allocator->blocks+i)->tail;
	header->next=0;
	header->idx=i;
	if (header->prev){
		header->prev->next=header;
	}
	else{
		(allocator->blocks+i)->head=header;
	}
	(allocator->blocks+i)->tail=header;
	allocator->block_bitmap|=1<<i;
	spinlock_release_exclusive(&(allocator->lock));
	scheduler_resume();
}
