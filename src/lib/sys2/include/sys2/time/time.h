#ifndef _SYS2_TIME_TIME_H_
#define _SYS2_TIME_TIME_H_ 1
#include <sys2/types.h>



typedef struct _SYS2_TIME{
	s16 years;
	u8 months;
	u8 days;
	u8 hours;
	u8 minutes;
	u8 seconds;
	u16 miliseconds;
	u16 microseconds;
	u16 nanoseconds;
} sys2_time_t;



void __sys2_time_init(void);



u64 sys2_time_get_boot_offset(void);



void sys2_time_from_nanoseconds(s64 time,sys2_time_t* out);



#endif
