#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/profiling.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "spinlock"



#define SPINLOCK_MAX_LOCK_TYPES 4096



static pmm_counter_descriptor_t _spinlock_profiling_data_pmm_counter=PMM_COUNTER_INIT_STRUCT("spinlock_profiling_data");
static pmm_counter_descriptor_t _spinlock_lock_type_omm_pmm_counter=PMM_COUNTER_INIT_STRUCT("omm_spinlock_lock_type");
static omm_allocator_t* _spinlock_lock_type_allocator=NULL;
static u16 _spinlock_next_type_id=0;



HANDLE_DECLARE_TYPE(SPINLOCK_TYPE,{});
HANDLE_DECLARE_TYPE(SPINLOCK_DATA,{});



void spinlock_profiling_init(void){
	LOG("Initializing spinlock profiling data...");
	_spinlock_lock_type_allocator=omm_init("spinlock_lock_type",sizeof(spinlock_profiling_setup_descriptor_t),8,1,&_spinlock_lock_type_omm_pmm_counter);
}



u16 __spinlock_profiling_alloc_lock_type(const char* func,u32 line,u16* out){
	if (((u16)((*out)+1))>1){
		return *out;
	}
	u16 expected=0;
	if (__atomic_compare_exchange_n(out,&expected,0xffff,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)){
		*out=__atomic_add_fetch(&_spinlock_next_type_id,1,__ATOMIC_SEQ_CST);
	}
	SPINLOOP(*out==0xffff);
	return *out;
}



spinlock_profiling_data_t* __spinlock_profiling_process_data(const char* func,u32 line,const spinlock_t* lock,u64* ptr){
	u64 expected=0;
	if (__atomic_compare_exchange_n(ptr,&expected,1,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)){
		u64 data=pmm_alloc(pmm_align_up_address(SPINLOCK_MAX_LOCK_TYPES*sizeof(spinlock_profiling_data_t))>>PAGE_SIZE_SHIFT,&_spinlock_profiling_data_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET;
		if (!data){
			return NULL;
		}
		spinlock_profiling_descriptor_t* descriptor=(void*)data;
		descriptor->func=func;
		descriptor->line=line;
		*ptr=data+sizeof(spinlock_profiling_descriptor_t);
	}
	if (*ptr<2){
		return NULL;
	}
	return (void*)((*ptr)+((*lock)>>16)*sizeof(spinlock_profiling_data_t));
}
