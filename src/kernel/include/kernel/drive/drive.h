#ifndef _KERNEL_DRIVE_DRIVE_H_
#define _KERNEL_DRIVE_DRIVE_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/types.h>



#define DRIVE_TYPE_AHCI 0
#define DRIVE_TYPE_ATA 1
#define DRIVE_TYPE_ATAPI 2
#define DRIVE_TYPE_NVME 3

#define DRIVE_FLAG_BOOT 1

#define DRIVE_NAME_LENGTH 16
#define DRIVE_SERIAL_NUMBER_LENGTH 32
#define DRIVE_MODEL_NUMBER_LENGTH 64

#define DRIVE_OFFSET_FLAG_WRITE 0x8000000000000000ull
#define DRIVE_OFFSET_MASK 0x7fffffffffffffffull



typedef struct _DRIVE_CONFIG{
	u8 type;
	u8 flags;
	u8 _padding[6];
	char name[DRIVE_NAME_LENGTH];
	char serial_number[DRIVE_SERIAL_NUMBER_LENGTH];
	char model_number[DRIVE_MODEL_NUMBER_LENGTH];
	u64 block_count;
	u64 block_size;
	u64 (*read_write)(void*,u64,void*,u64);
	void* extra_data;
} drive_config_t;



typedef struct _DRIVE2{
	handle_t handle;
	u8 type;
	u8 flags;
	u8 block_size_shift;
	u8 _padding[5];
	char name[DRIVE_NAME_LENGTH];
	char serial_number[DRIVE_SERIAL_NUMBER_LENGTH];
	char model_number[DRIVE_MODEL_NUMBER_LENGTH];
	u64 block_count;
	u64 block_size;
	u64 (*read_write)(void*,u64,void*,u64);
	void* extra_data;
} drive2_t;



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
	struct _DRIVE* next;
	u8 type;
	u8 flags;
	u8 index;
	u8 block_size_shift;
	char name[16];
	char serial_number[32];
	char model_number[64];
	u64 block_count;
	u64 block_size;
	u64 (*read_write)(void*,u64,void*,u64);
	void* extra_data;
	drive_stats_t* stats;
} drive_t;



extern drive_t* drive_data;



drive2_t* drive_create(const drive_config_t* config);



void drive_add(const drive_t* drive);



#endif
