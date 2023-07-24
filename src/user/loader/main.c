#include <user/drive.h>
#include <user/elf.h>
#include <user/partition.h>
#include <user/print.h>
#include <user/types.h>



static const char* partition_type_names[]={
	[PARTITION_TYPE_EMPTY_DRIVE]="empty drive",
	[PARTITION_TYPE_EMPTY]="empty",
	[PARTITION_TYPE_ISO9660]="iso9660",
	[PARTITION_TYPE_GPT]="gpt"
};



void main(void){
	drive_init();
	partition_init();
	if (drives[drive_boot_index].type==DRIVE_TYPE_ATA||drives[drive_boot_index].type==DRIVE_TYPE_ATAPI){
		printf("Booting from an installable medium!\n");
	}
	printf("Drives:\n");
	for (u32 i=0;i<drive_count;i++){
		printf("[%u]: %s (%s)%s\n",i,drives[i].name,drives[i].model_number,((drives[i].flags&DRIVE_FLAG_BOOT)?" [boot]":""));
	}
	*((char*)0)=0;
	for (;;);
	printf("Partitions:\n");
	for (u32 i=0;i<partition_count;i++){
		printf("[%u]: %s -> %s%s\n",i,partitions[i].name,partition_type_names[partitions[i].type],((partitions[i].flags&PARTITION_FLAG_BOOT)?" [boot]":""));
	}
	printf("\x1b[38;2;169;42;187mHello world!\x1b[0m\n");
	elf_load("/abc.elf");
}
