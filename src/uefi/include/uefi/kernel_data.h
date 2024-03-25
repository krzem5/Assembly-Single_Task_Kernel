#ifndef _UEFI_KERNEL_DATA_H_
#define _UEFI_KERNEL_DATA_H_ 1
#include <stdint.h>



#define KERNEL_MEMORY_ADDRESS 0x100000
#define KERNEL_STACK_PAGE_COUNT 3



typedef struct _KERNEL_DATA{
	uint16_t mmap_size;
	struct{
		uint64_t base;
		uint64_t length;
	} mmap[42];
	uint64_t first_free_address;
	uint64_t rsdp_address;
	uint64_t smbios_address;
	uint64_t initramfs_address;
	uint64_t initramfs_size;
	uint8_t boot_fs_guid[16];
	uint8_t master_key[64];
	struct{
		uint16_t year;
		uint8_t month;
		uint8_t day;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
		uint32_t nanosecond;
		uint64_t measurement_offset;
	} date;
} kernel_data_t;



#endif
