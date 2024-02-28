#ifndef _KERNEL_CLOCK_CLOCK_H_
#define _KERNEL_CLOCK_CLOCK_H_ 1
#include <kernel/types.h>



typedef u64 (*clock_source_calibration_callback_t)(void);



typedef struct _CLOCK_SOURCE{
	const char* name;
	clock_source_calibration_callback_t calibration_callback;
	const _Bool* is_stable;
} clock_source_t;



extern u64 clock_cpu_frequency;
extern u64 clock_conversion_factor;
extern u32 clock_conversion_shift;



void clock_add_source(const clock_source_t* source);



u64 clock_get_ticks(void);



u64 clock_get_time(void);



u64 clock_ticks_to_time(u64 ticks);



#endif
