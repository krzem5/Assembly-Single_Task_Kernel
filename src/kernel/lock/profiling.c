#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/_profiling_overload.h>
#include <kernel/lock/profiling.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "lock_profiling"



#define SPINLOCK_MAX_LOCK_TYPES 4096
#define SPINLOCK_EARLY_LOCK_TYPES 8



static pmm_counter_descriptor_t _lock_profiling_pmm_counter=PMM_COUNTER_INIT_STRUCT("lock_profiling");
static spinlock_t _lock_profiling_data_lock=SPINLOCK_INIT_STRUCT;
static KERNEL_ATOMIC u16 KERNEL_NOBSS _lock_next_type_id=0;
static lock_profiling_type_descriptor_t* _lock_profiling_type_descriptors=NULL;
static lock_profiling_type_descriptor_t KERNEL_NOBSS _lock_profiling_early_types[SPINLOCK_EARLY_LOCK_TYPES]={
	{
		.func="<unknown>",
		.line=0
	}
};



const lock_profiling_type_descriptor_t* lock_profiling_type_descriptors=NULL;
const lock_profiling_data_descriptor_t* lock_profiling_data_descriptor_head=NULL;



void lock_profiling_init(void){
	LOG("Initializing lock profiling data...");
}



u16 __lock_profiling_alloc_type(const char* func,u32 line,u16* out){
	if (((u16)((*out)+1))>1){
		return *out;
	}
	u16 expected=0;
	if (__atomic_compare_exchange_n(out,&expected,0xffff,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)){
		*out=__atomic_add_fetch(&_lock_next_type_id,1,__ATOMIC_SEQ_CST);
		if (_lock_profiling_type_descriptors){
			goto _skip_alloc;
		}
		u64 data=pmm_alloc(pmm_align_up_address(SPINLOCK_MAX_LOCK_TYPES*sizeof(lock_profiling_type_descriptor_t))>>PAGE_SIZE_SHIFT,&_lock_profiling_pmm_counter,0);
		if (!data){
			goto _skip_alloc;
		}
		_lock_profiling_type_descriptors=(void*)(data+VMM_HIGHER_HALF_ADDRESS_OFFSET);
		lock_profiling_type_descriptors=_lock_profiling_type_descriptors;
		if (*out!=SPINLOCK_EARLY_LOCK_TYPES){
			panic("Not enough early locks");
		}
		for (u16 i=0;i<SPINLOCK_EARLY_LOCK_TYPES;i++){
			*(_lock_profiling_type_descriptors+i)=*(_lock_profiling_early_types+i);
		}
_skip_alloc:
		if (!_lock_profiling_type_descriptors&&*out>=SPINLOCK_EARLY_LOCK_TYPES){
			panic("Too many early locks");
		}
		lock_profiling_type_descriptor_t* lock_profiling_type_descriptor=(_lock_profiling_type_descriptors?_lock_profiling_type_descriptors:_lock_profiling_early_types)+(*out);
		lock_profiling_type_descriptor->func=func;
		lock_profiling_type_descriptor->line=line;
	}
	SPINLOOP(*out==0xffff);
	return *out;
}



lock_local_profiling_data_t* __lock_profiling_alloc_data(const char* func,u32 line,u16 offset,u64* ptr){
	u64 expected=0;
	if (__atomic_compare_exchange_n(ptr,&expected,1,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)){
		u64 data=pmm_alloc(pmm_align_up_address(SPINLOCK_MAX_LOCK_TYPES*sizeof(lock_local_profiling_data_t))>>PAGE_SIZE_SHIFT,&_lock_profiling_pmm_counter,0);
		if (!data){
			return NULL;
		}
		data+=VMM_HIGHER_HALF_ADDRESS_OFFSET;
		lock_profiling_data_descriptor_t* descriptor=(void*)data;
		descriptor->func=func;
		descriptor->line=line;
		(spinlock_acquire_exclusive)(&_lock_profiling_data_lock);
		descriptor->next=lock_profiling_data_descriptor_head;
		lock_profiling_data_descriptor_head=descriptor;
		(spinlock_release_exclusive)(&_lock_profiling_data_lock);
		*ptr=data+sizeof(lock_profiling_data_descriptor_t);
	}
	if (*ptr<2){
		return NULL;
	}
	return (void*)((*ptr)+offset*sizeof(lock_local_profiling_data_t));
}
