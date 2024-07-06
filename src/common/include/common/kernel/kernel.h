#ifndef _COMMON_KERNEL_KERNEL_H_
#define _COMMON_KERNEL_KERNEL_H_ 1
#include <common/types.h>



typedef struct _KERNEL_DATA{
	u16 mmap_size;
	struct{
		u64 base;
		u64 length;
	} mmap[42];
	u64 first_free_address;
	u64 rsdp_address;
	u64 smbios_address;
	u64 initramfs_address;
	u64 initramfs_size;
	u8 boot_fs_uuid[16];
	u8 master_key[64];
	struct{
		u16 year;
		u8 month;
		u8 day;
		u8 hour;
		u8 minute;
		u8 second;
		u32 nanosecond;
		u64 measurement_offset;
	} date;
} kernel_data_t;



#endif
