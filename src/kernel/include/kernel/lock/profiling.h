#ifndef _KERNEL_LOCK_PROFILING_H_
#define _KERNEL_LOCK_PROFILING_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/_profiling_overload.h>
#include <kernel/types.h>



typedef struct _LOCK_PROFILING_TYPE_DESCRIPTOR{
	const char* func;
	u32 line;
	handle_t handle;
	u16 id;
} lock_profiling_type_descriptor_t;



typedef struct _LOCK_PROFILING_DATA_DESCRIPTOR{
	const char* func;
	u32 line;
	handle_t handle;
	lock_local_profiling_data_t data[];
} lock_profiling_data_descriptor_t;



extern handle_type_t HANDLE_TYPE_LOCK_TYPE;
extern handle_type_t HANDLE_TYPE_LOCK_DATA;



void lock_profiling_init(void);



#endif
