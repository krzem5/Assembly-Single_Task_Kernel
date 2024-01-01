#ifndef _SYS_CLOCK_CLOCK_H_
#define _SYS_CLOCK_CLOCK_H_ 1
#include <sys/types.h>



void __sys_clock_init(void);



u64 sys_clock_get_ticks(void);



double sys_clock_get_time(void);



u64 sys_clock_get_time_ns(void);



double sys_clock_convert_ticks_to_time(u64 ticks);



u64 sys_clock_convert_ticks_to_time_ns(u64 ticks);



u64 sys_clock_get_frequency(void);



#endif
