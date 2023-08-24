#ifndef _USER_PARTITION_H_
#define _USER_PARTITION_H_ 1
#include <user/types.h>



#define PARTITION_FLAG_BOOT 1
#define PARTITION_FLAG_HALF_INSTALLED 2
#define PARTITION_FLAG_PREVIOUS_BOOT 4

#define PARTITION_TYPE_DRIVE 0
#define PARTITION_TYPE_EMPTY 1
#define PARTITION_TYPE_ISO9660 2
#define PARTITION_TYPE_KFS 3



typedef struct _PARTITION{
	u8 flags;
	u8 type;
	u8 index;
	u64 first_block_index;
	u64 last_block_index;
	const char* name;
	u32 drive_index;
} partition_t;



extern u32 partition_count;
extern u32 partition_boot_index;
extern const partition_t* partitions;



#endif
