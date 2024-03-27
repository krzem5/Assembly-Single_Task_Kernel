#include <kernel/format/format.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/amm.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "amm"



#define ALLOCATOR_COUNT ((PAGE_SIZE_SHIFT-3)<<1)



static pmm_counter_descriptor_t* KERNEL_INIT_WRITE _amm_pmm_counter=NULL;
static omm_allocator_t* KERNEL_INIT_WRITE _amm_allocators[ALLOCATOR_COUNT];



static KERNEL_INLINE u64 _size_to_index(u64 size){
	size=(size+7)&0xfffffffffffffff8ull;
	u32 i=60-__builtin_clzll(size);
	return (size>3ull<<(PAGE_SIZE_SHIFT-2)?8ull<<(i+(!!(size&(size-1)))):(i<<1)+(!!(size&(size-1)))+(size>(3<<(i+2))));
}



static KERNEL_INLINE u64 _index_to_size(u64 index){
	return (index>=PAGE_SIZE?index:(8|((index&1)<<4))<<((index>>1)-(index&1)));
}



void KERNEL_EARLY_EXEC amm_init(void){
	LOG("Initializing arbitrary memory manager...");
	_amm_pmm_counter=pmm_alloc_counter("amm");
	char* allocator_names=(void*)(pmm_alloc(pmm_align_up_address(20*ALLOCATOR_COUNT)>>PAGE_SIZE_SHIFT,_amm_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	for (u64 i=0;i<ALLOCATOR_COUNT;i++){
		if (i==1){ // allocator of size 12 is impossible (qword alignment requirement)
			continue;
		}
		const char* name=allocator_names;
		allocator_names+=format_string(allocator_names,20,"amm_allocator_%u",_index_to_size(i))+1;
		_amm_allocators[i]=omm_init(name,_index_to_size(i)+sizeof(amm_header_t),8,4,_amm_pmm_counter);
		spinlock_init(&(_amm_allocators[i]->lock));
	}
}



KERNEL_PUBLIC void* amm_alloc(u32 length){
	if (!length){
		return NULL;
	}
	u64 index=_size_to_index(((u64)length)+sizeof(amm_header_t));
	amm_header_t* out=(index>=PAGE_SIZE?(void*)(pmm_alloc(index>>PAGE_SIZE_SHIFT,_amm_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET):omm_alloc(_amm_allocators[index]));
	out->index=index;
	mem_fill(out->data,length,0);
	return out->data;
}



KERNEL_PUBLIC void amm_dealloc(void* ptr){
	if (!ptr){
		return;
	}
	amm_header_t* header=(amm_header_t*)(((u64)ptr)-__builtin_offsetof(amm_header_t,data));
	u64 index=header->index;
	mem_fill(header,0,_index_to_size(index));
	if (index>=PAGE_SIZE){
		pmm_dealloc(((u64)header)-VMM_HIGHER_HALF_ADDRESS_OFFSET,index>>PAGE_SIZE_SHIFT,_amm_pmm_counter);
	}
	else{
		omm_dealloc(_amm_allocators[index],header);
	}
}



KERNEL_PUBLIC void* amm_realloc(void* ptr,u32 length){
	if (!ptr){
		return amm_alloc(length);
	}
	if (!length){
		amm_dealloc(ptr);
		return NULL;
	}
	amm_header_t* header=(amm_header_t*)(((u64)ptr)-__builtin_offsetof(amm_header_t,data));
	u64 index=_size_to_index(((u64)length)+sizeof(amm_header_t));
	if (index==header->index){
		return ptr;
	}
	u64 current_length=_index_to_size(header->index)-sizeof(amm_header_t);
	void* out=amm_alloc(length);
	mem_copy(out,ptr,(length<current_length?length:current_length));
	amm_dealloc(ptr);
	return out;
}
