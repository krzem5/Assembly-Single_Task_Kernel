#ifndef _SYS_HANDLE_HANDLE_H_
#define _SYS_HANDLE_HANDLE_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



typedef u64 sys_handle_t;



sys_error_t __attribute__((access(write_only,2,3),nonnull)) sys_handle_get_name(sys_handle_t handle,void* buffer,u32 buffer_size);



#endif
