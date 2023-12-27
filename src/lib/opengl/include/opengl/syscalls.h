#ifndef _OPENGL_SYSCALLS_H_
#define _OPENGL_SYSCALLS_H_ 1
#include <sys/types.h>



typedef u64 opengl_driver_instance_t;



typedef struct _OPENGL_DRIVER_INSTANCE_DATA{
	u16 opengl_version;
	char driver_name[32];
	char renderer_name[64];
} opengl_driver_instance_data_t;



_Bool opengl_syscalls_init(void);



opengl_driver_instance_t opengl_syscall_get_driver_instance(u16 min_version);



_Bool opengl_syscall_get_driver_instance_data(opengl_driver_instance_t instance,opengl_driver_instance_data_t* out);



#endif
