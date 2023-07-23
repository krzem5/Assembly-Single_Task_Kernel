#ifndef _USER_DRIVE_H_
#define _USER_DRIVE_H_ 1
#include <user/types.h>



#define MAX_DRIVES 32



#define DRIVE_FLAG_PRESENT 1
#define DRIVE_FLAG_BOOT 2

#define DRIVE_TYPE_AHCI 0
#define DRIVE_TYPE_ATA 1
#define DRIVE_TYPE_ATAPI 2
#define DRIVE_TYPE_NVME 3



typedef struct _DRIVE{
	u8 flags;
	u8 type;
	u8 index;
	char name[16];
	char serial_number[32];
	char model_number[64];
	u64 block_count;
	u64 block_size;
} drive_t;



extern drive_t drives[MAX_DRIVES];
extern u32 drive_count;
extern u32 drive_boot_index;



void drive_init(void);



#endif
