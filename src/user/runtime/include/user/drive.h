#ifndef _USER_DRIVE_H_
#define _USER_DRIVE_H_ 1
#include <user/types.h>



#define DRIVE_FLAG_BOOT 1

#define DRIVE_TYPE_AHCI 0
#define DRIVE_TYPE_ATA 1
#define DRIVE_TYPE_ATAPI 2
#define DRIVE_TYPE_NVME 3



typedef struct _DRIVE_STATS{
	u64 root_block_count;
	u64 batc_block_count;
	u64 nda3_block_count;
	u64 nda2_block_count;
	u64 nda1_block_count;
	u64 nfda_block_count;
	u64 data_block_count;
} drive_stats_t;



typedef struct _DRIVE{
	u8 flags;
	u8 type;
	u8 index;
	const char* name;
	const char* serial_number;
	const char* model_number;
	u64 block_count;
	u64 block_size;
} drive_t;



extern u32 drive_count;
extern u32 drive_boot_index;
extern const drive_t* drives;



_Bool drive_format(u32 index,const void* boot,u32 boot_length);



_Bool drive_get_stats(u32 index,drive_stats_t* stats);



#endif
