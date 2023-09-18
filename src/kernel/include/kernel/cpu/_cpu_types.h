#ifndef _KERNEL_CPU__CPU_TYPES_H_
#define _KERNEL_CPU__CPU_TYPES_H_ 1
#include <kernel/types.h>



typedef struct _CPU_HEADER{
	u8 index;
	u8 _padding[7];
	u64 kernel_rsp;
	u64 user_rsp;
	struct _CPU_DATA* cpu_data;
} cpu_header_t;



#endif
