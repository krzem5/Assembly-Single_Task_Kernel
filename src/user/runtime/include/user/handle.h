#ifndef _USER_HANDLE_H_
#define _USER_HANDLE_H_ 1
#include <user/types.h>



#define HANDLE_INVALID 0xffffffffffffffffull

#define HANDLE_FOREACH(iterator,name) for (handle_iter_start((name),(iterator));(iterator)->handle!=HANDLE_INVALID;handle_iter_next((iterator)))



typedef u16 handle_type_t;



typedef u64 handle_t;



typedef struct _HANDLE_ITERATOR{
	handle_type_t type;
	handle_t handle;
} handle_iterator_t;



typedef struct _HANDLE_DATA{
	char name[64];
	u64 count;
	u64 active_count;
	u16 type;
} handle_data_t;



handle_type_t handle_get_type(const char* name);



void handle_iter_start(const char* name,handle_iterator_t* out);



void handle_iter_next(handle_iterator_t* iterator);



_Bool handle_get_data(handle_t handle,handle_data_t* out);



#endif
