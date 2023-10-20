#ifndef _USER_HANDLE_H_
#define _USER_HANDLE_H_ 1
#include <user/types.h>



typedef u16 handle_type_t;



typedef struct _HANDLE_TYPE_DATA{
	char name[16];
	u64 count;
	u64 active_count;
} handle_type_data_t;



handle_type_t handle_get_type_by_name(const char* name);



handle_type_t handle_get_type_count(void);



_Bool handle_get_type_data(handle_type_t handle_type,handle_type_data_t* out);



#endif
