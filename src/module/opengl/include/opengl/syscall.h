#ifndef _OPENGL_SYSCALL_H_
#define _OPENGL_SYSCALL_H_ 1
#include <kernel/types.h>



typedef u64 opengl_user_driver_instance_t;



typedef struct _OPENGL_DRIVER_INSTANCE_DATA{
	u16 opengl_version;
	char driver_name[32];
	char renderer_name[64];
	char library[128];
} opengl_user_driver_instance_data_t;



typedef u64 opengl_user_state_t;



void opengl_syscall_init(void);



#endif
