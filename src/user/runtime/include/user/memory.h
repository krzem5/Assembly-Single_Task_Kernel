#ifndef _USER_MEMORY_H_
#define _USER_MEMORY_H_ 1
#include <user/types.h>



#define MEMORY_FLAG_LARGE 1
#define MEMORY_FLAG_EXTRA_LARGE 2

#define MEMORY_RANGE_TYPE_NORMAL 1
#define MEMORY_RANGE_TYPE_UNUSABLE 2
#define MEMORY_RANGE_TYPE_ACPI_TABLES 3
#define MEMORY_RANGE_TYPE_ACPI_NVS 4
#define MEMORY_RANGE_TYPE_BAD_MEMORY 5



typedef struct _MEMORY_STATS{
	u64 counter_total;
	u64 counter_driver_ahci;
	u64 counter_driver_i82540;
	u64 counter_kernel_stack;
	u64 counter_kfs;
	u64 counter_kmm;
	u64 counter_network;
	u64 counter_umm;
	u64 counter_user;
	u64 counter_user_stack;
	u64 counter_vmm;
} memory_stats_t;



typedef struct _MEMORY_RANGE{
	u64 base_address;
	u64 length;
	u32 type;
} memory_range_t;



extern u32 memory_range_count;
extern const memory_range_t* memory_ranges;



void* memory_map(u64 length,u8 flags);



_Bool memory_unmap(void* address,u64 length);



_Bool memory_stats(memory_stats_t* out);



#endif
