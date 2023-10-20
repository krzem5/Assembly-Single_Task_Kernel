#ifndef _KERNEL_DRIVE_DRIVE_H_
#define _KERNEL_DRIVE_DRIVE_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/partition/_partition_types.h>
#include <kernel/types.h>



#define DRIVE_TYPE_AHCI 0
#define DRIVE_TYPE_ATA 1
#define DRIVE_TYPE_ATAPI 2
#define DRIVE_TYPE_NVME 3
#define DRIVE_TYPE_INITRAMFS 4

#define DRIVE_NAME_LENGTH 16
#define DRIVE_SERIAL_NUMBER_LENGTH 32
#define DRIVE_MODEL_NUMBER_LENGTH 64

#define DRIVE_OFFSET_FLAG_WRITE 0x8000000000000000ull
#define DRIVE_OFFSET_MASK 0x7fffffffffffffffull



typedef u64 (*drive_io_callback_t)(void*,u64,void*,u64);



typedef struct _DRIVE_CONFIG{
	u8 type;
	u8 flags;
	u8 _padding[6];
	char name[DRIVE_NAME_LENGTH];
	char serial_number[DRIVE_SERIAL_NUMBER_LENGTH];
	char model_number[DRIVE_MODEL_NUMBER_LENGTH];
	u64 block_count;
	u64 block_size;
	drive_io_callback_t read_write;
	void* extra_data;
} drive_config_t;



typedef struct _DRIVE{
	handle_t handle;
	u8 type;
	u8 flags;
	u8 block_size_shift;
	partition_type_t partition_type;
	u32 _next_partition_index;
	char name[DRIVE_NAME_LENGTH];
	char serial_number[DRIVE_SERIAL_NUMBER_LENGTH];
	char model_number[DRIVE_MODEL_NUMBER_LENGTH];
	u64 block_count;
	u64 block_size;
	drive_io_callback_t read_write;
	void* extra_data;
} drive_t;



extern handle_type_t HANDLE_TYPE_DRIVE;



drive_t* drive_create(const drive_config_t* config);



u64 drive_read(drive_t* drive,u64 offset,void* buffer,u64 size);



u64 drive_write(drive_t* drive,u64 offset,const void* buffer,u64 size);



#endif
