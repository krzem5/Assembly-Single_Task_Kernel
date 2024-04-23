#ifndef _KERNEL_PARTITION__PARTITION_TYPES_H_
#define _KERNEL_PARTITION__PARTITION_TYPES_H_ 1
#include <kernel/handle/handle.h>



struct _DRIVE;



typedef struct _PARTITION_TABLE_DESCRIPTOR_CONFIG{
	const char* name;
	bool (*load_callback)(struct _DRIVE*);
	bool (*format_callback)(struct _DRIVE*);
} partition_table_descriptor_config_t;



typedef struct _PARTITION_TABLE_DESCRIPTOR{
	const partition_table_descriptor_config_t* config;
	handle_t handle;
} partition_table_descriptor_t;



#endif
