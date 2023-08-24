#ifndef _USER_MEMORY_H_
#define _USER_MEMORY_H_ 1
#include <user/types.h>



#define MEMORY_FLAG_LARGE 1
#define MEMORY_FLAG_EXTRA_LARGE 2



typedef struct _MEMORY_STATS{
	u64 counter_total;
	u64 counter_driver_ahci;
	u64 counter_driver_i82540;
	u64 counter_kernel_stack;
	u64 counter_kfs;
	u64 counter_kmm;
	u64 counter_network;
	u64 counter_pmm;
	u64 counter_umm;
	u64 counter_user;
	u64 counter_user_stack;
	u64 counter_vmm;
} memory_stats_t;



void* memory_map(u64 length,u8 flags);



_Bool memory_unmap(void* address,u64 length);



_Bool memory_stats(memory_stats_t* out);



#endif
