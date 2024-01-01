#ifndef _SYS_SYSTEM_SYSTEM_H_
#define _SYS_SYSTEM_SYSTEM_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



#define SYS_SYSTEM_SHUTDOWN_FLAG_RESTART 1



void __sys_init(void);



sys_error_t sys_system_shutdown(u32 flags);



#endif
