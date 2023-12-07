#include <kernel/cpu/cpu.h>
#include <kernel/cpu/local.h>
#include <kernel/handle/handle.h>
#include <kernel/kernel.h>
#include <kernel/lock/profiling.h>
#include <kernel/lock/spinlock.h>
#include <kernel/log/log.h>
#include <kernel/memory/omm.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/mp/thread.h>
#include <kernel/types.h>
#include <kernel/util/util.h>
#define KERNEL_LOG_NAME "lock_profiling"



static pmm_counter_descriptor_t _lock_profiling_pmm_counter=_PMM_COUNTER_INIT_STRUCT("lock_profiling");
static spinlock_t _lock_profiling_data_lock=SPINLOCK_INIT_STRUCT;
static KERNEL_ATOMIC u16 KERNEL_NOBSS _lock_next_type_id=0;
static lock_profiling_type_descriptor_t* _lock_profiling_type_descriptors=NULL;
static lock_profiling_type_descriptor_t KERNEL_EARLY_WRITE _lock_profiling_early_types[LOCK_PROFILING_EARLY_LOCK_TYPES]={
	{
		.func="<unknown>",
		.line=0
	}
};
static lock_profiling_thread_data_t _early_lock_profiling_data={
	.stack_size=0
};
static CPU_LOCAL_DATA(lock_profiling_thread_data_t,_lock_profiling_cpu_local_data);
static KERNEL_ATOMIC u64* _lock_profiling_dependency_matrix=NULL;

KERNEL_PUBLIC const lock_profiling_type_descriptor_t* lock_profiling_type_descriptors=NULL;
KERNEL_PUBLIC const lock_profiling_data_descriptor_t* lock_profiling_data_descriptor_head=NULL;



static KERNEL_INLINE lock_profiling_thread_data_t* _get_lock_data(void){
	if (!_lock_profiling_cpu_local_data){
		return &_early_lock_profiling_data;
	}
#if KERNEL_DISABLE_ASSERT==0
	if (CPU_HEADER_DATA->current_thread){
		return &(CPU_HEADER_DATA->current_thread->__lock_profiling_data);
	}
#endif
	return CPU_LOCAL(_lock_profiling_cpu_local_data);
}



static KERNEL_INLINE u32 _get_edge_index(u16 from,u16 to){
	return (from<<LOCK_PROFILING_MAX_LOCK_TYPES_SHIFT)|to;
}



KERNEL_PUBLIC u16 __lock_profiling_alloc_type(const char* func,u32 line,const char* arg,u16* out){
	if (((u16)((*out)+1))>1){
		return *out;
	}
	u16 expected=0;
	if (__atomic_compare_exchange_n(out,&expected,0xffff,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)){
		*out=__atomic_add_fetch(&_lock_next_type_id,1,__ATOMIC_SEQ_CST);
		if (_lock_profiling_type_descriptors){
			goto _skip_alloc;
		}
		u64 data=pmm_alloc(pmm_align_up_address(LOCK_PROFILING_MAX_LOCK_TYPES*sizeof(lock_profiling_type_descriptor_t))>>PAGE_SIZE_SHIFT,&_lock_profiling_pmm_counter,0);
		if (!data){
			goto _skip_alloc;
		}
		_lock_profiling_type_descriptors=(void*)(data+VMM_HIGHER_HALF_ADDRESS_OFFSET);
		lock_profiling_type_descriptors=_lock_profiling_type_descriptors;
		if (*out!=LOCK_PROFILING_EARLY_LOCK_TYPES){
			panic("LOCK_PROFILING_EARLY_LOCK_TYPES too large");
		}
		for (u16 i=0;i<LOCK_PROFILING_EARLY_LOCK_TYPES;i++){
			*(_lock_profiling_type_descriptors+i)=*(_lock_profiling_early_types+i);
		}
_skip_alloc:
		if (!_lock_profiling_type_descriptors&&*out>=LOCK_PROFILING_EARLY_LOCK_TYPES){
			panic("LOCK_PROFILING_EARLY_LOCK_TYPES too small");
		}
		lock_profiling_type_descriptor_t* lock_profiling_type_descriptor=(_lock_profiling_type_descriptors?_lock_profiling_type_descriptors:_lock_profiling_early_types)+(*out);
		lock_profiling_type_descriptor->func=func;
		lock_profiling_type_descriptor->line=line;
		lock_profiling_type_descriptor->arg=arg;
	}
	SPINLOOP(*out==0xffff);
	return *out;
}



