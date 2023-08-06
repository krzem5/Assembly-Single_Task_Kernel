#ifndef _KERNEL_BIOS_BIOS_H_
#define _KERNEL_BIOS_BIOS_H_ 1
#include <kernel/types.h>



#define BIOS_DATA_WAKEUP_TYPE_UNKNOWN 0
#define BIOS_DATA_WAKEUP_TYPE_POWER_SWITCH 1
#define BIOS_DATA_WAKEUP_TYPE_AC_POWER 2



typedef struct _BIOS_DATA{
	char bios_vendor[65];
	char bios_version[65];
	char manufacturer[65];
	char product[65];
	char version[65];
	char serial_number[65];
	u8 uuid[16];
	u8 wakeup_type;
} bios_data_t;



extern bios_data_t bios_data;



void bios_get_system_data(void);



#endif
