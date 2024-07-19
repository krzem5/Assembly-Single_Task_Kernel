#ifndef _SYS_TIME_TIME_H_
#define _SYS_TIME_TIME_H_ 1
#include <sys/types.h>



#define SYS_TIME_TYPE_BOOT 0
#define SYS_TIME_TYPE_EARLY_INIT 1
#define SYS_TIME_TYPE_INIT 2



typedef struct _SYS_TIME{
	s16 years;
	u8 months;
	u8 days;
	u8 hours;
	u8 minutes;
	u8 seconds;
	u16 miliseconds;
	u16 microseconds;
	u16 nanoseconds;
} sys_time_t;



u64 sys_time_get(u32 type);



void sys_time_from_nanoseconds(s64 time,sys_time_t* out);



#endif