KERNEL_PUBLIC lock_local_profiling_data_t* __lock_profiling_alloc_data(const char* func,u32 line,const char* arg,u16 offset,u64* ptr){
	u64 expected=0;
	if (__atomic_compare_exchange_n(ptr,&expected,1,0,__ATOMIC_SEQ_CST,__ATOMIC_SEQ_CST)){
		u64 data=pmm_alloc(pmm_align_up_address(LOCK_PROFILING_MAX_LOCK_TYPES*sizeof(lock_local_profiling_data_t))>>PAGE_SIZE_SHIFT,&_lock_profiling_pmm_counter,0);
		if (!data){
			return NULL;
		}
		data+=VMM_HIGHER_HALF_ADDRESS_OFFSET;
		lock_profiling_data_descriptor_t* descriptor=(void*)data;
		descriptor->func=func;
		descriptor->line=line;
		descriptor->arg=arg;
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



void __lock_profiling_enable_dependency_graph(void){
	_lock_profiling_dependency_matrix=(void*)(pmm_alloc(pmm_align_up_address(1<<((LOCK_PROFILING_MAX_LOCK_TYPES_SHIFT<<1)-3))>>PAGE_SIZE_SHIFT,&_lock_profiling_pmm_counter,0)+VMM_HIGHER_HALF_ADDRESS_OFFSET);
}



void __lock_profiling_init_thread_data(lock_profiling_thread_data_t* out){
	out->stack_size=0;
}



KERNEL_PUBLIC void __lock_profiling_push_lock(void* lock,u16 id,const char* func,u32 line){
	if (!_lock_profiling_dependency_matrix){
		return;
	}
	lock_profiling_thread_data_t* data=_get_lock_data();
	if (data->stack_size==LOCK_PROFILING_MAX_NESTED_LOCKS){
		log("\x1b[1m\x1b[38;2;41;137;255m%s(%u): Lock stack too large\x1b[0m\n",func,line);
		return;
	}
	data->stack[data->stack_size]=lock;
	data->id_stack[data->stack_size]=id;
	data->stack_size++;
	for (u64 i=0;i<data->stack_size-1;i++){
		if (data->id_stack[i]==id&&data->stack[i]!=lock){
			continue;
		}
		u32 edge=_get_edge_index(data->id_stack[i],id);
		_lock_profiling_dependency_matrix[edge>>6]|=1ull<<(edge&63);
		u32 inverse_edge=_get_edge_index(id,data->id_stack[i]);
		if (_lock_profiling_dependency_matrix[inverse_edge>>6]&(1ull<<(inverse_edge&63))){
			const lock_profiling_type_descriptor_t* src=lock_profiling_type_descriptors+data->id_stack[i];
			const lock_profiling_type_descriptor_t* dst=lock_profiling_type_descriptors+id;
			log("\x1b[1m\x1b[38;2;41;137;255m%s(%u): Lock '%s(%u)' deadlocked by '%s(%u)'\x1b[0m\n",func,line,src->func,src->line,dst->func,dst->line);
		}
	}
}



#pragma GCC optimize ("no-tree-loop-distribute-patterns") // disable memmove
KERNEL_PUBLIC void __lock_profiling_pop_lock(void* lock,u16 id,const char* func,u32 line){
	if (!_lock_profiling_dependency_matrix){
		return;
	}
	lock_profiling_thread_data_t* data=_get_lock_data();
	for (u64 i=data->stack_size;i;){
		i--;
		if (data->stack[i]==lock&&data->id_stack[i]==id){
			data->stack_size--;
			for (;i<data->stack_size;i++){
				data->stack[i]=data->stack[i+1];
				data->id_stack[i]=data->id_stack[i+1];
			}
			return;
		}
	}
	log("\x1b[1m\x1b[38;2;41;137;255m%s(%u): Lock '%p (%u)' not acquired in this context\x1b[0m\n",func,line,lock,id);
}
