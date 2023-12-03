#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "amm"



static pmm_counter_descriptor_t* _amm_omm_pmm_counter=NULL;
static omm_allocator_t* KERNEL_BSS _amm_allocators[(PAGE_SIZE_SHIFT-3)<<1];



typedef struct _AMM_HEADER{
	u64 index;
	u8 data[];
} amm_header_t;



static KERNEL_INLINE u64 _size_to_index(u64 size){
	size=(size+7)&0xfffffffffffffff8ull;
	u32 i=60-__builtin_clzll(size);
	return (size>3ull<<(PAGE_SIZE_SHIFT-2)?pmm_align_up_address(size):(i<<1)+(!!(size&(size-1)))+(size>(3<<(i+2))));
}



static KERNEL_INLINE u64 _index_to_size(u64 index){
	return (index>>PAGE_SIZE_SHIFT?index:(8|((index&1)<<4))<<((index>>1)-(index&1)));
}



static amm_header_t* _alloc_memory(u64 index){
	if (!_amm_omm_pmm_counter){
		_amm_omm_pmm_counter=pmm_alloc_counter("amm");
	}
	if (index>>PAGE_SIZE_SHIFT){
		return (void*)(pmm_alloc(index>>PAGE_SIZE_SHIFT,_amm_omm_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	}
	omm_allocator_t** allocator=_amm_allocators+index;
	if (!(*allocator)){
		*allocator=omm_init("amm_allocator_???",_index_to_size(index)+sizeof(amm_header_t),4,2,_amm_omm_pmm_counter);
		spinlock_init(&((*allocator)->lock));
	}
	return omm_alloc(*allocator);
}



static void _dealloc_memory(amm_header_t* header){
	if (header->index>>PAGE_SIZE_SHIFT){
		pmm_dealloc(((u64)header)-VMM_HIGHER_HALF_ADDRESS_OFFSET,header->index>>PAGE_SIZE_SHIFT,_amm_omm_pmm_counter);
	}
	else{
		omm_dealloc(_amm_allocators[header->index],header);
	}
}



KERNEL_PUBLIC void* amm_alloc(u32 length){
	if (!length){
		return NULL;
	}
	u64 index=_size_to_index(((u64)length)+sizeof(amm_header_t));
	amm_header_t* out=_alloc_memory(index);
	out->index=index;
	return out->data;
}



KERNEL_PUBLIC void amm_dealloc(void* ptr){
	if (!ptr){
		return;
	}
	amm_header_t* header=(amm_header_t*)(((u64)ptr)-__builtin_offsetof(amm_header_t,data));
	_dealloc_memory(header);
}



KERNEL_PUBLIC void* amm_realloc(void* ptr,u32 length){
	if (!ptr){
		return amm_alloc(length);
	}
	if (!length){
		amm_dealloc(ptr);
		return NULL;
	}
	panic("amm_realloc");
}
