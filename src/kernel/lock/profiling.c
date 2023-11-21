#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/_profiling_overload.h>
#include <kernel/lock/profiling.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "lock_profiling"



#define SPINLOCK_MAX_LOCK_TYPES 4096



static pmm_counter_descriptor_t _lock_profiling_data_pmm_counter=PMM_COUNTER_INIT_STRUCT("lock_profiling_data");
static pmm_counter_descriptor_t _lock_lock_type_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_lock_lock_type");
static omm_allocator_t* _lock_lock_type_allocator=NULL;
static u16 _lock_next_type_id=0;



HANDLE_DECLARE_TYPE(LOCK_TYPE,{});
HANDLE_DECLARE_TYPE(LOCK_DATA,{});



void lock_profiling_init(void){
	LOG("Initializing lock profiling data...");
	_lock_lock_type_allocator=omm_init("lock_lock_type",sizeof(lock_profiling_type_descriptor_t),8,1,&_lock_lock_type_omm_pmm_counter);
}



u16 __lock_profiling_alloc_type(const char* func,u32 line,u16* out){
	if (((u16)((*out)+1))>1){
		return *out;
	}
	u16 expected=0;
	if (__atomic_compare_exchange_n(out,&expected,0xffff,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)){
		*out=__atomic_add_fetch(&_lock_next_type_id,1,__ATOMIC_SEQ_CST);
	}
	SPINLOOP(*out==0xffff);
	return *out;
}



lock_local_profiling_data_t* __lock_profiling_alloc_data(const char* func,u32 line,u16 offset,u64* ptr){
	u64 expected=0;
	if (__atomic_compare_exchange_n(ptr,&expected,1,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)){
		u64 data=pmm_alloc(pmm_align_up_address(SPINLOCK_MAX_LOCK_TYPES*sizeof(lock_local_profiling_data_t))>>PAGE_SIZE_SHIFT,&_lock_profiling_data_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET;
		if (!data){
			return NULL;
		}
		lock_profiling_data_descriptor_t* descriptor=(void*)data;
		descriptor->func=func;
		descriptor->line=line;
		*ptr=data+sizeof(lock_profiling_data_descriptor_t);
	}
	if (*ptr<2){
		return NULL;
	}
	return (void*)((*ptr)+offset*sizeof(lock_local_profiling_data_t));
}
