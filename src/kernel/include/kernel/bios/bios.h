#ifndef _KERNEL_BIOS_BIOS_H_
#define _KERNEL_BIOS_BIOS_H_ 1
#include <kernel/memory/smm.h>
#include <kernel/types.h>



#define BIOS_DATA_WAKEUP_TYPE_UNKNOWN 0
#define BIOS_DATA_WAKEUP_TYPE_POWER_SWITCH 1
#define BIOS_DATA_WAKEUP_TYPE_AC_POWER 2



typedef struct _BIOS_DATA{
	string_t* bios_vendor;
	string_t* bios_version;
	string_t* manufacturer;
	string_t* product;
	string_t* version;
	string_t* serial_number;
	u8 uuid[16];
	string_t* uuid_str;
	u8 wakeup_type;
	string_t* wakeup_type_str;
} bios_data_t;



extern bios_data_t bios_data;



void bios_get_system_data(void);



#endif
