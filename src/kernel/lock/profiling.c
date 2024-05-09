#ifndef KERNEL_RELEASE
#include <kernel/clock/clock.h>
#include <kernel/cpu/local.h>
#include <kernel/lock/bitlock.h>
#include <kernel/lock/profiling.h>
#include <kernel/log/log.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



#define GLOBAL_LOCK_TYPE_BIT 0
#define GLOBAL_LOCK_STAT_BIT 1



static u32 _lock_profiling_lock_types[LOCK_PROFILING_MAX_LOCK_TYPES];
static KERNEL_ATOMIC u32 _lock_profiling_lock_type_count=0;
static u32 _lock_profiling_global_locks=0;
static u64* KERNEL_INIT_WRITE _lock_profiling_dependency_matrix=NULL;
static __lock_profiling_lock_stack_t KERNEL_EARLY_WRITE _lock_profiling_global_lock_stack={
	.size=0
};
static CPU_LOCAL_DATA(__lock_profiling_lock_stack_t,_lock_profiling_cpu_local_data);
static lock_profiling_stats_t* KERNEL_INIT_WRITE _lock_profiling_lock_stats=NULL;
static u32 _lock_profiling_lock_stat_count=0;



static KERNEL_INLINE __lock_profiling_lock_stack_t* KERNEL_NOCOVERAGE _get_lock_stack(void){
	if (!_lock_profiling_cpu_local_data){
		return &_lock_profiling_global_lock_stack;
	}
#ifndef KERNEL_RELEASE
	if (CPU_HEADER_DATA->current_thread){
		return &(CPU_HEADER_DATA->current_thread->__lock_profiling_lock_stack);
	}
#endif
	return CPU_LOCAL(_lock_profiling_cpu_local_data);
}



