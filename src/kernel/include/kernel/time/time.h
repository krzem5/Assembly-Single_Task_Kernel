#ifndef _KERNEL_TIME_TIME_H_
#define _KERNEL_TIME_TIME_H_ 1
#include <kernel/types.h>



extern u64 time_boot_offset;



s64 time_to_nanoseconds(s16 year,u8 month,u8 day,u8 hour,u8 minute,u8 second);



#endif
