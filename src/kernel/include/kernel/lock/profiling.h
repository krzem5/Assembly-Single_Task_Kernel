#ifndef _KERNEL_LOCK_PROFILING_H_
#define _KERNEL_LOCK_PROFILING_H_ 1
#include <kernel/types.h>



/****************************************************************************/



#ifdef KERNEL_RELEASE
#define LOCK_PROFILING_DATA
#define lock_profiling_init(lock)
#else
#define LOCK_PROFILING_DATA __lock_profiling_data_t __lock_profiling_data;
#define lock_profiling_init(lock) __lock_profiling_init(&((lock)->__lock_profiling_data));
#endif



typedef struct ___LOCK_PROFILING_DATA{
	u64 alloc_address;
} __lock_profiling_data_t;



void __lock_profiling_init(__lock_profiling_data_t* out);



/****************************************************************************/



#define LOCK_PROFILING_MAX_LOCK_TYPES_SHIFT 12
#define LOCK_PROFILING_MAX_LOCK_TYPES (1<<LOCK_PROFILING_MAX_LOCK_TYPES_SHIFT)
#define LOCK_PROFILING_EARLY_LOCK_TYPES 5

#define LOCK_PROFILING_MAX_NESTED_LOCKS 16



typedef struct _LOCK_LOCAL_PROFILING_DATA{
	KERNEL_ATOMIC u64 count;
	KERNEL_ATOMIC u64 ticks;
	KERNEL_ATOMIC u64 max_ticks;
} lock_local_profiling_data_t;



typedef struct _LOCK_PROFILING_TYPE_DESCRIPTOR{
	const char* func;
	u32 line;
	const char* arg;
} lock_profiling_type_descriptor_t;



typedef struct _LOCK_PROFILING_DATA_DESCRIPTOR{
	const struct _LOCK_PROFILING_DATA_DESCRIPTOR* next;
	const char* func;
	u32 line;
	const char* arg;
	lock_local_profiling_data_t data[];
} lock_profiling_data_descriptor_t;



typedef struct _LOCK_PROFILING_THREAD_DATA{
	void* stack[LOCK_PROFILING_MAX_NESTED_LOCKS];
	u16 id_stack[LOCK_PROFILING_MAX_NESTED_LOCKS];
	u64 stack_size;
} lock_profiling_thread_data_t;



extern const lock_profiling_type_descriptor_t* lock_profiling_type_descriptors;
extern const lock_profiling_data_descriptor_t* lock_profiling_data_descriptor_head;



u16 __lock_profiling_alloc_type(const char* func,u32 line,const char* arg,u16* out);



lock_local_profiling_data_t* __lock_profiling_alloc_data(const char* func,u32 line,const char* arg,u16 offset,u64* ptr);



void __lock_profiling_enable_dependency_graph(void);



void __lock_profiling_init_thread_data(lock_profiling_thread_data_t* out);



void __lock_profiling_push_lock(void* lock,u16 id,const char* func,u32 line);



void __lock_profiling_pop_lock(void* lock,u16 id,const char* func,u32 line);



#endif
