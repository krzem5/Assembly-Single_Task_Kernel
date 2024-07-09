#ifndef _SYS_MUTEX_MUTEX_H_
#define _SYS_MUTEX_MUTEX_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



typedef u64 sys_mutex_t;



sys_mutex_t sys_mutex_create(void);



sys_error_t sys_mutex_delete(sys_mutex_t mutex);



sys_error_t sys_mutex_get_holder(sys_mutex_t mutex);



sys_error_t sys_mutex_acquire(sys_mutex_t mutex);



sys_error_t sys_mutex_release(sys_mutex_t mutex);



#endif
