#ifndef _KERNEL_LOCK__PROFILING_OVERLOAD_H_
#define _KERNEL_LOCK__PROFILING_OVERLOAD_H_ 1
#include <kernel/kernel.h>



#define __lock_override_type_function(func,lock) \
	do{ \
		static u16 KERNEL_NOBSS __lock_id=0; \
		(func)(lock); \
		(*(lock))|=__lock_profiling_alloc_type(__func__,__LINE__,&__lock_id)<<16; \
	} while (0)
#define __lock_override_data_function(func,lock) \
	do{ \
		extern u64 clock_get_ticks(void); \
		static u64 KERNEL_NOBSS __lock_profiling_data=0; \
		lock_local_profiling_data_t* __local_profiling_data=__lock_profiling_alloc_data(__func__,__LINE__,(*(lock))>>16,&__lock_profiling_data); \
		u64 __start_ticks=clock_get_ticks(); \
		(func)(lock); \
		u64 __end_ticks=clock_get_ticks(); \
		if (__local_profiling_data){ \
			u64 __ticks=__end_ticks-__start_ticks; \
			__local_profiling_data->ticks+=__ticks; \
			__local_profiling_data->count++; \
			if (__ticks>__local_profiling_data->max_ticks){ \
				__local_profiling_data->max_ticks=__ticks; \
			} \
		} \
	} while (0)



typedef struct _LOCK_LOCAL_PROFILING_DATA{
	KERNEL_ATOMIC u64 count;
	KERNEL_ATOMIC u64 ticks;
	KERNEL_ATOMIC u64 max_ticks;
} lock_local_profiling_data_t;



u16 __lock_profiling_alloc_type(const char* func,u32 line,u16* out);



lock_local_profiling_data_t* __lock_profiling_alloc_data(const char* func,u32 line,u16 offset,u64* ptr);



#endif
