#include <user/drive.h>
#include <user/syscall.h>
#include <user/types.h>



_Bool drive_get_data(u64 handle,drive_data_t* out){
	return _syscall_drive_get_data(handle,out,sizeof(drive_data_t));
}
