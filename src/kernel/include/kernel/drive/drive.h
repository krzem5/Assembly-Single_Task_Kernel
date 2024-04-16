#ifndef _KERNEL_DRIVE_DRIVE_H_
#define _KERNEL_DRIVE_DRIVE_H_ 1
#include <kernel/handle/handle.h>
#include <kernel/memory/smm.h>
#include <kernel/partition/_partition_types.h>
#include <kernel/types.h>



#define DRIVE_OFFSET_FLAG_WRITE 0x8000000000000000ull
#define DRIVE_OFFSET_MASK 0x7fffffffffffffffull

#define DRIVE_TYPE_FLAG_READ_ONLY 1
#define DRIVE_TYPE_FLAG_NO_CACHE 2



struct _DRIVE;



typedef struct _DRIVE_TYPE{
	const char* name;
	u32 flags;
	u64 (*io_callback)(struct _DRIVE*,u64,u64,u64);
} drive_type_t;



typedef struct _DRIVE_CONFIG{
	const drive_type_t* type;
	u32 controller_index;
	u32 device_index;
	string_t* serial_number;
	string_t* model_number;
	u64 block_count;
	u64 block_size;
	void* extra_data;
} drive_config_t;



typedef struct _DRIVE{
	handle_t handle;
	const drive_type_t* type;
	u8 block_size_shift;
	u16 controller_index;
	u16 device_index;
	string_t* serial_number;
	string_t* model_number;
	u64 block_count;
	u64 block_size;
	void* extra_data;
	partition_table_descriptor_t* partition_table_descriptor;
} drive_t;



typedef struct _DRIVE_USER_DATA{
	char type[64];
	u16 controller_index;
	u16 device_index;
	char serial_number[256];
	char model_number[256];
	u64 block_count;
	u64 block_size;
	char partition_table_type[64];
} drive_user_data_t;



extern handle_type_t drive_handle_type;



drive_t* drive_create(const drive_config_t* config);



u64 drive_read(drive_t* drive,u64 offset,void* buffer,u64 size);



u64 drive_write(drive_t* drive,u64 offset,const void* buffer,u64 size);



#endif
