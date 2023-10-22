#ifndef _KERNEL_DRIVE_DRIVE_H_
#define _KERNEL_DRIVE_DRIVE_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/partition/_partition_types.h>
#include <kernel/types.h>



#define DRIVE_NAME_LENGTH 16
#define DRIVE_SERIAL_NUMBER_LENGTH 32
#define DRIVE_MODEL_NUMBER_LENGTH 64

#define DRIVE_OFFSET_FLAG_WRITE 0x8000000000000000ull
#define DRIVE_OFFSET_MASK 0x7fffffffffffffffull



typedef u64 (*drive_io_callback_t)(void*,u64,void*,u64);



typedef struct _DRIVE_TYPE{
	const char* name;
	drive_io_callback_t io_callback;
} drive_type_t;



typedef struct _DRIVE_CONFIG{
	const drive_type_t* type;
	char name[DRIVE_NAME_LENGTH];
	char serial_number[DRIVE_SERIAL_NUMBER_LENGTH];
	char model_number[DRIVE_MODEL_NUMBER_LENGTH];
	u64 block_count;
	u64 block_size;
	void* extra_data;
} drive_config_t;



typedef struct _DRIVE{
	handle_t handle;
	const drive_type_t* type;
	u8 block_size_shift;
	partition_descriptor_t* partition_descriptor;
	char name[DRIVE_NAME_LENGTH];
	char serial_number[DRIVE_SERIAL_NUMBER_LENGTH];
	char model_number[DRIVE_MODEL_NUMBER_LENGTH];
	u64 block_count;
	u64 block_size;
	void* extra_data;
} drive_t;



extern handle_type_t HANDLE_TYPE_DRIVE;



drive_t* drive_create(const drive_config_t* config);



u64 drive_read(drive_t* drive,u64 offset,void* buffer,u64 size);



u64 drive_write(drive_t* drive,u64 offset,const void* buffer,u64 size);



#endif