static KERNEL_INLINE u32 KERNEL_NOCOVERAGE _get_edge_index(u32 from,u32 to){
	return (from<<LOCK_PROFILING_MAX_LOCK_TYPES_SHIFT)|to;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_init(__lock_profiling_data_t* out){
	out->alloc_address=(u32)(u64)__builtin_return_address(1);
	u32 type_count=_lock_profiling_lock_type_count;
	for (u32 i=0;i<type_count;i++){
		if (_lock_profiling_lock_types[i]==out->alloc_address){
			out->id=i;
			return;
		}
	}
	bitlock_acquire(&_lock_profiling_global_locks,GLOBAL_LOCK_TYPE_BIT);
	out->id=_lock_profiling_lock_type_count;
	_lock_profiling_lock_types[_lock_profiling_lock_type_count]=out->alloc_address;
	_lock_profiling_lock_type_count++;
	bitlock_release(&_lock_profiling_global_locks,GLOBAL_LOCK_TYPE_BIT);
}



void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_init_lock_stack(__lock_profiling_lock_stack_t* out){
	out->size=0;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_acquire_start(__lock_profiling_data_t* lock,__lock_profiling_acquisition_context_t* ctx){
	if (!_lock_profiling_dependency_matrix||!_lock_profiling_lock_stats){
		return;
	}
	u32 address=(u64)__builtin_return_address(1);
	u32 stat_count=_lock_profiling_lock_stat_count;
	for (u32 i=0;i<stat_count;i++){
		if ((_lock_profiling_lock_stats+i)->address==address){
			ctx->stat_id=i;
			goto _skip_stat_alloc;
		}
	}
	bitlock_acquire(&_lock_profiling_global_locks,GLOBAL_LOCK_STAT_BIT);
	ctx->stat_id=_lock_profiling_lock_stat_count;
	(_lock_profiling_lock_stats+ctx->stat_id)->id=lock->id;
	(_lock_profiling_lock_stats+ctx->stat_id)->address=address;
	_lock_profiling_lock_stat_count++;
	bitlock_release(&_lock_profiling_global_locks,GLOBAL_LOCK_STAT_BIT);
_skip_stat_alloc:
	__lock_profiling_lock_stack_t* stack=_get_lock_stack();
	if (stack->size>=LOCK_PROFILING_MAX_NESTED_LOCKS){
		_log_untraced("\x1b[1m\x1b[38;2;41;137;255m: Lock stack too small\x1b[0m\n");
		for (;;);
	}
	for (u64 i=0;i<stack->size;i++){
		if (stack->data[i]->id==lock->id&&stack->data[i]!=lock){
			continue;
		}
		u32 edge=_get_edge_index(stack->data[i]->id,lock->id);
		_lock_profiling_dependency_matrix[edge>>6]|=1ull<<(edge&63);
		u32 inverse_edge=_get_edge_index(lock->id,stack->data[i]->id);
		if (_lock_profiling_dependency_matrix[inverse_edge>>6]&(1ull<<(inverse_edge&63))){
			_log_untraced("\x1b[1m\x1b[38;2;41;137;255mLock '%p' deadlocked by '%p'\x1b[0m\n",lock->alloc_address|0xffffffff00000000ull,stack->data[i]->alloc_address|0xffffffff00000000ull);
		}
	}
	stack->data[stack->size]=lock;
	stack->size++;
	ctx->start_ticks=clock_get_ticks();
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_acquire_end(__lock_profiling_data_t* lock,__lock_profiling_acquisition_context_t* ctx){
	u64 end_ticks=clock_get_ticks();
	if (!_lock_profiling_dependency_matrix||!_lock_profiling_lock_stats){
		return;
	}
	u64 ticks=end_ticks-ctx->start_ticks;
	lock_profiling_stats_t* stats=_lock_profiling_lock_stats+ctx->stat_id;
	__atomic_add_fetch(&(stats->ticks),ticks,__ATOMIC_SEQ_CST);
	__atomic_add_fetch(&(stats->count),1,__ATOMIC_SEQ_CST);
	if (ticks>stats->max_ticks){
		__atomic_store_n(&(stats->max_ticks),ticks,__ATOMIC_SEQ_CST);
	}
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_release(__lock_profiling_data_t* lock){
	if (!_lock_profiling_dependency_matrix||!_lock_profiling_lock_stats){
		return;
	}
	__lock_profiling_lock_stack_t* stack=_get_lock_stack();
	u64 i=0;
	for (;i<stack->size&&stack->data[i]!=lock;i++);
	if (i==stack->size){
		_log_untraced("\x1b[1m\x1b[38;2;41;137;255mLock '%p' not acquired in this context\x1b[0m\n",lock);
		for (;;);
	}
	stack->size--;
	stack->data[i]=stack->data[stack->size];
}



void KERNEL_NOCOVERAGE KERNEL_EARLY_EXEC lock_profiling_enable_dependency_graph(void){
	pmm_counter_descriptor_t* counter=pmm_alloc_counter("lock_profiling");
	_lock_profiling_dependency_matrix=(void*)(pmm_alloc(pmm_align_up_address(1<<(LOCK_PROFILING_MAX_LOCK_TYPES_SHIFT<<1))>>PAGE_SIZE_SHIFT,counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
	_lock_profiling_lock_stats=(void*)(pmm_alloc(pmm_align_up_address(LOCK_PROFILING_MAX_LOCK_USES*sizeof(lock_profiling_stats_t))>>PAGE_SIZE_SHIFT,counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
}



KERNEL_PUBLIC bool lock_profiling_get_descriptor(u32 index,lock_profiling_descriptor_t* out){
	if (index>=_lock_profiling_lock_type_count){
		return 0;
	}
	out->id=index;
	out->address=_lock_profiling_lock_types[index];
	return 1;
}



KERNEL_PUBLIC bool lock_profiling_get_stats(u32 index,lock_profiling_stats_t* out){
	if (index>=_lock_profiling_lock_stat_count){
		return 0;
	}
	*out=*(_lock_profiling_lock_stats+index);
	return 0;
}



#else
#include <kernel/lock/profiling.h>
#include <kernel/types.h>



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_init(__lock_profiling_data_t* out){
	return;
}



void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_init_lock_stack(__lock_profiling_lock_stack_t* out){
	return;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_acquire_start(__lock_profiling_data_t* lock,__lock_profiling_acquisition_context_t* ctx){
	return;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_acquire_end(__lock_profiling_data_t* lock,__lock_profiling_acquisition_context_t* ctx){
	return;
}



KERNEL_PUBLIC void KERNEL_NOCOVERAGE KERNEL_NOINLINE __lock_profiling_release(__lock_profiling_data_t* lock){
	return;
}



void KERNEL_NOCOVERAGE KERNEL_EARLY_EXEC lock_profiling_enable_dependency_graph(void){
	return;
}



KERNEL_PUBLIC bool lock_profiling_get_descriptor(u32 index,lock_profiling_descriptor_t* out){
	return 0;
}



KERNEL_PUBLIC bool lock_profiling_get_stats(u32 index,lock_profiling_stats_t* out){
	return 0;
}



#endif
