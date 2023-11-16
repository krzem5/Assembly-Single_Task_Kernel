#ifndef _SYS_SYSTEM_H_
#define _SYS_SYSTEM_H_ 1
#include <sys/types.h>



#define SYS_SHUTDOWN_FLAG_RESTART 1



void sys_system_shutdown(u8 flags);



#endif
