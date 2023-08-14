#ifndef _USER_SYSTEM_H_
#define _USER_SYSTEM_H_ 1



#define SYSTEM_WAKEUP_TYPE_UNKNOWN 0
#define SYSTEM_WAKEUP_TYPE_POWER_SWITCH 1
#define SYSTEM_WAKEUP_TYPE_AC_POWER 2

#define SHUTDOWN_FLAG_RESTART 1



typedef struct _SYSTEM_DATA{
	char bios_vendor[65];
	char bios_version[65];
	char manufacturer[65];
	char product[65];
	char version[65];
	char serial_number[65];
	u8 uuid[16];
	u8 wakeup_type;
} system_data_t;



extern system_data_t system_data;



void system_init(void);



void system_shutdown(u8 flags);



void system_poll(void);



#endif
