#include <user/drive.h>
#include <user/syscall.h>
#include <user/types.h>



drive_t __attribute__((section(".bss"))) drives[MAX_DRIVES];
u32 drive_count;
u32 drive_boot_index=0xffffffff;



void drive_init(void){
	if (drive_boot_index!=0xffffffff){
		return;
	}
	for (u32 i=0;i<MAX_DRIVES;i++){
		drives[i].flags=0;
	}
	drive_count=_syscall_drive_list_length();
	if (drive_count>MAX_DRIVES){
		drive_count=MAX_DRIVES;
	}
	for (u32 i=0;i<drive_count;i++){
		if (_syscall_drive_list_get(i,drives+i,sizeof(drive_t))<0){
			drives[i].flags=0;
		}
		if (drives[i].flags&DRIVE_FLAG_BOOT){
			drive_boot_index=i;
		}
	}
}
