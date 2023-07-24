#ifndef _USER_PARTITION_H_
#define _USER_PARTITION_H_ 1
#include <user/types.h>



#define MAX_PARTITIONS 32



#define PARTITION_FLAG_PRESENT 1
#define PARTITION_FLAG_BOOT 2

#define PARTITION_TYPE_EMPTY_DRIVE 0
#define PARTITION_TYPE_EMPTY 1
#define PARTITION_TYPE_ISO9660 2
#define PARTITION_TYPE_GPT 3



typedef struct _PARTITION{
	u8 flags;
	u8 type;
	u8 index;
	u64 first_block_index;
	u64 last_block_index;
	char name[16];
	u32 drive_index;
} partition_t;



extern partition_t partitions[MAX_PARTITIONS];
extern u32 partition_count;
extern u32 partition_boot_index;



void partition_init(void);



#endif
