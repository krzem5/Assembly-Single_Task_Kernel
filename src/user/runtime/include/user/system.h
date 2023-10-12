#ifndef _USER_SYSTEM_H_
#define _USER_SYSTEM_H_ 1



#define SYSTEM_WAKEUP_TYPE_UNKNOWN 0
#define SYSTEM_WAKEUP_TYPE_POWER_SWITCH 1
#define SYSTEM_WAKEUP_TYPE_AC_POWER 2

#define SHUTDOWN_FLAG_RESTART 1



typedef struct _SYSTEM_BIOS_DATA{
	const char* bios_vendor;
	const char* bios_version;
	const char* manufacturer;
	const char* product;
	const char* version;
	const char* serial_number;
	u8 uuid[16];
	u8 wakeup_type;
} system_bios_data_t;



extern const system_bios_data_t* system_bios_data;



void system_shutdown(u8 flags);



#endif
