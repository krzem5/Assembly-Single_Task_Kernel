#ifndef _SYS_CONTAINER_CONTAINER_H_
#define _SYS_CONTAINER_CONTAINER_H_ 1
#include <sys/error/error.h>
#include <sys/types.h>



typedef u64 sys_container_t;



sys_container_t sys_container_create(void);



sys_error_t sys_container_delete(sys_container_t container);



sys_error_t __attribute__((access(read_only,2,3),nonnull)) sys_container_add(sys_container_t container,const u64* handles,u64 handle_count);



sys_error_t __attribute__((access(write_only,3,4),nonnull)) sys_container_get(sys_container_t container,u64 offset,u64* handles,u64 handle_count);



#endif
