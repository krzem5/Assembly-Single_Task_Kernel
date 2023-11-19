#ifndef _KERNEL_SPINLOCK_SPINLOCK_H_
#define _KERNEL_SPINLOCK_SPINLOCK_H_ 1
#include <kernel/types.h>



#define SPINLOCK_INIT_STRUCT (0)



typedef u32 spinlock_t;



#if KERNEL_DISABLE_ASSERT==0
#include <kernel/clock/clock.h>
#include <kernel/kernel.h>



#define _spinlock_profile_setup_function(func,lock) \
	do{ \
		static u16 __spinlock_id=0; \
		static const spinlock_profiling_setup_descriptor_t __spinlock_profiling_setup_descriptor={ \
			__func__, \
			__LINE__, \
			&__spinlock_id \
		}; \
		static const spinlock_profiling_setup_descriptor_t*const __attribute__((used,section(".spinlock_setup"))) __spinlock_profiling_setup_descriptor_ptr=&__spinlock_profiling_setup_descriptor; \
		(func)(lock); \
		(*(lock))|=__spinlock_id<<16; \
	} while (0)
#define _spinlock_profile_function(func,lock) \
	do{ \
		static spinlock_profiling_data_t* __spinlock_profiling_data=NULL; \
		static const spinlock_profiling_descriptor_t __spinlock_profiling_descriptor={ \
			__func__, \
			__LINE__, \
			&__spinlock_profiling_data \
		}; \
		static const spinlock_profiling_descriptor_t*const __attribute__((used,section(".spinlock"))) __spinlock_profiling_descriptor_ptr=&__spinlock_profiling_descriptor; \
		u64 __start_ticks=clock_get_ticks(); \
		(func)(lock); \
		u64 __end_ticks=clock_get_ticks(); \
		if (__spinlock_profiling_data){ \
			spinlock_profiling_data_t* __local_profiling_data=__spinlock_profiling_data+((*(lock))>>16); \
			u64 __ticks=__end_ticks-__start_ticks; \
			__local_profiling_data->ticks+=__ticks; \
			__local_profiling_data->count++; \
			if (__ticks>__local_profiling_data->max_ticks){ \
				__local_profiling_data->max_ticks=__ticks; \
			} \
		} \
	} while (0)



#define spinlock_init(lock) _spinlock_profile_setup_function(spinlock_init,(lock))
#define spinlock_acquire_exclusive(lock) _spinlock_profile_function(spinlock_acquire_exclusive,(lock))
#define spinlock_acquire_shared(lock) _spinlock_profile_function(spinlock_acquire_shared,(lock))
#endif



typedef struct _SPINLOCK_PROFILING_DATA{
	KERNEL_ATOMIC u64 count;
	KERNEL_ATOMIC u64 ticks;
	KERNEL_ATOMIC u64 max_ticks;
} spinlock_profiling_data_t;



typedef struct _SPINLOCK_PROFILING_SETUP_DESCRIPTOR{
	const char* func;
	u32 line;
	u16* id;
} spinlock_profiling_setup_descriptor_t;



typedef struct _SPINLOCK_PROFILING_DESCRIPTOR{
	const char* func;
	u32 line;
	spinlock_profiling_data_t** data;
} spinlock_profiling_descriptor_t;



void spinlock_profiling_init(void);



const spinlock_profiling_setup_descriptor_t*const* spinlock_profiling_get_setup_descriptors(u32* count);



const spinlock_profiling_descriptor_t*const* spinlock_profiling_get_descriptors(u32* count);



void (spinlock_init)(spinlock_t* out);



void (spinlock_acquire_exclusive)(spinlock_t* lock);



void spinlock_release_exclusive(spinlock_t* lock);



void (spinlock_acquire_shared)(spinlock_t* lock);



void spinlock_release_shared(spinlock_t* lock);



_Bool spinlock_is_held(spinlock_t* lock);



#endif
