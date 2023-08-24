#include <user/drive.h>
#include <user/syscall.h>
#include <user/types.h>



_Bool drive_format(u32 index,const void* boot,u32 boot_length){
	return _syscall_drive_format(index,boot,boot_length);
}



_Bool drive_get_stats(u32 index,drive_stats_t* stats){
	return _syscall_drive_stats(index,stats,sizeof(drive_stats_t));
}
