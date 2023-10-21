#include <user/handle.h>
#include <user/syscall.h>



handle_type_t handle_get_type(const char* name){
	u32 length=0;
	for (;name[length];length++);
	return _syscall_handle_get_type(name,length);
}



void handle_iter_start(const char* name,handle_iterator_t* out){
	out->type=handle_get_type(name);
	out->handle=(out->type?_syscall_handle_get_handle(out->type,0):HANDLE_INVALID);
}



void handle_iter_next(handle_iterator_t* iterator){
	if (iterator->handle!=HANDLE_INVALID){
		// Increasing the handle 'type' bits by 1 has the same effect as increasing the index, as the handle returned by the syscall is >= the handle supplied
		iterator->handle=_syscall_handle_get_handle(iterator->type,iterator->handle+1);
	}
}



_Bool handle_get_data(handle_t handle,handle_data_t* out){
	return _syscall_handle_get_data(handle,out,sizeof(handle_data_t));
}
