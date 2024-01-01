#ifndef _SYS_TIME_TIME_H_
#define _SYS_TIME_TIME_H_ 1
#include <sys/types.h>



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



void __sys_time_init(void);



u64 sys_time_get_boot_offset(void);



void sys_time_from_nanoseconds(s64 time,sys_time_t* out);



#endif
