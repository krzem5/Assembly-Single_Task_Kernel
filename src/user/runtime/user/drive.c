#include <user/drive.h>
#include <user/syscall.h>
#include <user/types.h>



_Bool drive_get(u32 index,drive_t* out){
	return _syscall_drive_get(index,out,sizeof(drive_t));
}
