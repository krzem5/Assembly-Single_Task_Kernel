#ifndef _KERNEL_BIOS_BIOS_H_
#define _KERNEL_BIOS_BIOS_H_ 1
#include <kernel/vfs/name.h>
#include <kernel/types.h>



#define BIOS_DATA_WAKEUP_TYPE_UNKNOWN 0
#define BIOS_DATA_WAKEUP_TYPE_POWER_SWITCH 1
#define BIOS_DATA_WAKEUP_TYPE_AC_POWER 2



typedef struct _BIOS_DATA{
	vfs_name_t* bios_vendor;
	vfs_name_t* bios_version;
	vfs_name_t* manufacturer;
	vfs_name_t* product;
	vfs_name_t* version;
	vfs_name_t* serial_number;
	u8 uuid[16];
	vfs_name_t* uuid_str;
	u8 wakeup_type;
	vfs_name_t* wakeup_type_str;
} bios_data_t;



extern bios_data_t bios_data;



void bios_get_system_data(void);



#endif
