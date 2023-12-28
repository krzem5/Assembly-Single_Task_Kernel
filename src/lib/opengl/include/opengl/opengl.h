#ifndef _OPENGL_OPENGL_H_
#define _OPENGL_OPENGL_H_ 1
#include <sys/types.h>



typedef u64 opengl_driver_instance_t;



typedef struct _OPENGL_DRIVER_INSTANCE_DATA{
	u16 opengl_version;
	char driver_name[32];
	char renderer_name[64];
} opengl_driver_instance_data_t;



typedef u64 opengl_state_t;



void opengl_init(void);



opengl_state_t opengl_create_state(u16 min_version);



#endif
