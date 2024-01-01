#ifndef _SYS2_SYSTEM_SYSTEM_H_
#define _SYS2_SYSTEM_SYSTEM_H_ 1
#include <sys2/error/error.h>
#include <sys2/types.h>



#define SYS2_SYSTEM_SHUTDOWN_FLAG_RESTART 1



void __sys2_init(void);



sys2_error_t sys2_system_shutdown(u32 flags);



#endif
