#ifndef _SYS2_CLOCK_CLOCK_H_
#define _SYS2_CLOCK_CLOCK_H_ 1
#include <sys2/types.h>



u64 sys2_clock_get_ticks(void);



double sys2_clock_get_time(void);



u64 sys2_clock_get_time_ns(void);



double sys2_clock_convert_ticks_to_time(u64 ticks);



u64 sys2_clock_convert_ticks_to_time_ns(u64 ticks);



u64 sys2_clock_get_frequency(void);



#endif
