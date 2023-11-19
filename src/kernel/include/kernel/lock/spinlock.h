#ifndef _KERNEL_SPINLOCK_SPINLOCK_H_
#define _KERNEL_SPINLOCK_SPINLOCK_H_ 1
#include <kernel/types.h>



#define SPINLOCK_INIT_STRUCT (0)



typedef u32 spinlock_t;



#if KERNEL_DISABLE_ASSERT==0
#include <kernel/clock/clock.h>



#define _spinlock_profile_setup_function(func,lock) \
	do{ \
		static u16 __spinlock_id=0; \
		static const spinlock_setup_descriptor_t __spinlock_setup_descriptor={ \
			__func__, \
			__LINE__, \
			&__spinlock_id \
		}; \
		static const spinlock_setup_descriptor_t*const __attribute__((used,section(".spinlock_setup"))) __spinlock_setup_descriptor_ptr=&__spinlock_setup_descriptor; \
		(func)(lock); \
		*(lock)|=__spinlock_id<<16; \
	} while (0)
#define _spinlock_profile_function(func,lock) \
	do{ \
		static spinlock_profiling_data_t* __spinlock_perf_data=NULL; \
		static const spinlock_profiling_descriptor_t __spinlock_descriptor={ \
			__func__, \
			__LINE__, \
			&__spinlock_perf_data \
		}; \
		static const spinlock_profiling_descriptor_t*const __attribute__((used,section(".spinlock"))) __spinlock_descriptor_ptr=&__spinlock_descriptor; \
		u64 __start_ticks=clock_get_ticks(); \
		(func)(lock); \
		u64 __end_ticks=clock_get_ticks(); \
		if (__spinlock_perf_data){ \
			(__spinlock_perf_data+((*(lock))>>16))->time+=clock_ticks_to_time(__end_ticks-__start_ticks); \
			(__spinlock_perf_data+((*(lock))>>16))->count++; \
		} \
	} while (0)



#define spinlock_init(lock) _spinlock_profile_setup_function(spinlock_init,(lock))
#define spinlock_acquire_exclusive(lock) _spinlock_profile_function(spinlock_acquire_exclusive,(lock))
#define spinlock_acquire_shared(lock) _spinlock_profile_function(spinlock_acquire_shared,(lock))
#endif



typedef struct _SPINLOCK_PROFILING_DATA{
	KERNEL_ATOMIC u64 count;
	KERNEL_ATOMIC u64 time;
} spinlock_profiling_data_t;



typedef struct _SPINLOCK_SETUP_DESCRIPTOR{
	const char* func;
	u32 line;
	u16* id;
} spinlock_setup_descriptor_t;



typedef struct _SPINLOCK_PROFILING_DESCRIPTOR{
	const char* func;
	u32 line;
	spinlock_profiling_data_t** data;
} spinlock_profiling_descriptor_t;



void spinlock_profiling_init(void);



void (spinlock_init)(spinlock_t* out);



void (spinlock_acquire_exclusive)(spinlock_t* lock);



void spinlock_release_exclusive(spinlock_t* lock);



void (spinlock_acquire_shared)(spinlock_t* lock);



void spinlock_release_shared(spinlock_t* lock);



_Bool spinlock_is_held(spinlock_t* lock);



#endif
