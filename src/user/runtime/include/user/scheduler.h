#ifndef _USER_SCHEDULER_H_
#define _USER_SCHEDULER_H_ 1
#include <user/types.h>



typedef struct _SCHEDULER_STATS{
	u64 added_thread_count;
	u64 free_slot_count;
	u64 used_slot_count;
} scheduler_stats_t;



_Bool scheduler_get_stats(u32 cpu_index,scheduler_stats_t* out);



#endif
