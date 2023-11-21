#ifndef _KERNEL_SPINLOCK_PROFILING_H_
#define _KERNEL_SPINLOCK_PROFILING_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/lock/spinlock.h>
#include <kernel/types.h>



typedef struct _SPINLOCK_PROFILING_SETUP_DESCRIPTOR{
	const char* func;
	u32 line;
	handle_t handle;
	u16 id;
} spinlock_profiling_setup_descriptor_t;



typedef struct _SPINLOCK_PROFILING_DESCRIPTOR{
	const char* func;
	u32 line;
	handle_t handle;
	spinlock_profiling_data_t* data;
} spinlock_profiling_descriptor_t;



extern handle_type_t HANDLE_TYPE_SPINLOCK_TYPE;
extern handle_type_t HANDLE_TYPE_SPINLOCK_DATA;



void spinlock_profiling_init(void);



#endif
