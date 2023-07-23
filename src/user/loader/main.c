#include <user/drive.h>
#include <user/partition.h>
#include <user/print.h>
#include <user/types.h>
#include <user/syscall.h>



void main(void){
	drive_init();
	partition_init();
	if (drives[drive_boot_index].type==DRIVE_TYPE_ATA||drives[drive_boot_index].type==DRIVE_TYPE_ATAPI){
		printf("Booting from an installable medium!\n");
	}
	for (u32 i=0;i<drive_count;i++){
		printf("[%u]: %s (%s)%s\n",i,drives[i].name,drives[i].model_number,((drives[i].flags&DRIVE_FLAG_BOOT)?" [boot]":""));
	}
	for (u32 i=0;i<partition_count;i++){
		printf("[%u]: %s%s\n",i,partitions[i].name,((partitions[i].flags&PARTITION_FLAG_BOOT)?" [boot]":""));
	}
	printf("\x1b[38;2;169;42;187mHello world!\x1b[0m\n");
	_syscall_elf_load("/abc.elf",8);
}
