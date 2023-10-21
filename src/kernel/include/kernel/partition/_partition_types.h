#ifndef _KERNEL_PARTITION__PARTITION_TYPES_H_
#define _KERNEL_PARTITION__PARTITION_TYPES_H_ 1
#include <kernel/types.h>



struct _DRIVE;



typedef struct _PARTITION_DESCRIPTOR{
	const char* name;
	_Bool (*load_callback)(struct _DRIVE*);
} partition_descriptor_t;



#endif
