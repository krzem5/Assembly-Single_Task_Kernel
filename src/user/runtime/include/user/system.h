#ifndef _USER_SYSTEM_H_
#define _USER_SYSTEM_H_ 1
#include <user/types.h>



#define SYSTEM_STRING_BIOS_VENDOR 0
#define SYSTEM_STRING_BIOS_VERSION 1
#define SYSTEM_STRING_MANUFACTURER 2
#define SYSTEM_STRING_PRODUCT 3
#define SYSTEM_STRING_VERSION 4
#define SYSTEM_STRING_SERIAL_NUMBER 5
#define SYSTEM_STRING_UUID 6
#define SYSTEM_STRING_LAST_WAKEUP 7

#define SHUTDOWN_FLAG_RESTART 1



u32 system_get_string(u32 index,char* buffer,u32 size);



void system_shutdown(u8 flags);



#endif
